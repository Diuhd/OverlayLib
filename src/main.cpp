#include "overlay.h"

#include "argparse/argparse.hpp"

#include <utility>

int main(int argc, char** argv) {
    argparse::ArgumentParser program("ovl");

    program.add_argument("")
    // OverlayConfig config{"io.diuhd.overlaylib", "samples/index.html"};
    // config.window()
    //     .set_fullscreen(true)
    //     .set_passthrough(true);
    // config.webview().set_developer_mode(false);

    // Overlay overlay{std::move(config)};
    // return overlay.run(argc, argv);
}
