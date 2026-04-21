#include "overlay.h"

int main(int argc, char** argv) {
    Overlay overlay{"io.diuhd.overlaylib", "samples/index.html"};
    overlay.set_fullscreen(true);
    return overlay.run(argc, argv);
}
