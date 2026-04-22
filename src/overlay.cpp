#include "overlay.h"

#include "overlay_webview.h"
#include "overlay_window.h"

#include <memory>
#include <utility>

namespace {
struct GObjectUnref {
    void operator()(gpointer object) const noexcept {
        g_object_unref(object);
    }
};

using GtkApplicationPtr = std::unique_ptr<GtkApplication, GObjectUnref>;
} // namespace

Overlay::Overlay(std::string app_id, std::filesystem::path path)
    : Overlay(OverlayConfig{std::move(app_id), std::move(path)}) {}

Overlay::Overlay(OverlayConfig config)
    : config_(std::move(config)) {}

Overlay::~Overlay() = default;

void Overlay::set_window_dimensions(const size_t x, const size_t y, const size_t width, const size_t height) {
    config_.window().set_bounds(x, y, width, height);
}

void Overlay::set_fullscreen(const bool yn) {
    config_.window().set_fullscreen(yn);
}

void Overlay::allow_passthrough(const bool yn) {
    config_.window().set_passthrough(yn);
}

void Overlay::config_dev_mode(const bool yn) {
    config_.webview().set_developer_mode(yn);
}

OverlayConfig& Overlay::config() noexcept {
    return config_;
}

const OverlayConfig& Overlay::config() const noexcept {
    return config_;
}

void Overlay::activate(GtkApplication* app) {
    window_controller_ = std::make_unique<OverlayWindow>();
    auto* window = window_controller_->create(app, config_.application_id(), config_.window());

    webview_controller_ = std::make_unique<OverlayWebView>(
        config_.webview(),
        config_.window().is_passthrough_enabled(),
        [window_controller = window_controller_.get()](const int x, const int y, const int width, const int height) {
            window_controller->set_input_rect(x, y, width, height);
        }
    );

    auto* webview_widget = webview_controller_->create();
    gtk_widget_set_focusable(webview_widget, TRUE);
    gtk_window_set_child(window, webview_widget);

    gtk_window_present(window);
    gtk_widget_grab_focus(webview_widget);
}

int Overlay::run(int argc, char** argv) {
    GtkApplicationPtr app{
        gtk_application_new(config_.application_id().c_str(), G_APPLICATION_DEFAULT_FLAGS),
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
