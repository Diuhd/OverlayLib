#include "overlay.h"

int main(int argc, char** argv) {
    Overlay overlay{"io.diuhd.overlaylib", "samples/index.html"};
    overlay.set_window_dimensions(0, 0, 1080, 720);
    overlay.allow_passthrough(false);
    overlay.config_dev_mode(true);
    return overlay.run(argc, argv);
}
