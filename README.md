# overlaylib

## Docker

If your host `webkitgtk-6.0` looks suspect, you can build and run the sample in an
official Arch Linux container instead of using the host CachyOS package.

### Prerequisites

- Docker installed and working for your user
- A Wayland session on the host
- Access to `/dev/dri` for GPU acceleration

### Run the sample

```bash
chmod +x scripts/run-docker-wayland.sh
./scripts/run-docker-wayland.sh
```

That script will:

- build an `archlinux:base-devel` image
- install `gtk4`, `gtk4-layer-shell`, `meson`, `ninja`, `pkgconf`, and `webkitgtk-6.0`
- mount your repo into the container
- forward your Wayland socket into the container
- set a writable `HOME` and `XDG_CACHE_HOME` inside the container
- disable the WebKitGTK bubblewrap sandbox for this container run
- build the project into `builddir-docker`
- launch `./builddir-docker/overlaylib`

The sandbox disable is intentional here because some Docker setups do not allow
the unprivileged user namespaces that WebKitGTK's internal bubblewrap sandbox
expects. This is fine for local debugging, but it is less secure than a normal
desktop install.

### Run a different command

You can override the default container command if you want a shell or a custom build step:

```bash
CONTAINER_CMD='bash' ./scripts/run-docker-wayland.sh
```

or

```bash
CONTAINER_CMD='meson compile -C builddir-docker' ./scripts/run-docker-wayland.sh
```
