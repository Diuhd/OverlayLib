#ifndef OVERLAY_WINDOW_H
#define OVERLAY_WINDOW_H

#include "overlay_config.h"

#include <string>

#include <gtk/gtk.h>

class OverlayWindow {
    public:
        GtkWindow* create(GtkApplication* app, const std::string& title, const OverlayWindowConfig& config);
        void set_input_rect(int x, int y, int width, int height) const;

    private:
        void configure_layer_surface(const OverlayWindowConfig& config) const;

        GtkWindow* window_ = nullptr;
};

#endif // OVERLAY_WINDOW_H
