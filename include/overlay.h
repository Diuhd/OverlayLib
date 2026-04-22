#ifndef OVERLAY_H
#define OVERLAY_H

#include "overlay_config.h"

#include <filesystem>
#include <memory>
#include <string>

#include <gtk/gtk.h>

class OverlayWebView;
class OverlayWindow;

class Overlay {
    public:
        explicit Overlay(std::string app_id, std::filesystem::path path);
        explicit Overlay(OverlayConfig config);
        ~Overlay();

        int run(int argc, char** argv);

        void set_window_dimensions(size_t x, size_t y, size_t width, size_t height);
        void set_fullscreen(bool yn);
        void allow_passthrough(bool yn);
        void config_dev_mode(bool yn);

        [[nodiscard]] OverlayConfig& config() noexcept;
        [[nodiscard]] const OverlayConfig& config() const noexcept;

    private:
        void activate(GtkApplication* app);

        OverlayConfig config_;
        std::unique_ptr<OverlayWindow> window_controller_;
        std::unique_ptr<OverlayWebView> webview_controller_;
};

#endif // OVERLAY_H
