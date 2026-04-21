#ifndef OVERLAY_H
#define OVERLAY_H

#include "glib.h"
#include "gtk4-layer-shell.h"
#include "jsc/jsc.h"

#include <memory>
#include <string>
#include <filesystem>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

struct GObjectUnref {
    void operator()(gpointer object) const noexcept {
        g_object_unref(object);
    }
};

using GtkApplicationPtr = std::unique_ptr<GtkApplication, GObjectUnref>;

class Overlay {
    public:
        explicit Overlay(std::string app_id, std::filesystem::path path);
        int run(int argc, char** argv);
        void set_window_dimensions(const size_t x, const size_t y, const size_t width, const size_t height);
        void set_fullscreen(bool yn);
    private:
        void configure_layer_surface(GtkWindow* window) const;
        void activate(GtkApplication* app);

        GtkWidget* create_webview() const;
        void set_input_rect(const int x, const int y, const int width, const int height) const;
        void handle_click_message(JSCValue *js_result) const;

        GtkWindow* window = nullptr;

        std::string application_id_;
        std::filesystem::path html_to_render;

        bool is_fullscreen = false;

        size_t x_ = 0;
        size_t y_ = 0; 
        size_t window_width_ = 960;
        size_t window_height_ = 540;
};

#endif // OVERLAY_H
