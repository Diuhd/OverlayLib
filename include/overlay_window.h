#ifndef OVERLAY_WINDOW_H
#define OVERLAY_WINDOW_H

#include "overlay_config.h"

#include <string>

#include <gtk/gtk.h>
#include <vector>

struct InputRegion {
    int x;
    int y;
    int width;
    int height;

    bool operator==(const InputRegion&) const = default;
};

class OverlayWindow {
    public:
        GtkWindow* create(GtkApplication* app, const std::string& title, const OverlayWindowConfig& config);
        void set_input_rect(const std::vector<InputRegion> &rects) const;

    private:
        void configure_layer_surface(const OverlayWindowConfig& config) const;

        GtkWindow* window_ = nullptr;
        mutable std::vector<InputRegion> input_rects_;
};

#endif // OVERLAY_WINDOW_H
