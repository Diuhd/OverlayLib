FROM archlinux:base-devel

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm --needed \
        gtk4 \
        gtk4-layer-shell \
        meson \
        ninja \
        pkgconf \
        webkitgtk-6.0 \
    && pacman -Scc --noconfirm

WORKDIR /workspace

CMD ["bash"]
