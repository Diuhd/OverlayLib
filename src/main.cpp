#include "overlay.h"

#include <utility>

int main(int argc, char** argv) {
    OverlayConfig config{"io.github.Diuhd.ovl_runtime", "/app/share/ovl_runtime/ovl_host/index.html"};
    config.window()
        .set_fullscreen(true)
        .set_passthrough(true);
    //config.window().set_bounds(0, 0, 600, 400);
    config.webview().set_developer_mode(false);
    config.webview().load_components();

    Overlay overlay{std::move(config)};
    return overlay.run(argc, argv);
}
