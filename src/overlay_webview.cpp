#include "overlay_webview.h"

#include "glib-object.h"
#include "gtk/gtkcssprovider.h"
#include "webkit/webkit.h"

#include <filesystem>
#include <utility>

namespace {
constexpr auto kMissingHtmlFallback = R"(
    <!doctype html>
    <html lang="en">
    <body style="margin:0;font-family:sans-serif;background:transparent;display:grid;place-items:center;height:100vh;">
      <div>
        <h1>OverlayLib</h1>
        <p>Sample HTML file not found. Expected samples/index.html.</p>
      </div>
    </body>
    </html>
)";

constexpr auto kInputRegionScript = R"(
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
)";
} // namespace

OverlayWebView::OverlayWebView(
    OverlayWebViewConfig config,
    const bool passthrough,
    InputRegionHandler input_region_handler
) : config_(std::move(config)),
    passthrough_(passthrough),
    input_region_handler_(std::move(input_region_handler)) {}

GtkWidget* OverlayWebView::create() const {
    auto* manager = webkit_user_content_manager_new();

    if (passthrough_) {
        webkit_user_content_manager_register_script_message_handler(manager, "region", nullptr);
        g_signal_connect(
            manager,
            "script-message-received::region",
            G_CALLBACK(+[](
                [[maybe_unused]] WebKitUserContentManager* manager,
                JSCValue* js_result,
                gpointer user_data
            ) {
                const auto* overlay_webview = static_cast<OverlayWebView*>(user_data);
                overlay_webview->handle_click_message(js_result);
            }),
            gpointer(this)
        );

        auto* script = webkit_user_script_new(
            kInputRegionScript,
            WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            nullptr,
            nullptr
        );

        webkit_user_content_manager_add_script(manager, script);
        webkit_user_script_unref(script);
    }

    auto* settings = webkit_settings_new();
    webkit_settings_set_hardware_acceleration_policy(
        settings,
        WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS
    );
    webkit_settings_set_enable_2d_canvas_acceleration(settings, TRUE);
    webkit_settings_set_enable_page_cache(settings, TRUE);
    webkit_settings_set_enable_developer_extras(settings, config_.is_developer_mode_enabled());
    webkit_settings_set_draw_compositing_indicators(settings, FALSE);

    auto* webview = static_cast<GtkWidget*>(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "user-content-manager", manager,
        "settings", settings,
        nullptr
    ));
    g_object_unref(settings);
    g_object_unref(manager);

    const GdkRGBA bg_color{0.0f, 0.0f, 0.0f, 0.0f};
    webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(webview), &bg_color);

    auto* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, "* { background-color: transparent; }");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
    load_content(webview);
    return webview;
}

void OverlayWebView::handle_click_message(JSCValue* js_result) const {
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
        input_region_handler_(
            jsc_value_to_int32(x),
            jsc_value_to_int32(y),
            jsc_value_to_int32(width),
            jsc_value_to_int32(height)
        );
    }
}

void OverlayWebView::load_content(GtkWidget* webview) const {
    const auto& html_path = config_.html_path();
    if (!html_path.empty() && std::filesystem::exists(html_path)) {
        const auto absolute_path = std::filesystem::absolute(html_path);
        g_autofree char* uri = g_filename_to_uri(absolute_path.c_str(), nullptr, nullptr);
        if (uri != nullptr) {
            webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri);
            return;
        }
    }

    webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview), kMissingHtmlFallback, nullptr);
}
