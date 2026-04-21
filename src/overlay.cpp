#include "overlay.h"
#include "cairo.h"
#include "gdk/gdk.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkcssprovider.h"
#include "gtk/gtkshortcut.h"
#include "jsc/jsc.h"
#include "webkit/WebKitSettings.h"
#include "webkit/WebKitUserContentManager.h"

#include <array>
#include <cstddef>

Overlay::Overlay(
    std::string app_id,
    std::filesystem::path path
): application_id_(std::move(app_id)), html_to_render(std::move(path)) {}

void Overlay::set_fullscreen(bool yn) {
    is_fullscreen = yn;
}

void Overlay::set_window_dimensions(const size_t x, const size_t y, const size_t width, const size_t height) {
    x_ = x;
    y_ = y;
    window_width_ = width;
    window_height_ = height;
}

void Overlay::configure_layer_surface(GtkWindow* window) const {
    gtk_layer_init_for_window(window);
    gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);

    if (is_fullscreen) {
        // A fullscreen overlay should not reserve the entire output for itself.
        gtk_layer_set_exclusive_zone(window, 0);
    } else {
        gtk_layer_auto_exclusive_zone_enable(window);
    }

    if (is_fullscreen) {
        gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, 0);
        gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_RIGHT, 0);
        gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, 0);
        gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_BOTTOM, 0);
    } else {
        gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, x_);
        gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, y_);
    }

    const auto anchor_config = is_fullscreen
        ? std::array{
            std::pair{GTK_LAYER_SHELL_EDGE_LEFT, true},
            std::pair{GTK_LAYER_SHELL_EDGE_RIGHT, true},
            std::pair{GTK_LAYER_SHELL_EDGE_TOP, true},
            std::pair{GTK_LAYER_SHELL_EDGE_BOTTOM, true},
        }
        : std::array{
            std::pair{GTK_LAYER_SHELL_EDGE_LEFT, true},
            std::pair{GTK_LAYER_SHELL_EDGE_RIGHT, false},
            std::pair{GTK_LAYER_SHELL_EDGE_TOP, true},
            std::pair{GTK_LAYER_SHELL_EDGE_BOTTOM, false},
        };

    for (const auto& [edge, anchored] : anchor_config) {
        gtk_layer_set_anchor(window, edge, anchored);
    }
}

void Overlay::set_input_rect(const int x, const int y, const int width, const int height) const {
    auto* native = gtk_widget_get_native(GTK_WIDGET(window));
    auto* surface = gtk_native_get_surface(native);

    cairo_rectangle_int_t rect {
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };

    auto* region = cairo_region_create_rectangle(&rect);
    gdk_surface_set_input_region(surface, region);
    cairo_region_destroy(region);
}

void Overlay::handle_click_message(JSCValue *js_result) const {
    if (!jsc_value_is_object(js_result)) {
        g_warning("Received click message, but payload is not an object.");
        return;
    }

    g_autoptr(JSCValue) x = jsc_value_object_get_property(js_result, "x");
    g_autoptr(JSCValue) y = jsc_value_object_get_property(js_result, "y");
    g_autoptr(JSCValue) width = jsc_value_object_get_property(js_result, "width");
    g_autoptr(JSCValue) height = jsc_value_object_get_property(js_result, "height");
    if (
        jsc_value_is_number(x)
        && jsc_value_is_number(y)
        && jsc_value_is_number(width)
        && jsc_value_is_number(height)
    ) {
        set_input_rect(
            jsc_value_to_int32(x), 
            jsc_value_to_int32(y), 
            jsc_value_to_int32(width), 
            jsc_value_to_int32(height)
        );
    }
}

GtkWidget* Overlay::create_webview() const {
    auto* manager = webkit_user_content_manager_new();

    webkit_user_content_manager_register_script_message_handler(manager, "region", nullptr);
    g_signal_connect(manager, "script-message-received::region",
        G_CALLBACK(+[](
            [[maybe_unused]] WebKitUserContentManager* manager, 
            [[maybe_unused]] JSCValue* js_result, 
            [[maybe_unused]] gpointer user_data) {
                auto *overlay = static_cast<Overlay*>(user_data);
                overlay->handle_click_message(js_result);
            }
        ), gpointer(this)
    );

    auto* script = webkit_user_script_new(
        R"(
            function sendInputRegion() {
                const box = document.getElementById("box");
                if (!box) {
                    return;
                }

                const rect = box.getBoundingClientRect();

                window.webkit.messageHandlers.region.postMessage({
                    x: Math.round(rect.left),
                    y: Math.round(rect.top),
                    width: Math.round(rect.width),
                    height: Math.round(rect.height)
                });
            }

            window.addEventListener("load", sendInputRegion);
            window.addEventListener("resize", sendInputRegion);
            document.addEventListener("pointerup", sendInputRegion);
        )",
        WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
        nullptr,
        nullptr
    );

    webkit_user_content_manager_add_script(manager, script);
    webkit_user_script_unref(script);

    auto* settings = webkit_settings_new();
    webkit_settings_set_hardware_acceleration_policy(
        settings,
        WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS
    );
    webkit_settings_set_enable_2d_canvas_acceleration(settings, TRUE);
    webkit_settings_set_enable_page_cache(settings, TRUE);

    webkit_settings_set_draw_compositing_indicators(settings, FALSE); // Only for debugging (telling if compositing is happeninf)
    

    auto* webview = (GtkWidget*)g_object_new(
        WEBKIT_TYPE_WEB_VIEW, 
        "user-content-manager", manager,
        "settings", settings, 
        nullptr
    );
    g_object_unref(settings);
    g_object_unref(manager);

    const GdkRGBA bg_color{0.0f, 0.0f, 0.0f, 0.0f};
    webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(webview), &bg_color);

    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "* { background-color: transparent; }");
    

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);

    if (!html_to_render.empty() && std::filesystem::exists(html_to_render)) {
        const auto absolute_path = std::filesystem::absolute(html_to_render);
        g_autofree char* uri = g_filename_to_uri(absolute_path.c_str(), nullptr, nullptr);
        if (uri != nullptr) {
            webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri);
            return webview;
        }
    }

    webkit_web_view_load_html(
        WEBKIT_WEB_VIEW(webview),
        R"(
            <!doctype html>
            <html lang="en">
            <body style="margin:0;font-family:sans-serif;background:transparent;display:grid;place-items:center;height:100vh;">
              <div>
                <h1>OverlayLib</h1>
                <p>Sample HTML file not found. Expected samples/index.html.</p>
              </div>
            </body>
            </html>
        )",
        nullptr
    );

    return webview;
}

void Overlay::activate(GtkApplication* app) {
    window = GTK_WINDOW(gtk_application_window_new(app));

    configure_layer_surface(window);
    gtk_window_set_title(window, application_id_.c_str());
    if (!is_fullscreen) {
        gtk_window_set_default_size(window, window_width_, window_height_);
    }
    auto* webview = create_webview();
    gtk_window_set_child(window, webview);
    gtk_window_present(window);
}

int Overlay::run(int argc, char** argv) {
    GtkApplicationPtr app{
        gtk_application_new(application_id_.c_str(), G_APPLICATION_DEFAULT_FLAGS),
    };

    g_signal_connect(
        app.get(),
        "activate",
        G_CALLBACK(+[](GtkApplication* gtk_app, gpointer user_data) {
            auto* overlay = static_cast<Overlay*>(user_data);
            overlay->activate(gtk_app);
        }),
        this
    );
    return g_application_run(G_APPLICATION(app.get()), argc, argv);
}
