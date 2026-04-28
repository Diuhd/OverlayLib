#include "overlay_window.h"

#include "cairo.h"
#include "gdk/gdk.h"
#include "gtk4-layer-shell.h"

#include <array>
#include <utility>
#include <vector>

GtkWindow* OverlayWindow::create(
    GtkApplication* app,
    const std::string& title,
    const OverlayWindowConfig& config
) {
    window_ = GTK_WINDOW(gtk_application_window_new(app));

    configure_layer_surface(config);
    gtk_window_set_title(window_, title.c_str());

    if (!config.is_fullscreen()) {
        const auto& bounds = config.bounds();
        gtk_window_set_default_size(window_, bounds.width, bounds.height);
    }

    return window_;
}

void OverlayWindow::set_input_rect(const std::vector<InputRegion> &rects) const {
    if (window_ == nullptr) {
        return;
    }

    if (rects == input_rects_) {
        return;
    }

    auto* native = gtk_widget_get_native(GTK_WIDGET(window_));
    if (native == nullptr) {
        return;
    }

    auto* surface = gtk_native_get_surface(native);
    if (surface == nullptr) {
        return;
    }

    input_rects_ = rects;

    auto* region = cairo_region_create();

    for (const auto& rect: rects) {
        cairo_rectangle_int_t cairo_rect {
            .x = rect.x,
            .y = rect.y,
            .width = rect.width,
            .height = rect.height
        };
        cairo_region_union_rectangle(region, &cairo_rect);
    }

    gdk_surface_set_input_region(surface, region);
    cairo_region_destroy(region);
}

void OverlayWindow::configure_layer_surface(const OverlayWindowConfig& config) const {
    gtk_layer_init_for_window(window_);
    gtk_layer_set_layer(window_, GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_keyboard_mode(window_, GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);

    if (config.is_fullscreen()) {
        // A fullscreen overlay should not reserve the entire output for itself.
        gtk_layer_set_exclusive_zone(window_, 0);
    } else {
        gtk_layer_auto_exclusive_zone_enable(window_);
    }

    if (config.is_fullscreen()) {
        gtk_layer_set_margin(window_, GTK_LAYER_SHELL_EDGE_LEFT, 0);
        gtk_layer_set_margin(window_, GTK_LAYER_SHELL_EDGE_RIGHT, 0);
        gtk_layer_set_margin(window_, GTK_LAYER_SHELL_EDGE_TOP, 0);
        gtk_layer_set_margin(window_, GTK_LAYER_SHELL_EDGE_BOTTOM, 0);
    } else {
        const auto& bounds = config.bounds();
        gtk_layer_set_margin(window_, GTK_LAYER_SHELL_EDGE_LEFT, bounds.x);
        gtk_layer_set_margin(window_, GTK_LAYER_SHELL_EDGE_TOP, bounds.y);
    }

    const auto anchor_config = config.is_fullscreen()
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
        gtk_layer_set_anchor(window_, edge, anchored);
    }
}
