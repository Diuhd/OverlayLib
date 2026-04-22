#ifndef OVERLAY_WEBVIEW_H
#define OVERLAY_WEBVIEW_H

#include "overlay_config.h"

#include "jsc/jsc.h"

#include <functional>

#include <gtk/gtk.h>

class OverlayWebView {
    public:
        using InputRegionHandler = std::function<void(int x, int y, int width, int height)>;

        OverlayWebView(OverlayWebViewConfig config, bool passthrough, InputRegionHandler input_region_handler);

        GtkWidget* create() const;

    private:
        void handle_click_message(JSCValue* js_result) const;
        void load_content(GtkWidget* webview) const;

        OverlayWebViewConfig config_;
        bool passthrough_ = true;
        InputRegionHandler input_region_handler_;
};

#endif // OVERLAY_WEBVIEW_H
