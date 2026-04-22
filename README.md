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
- relax Docker security policy enough for WebKitGTK's own bubblewrap sandbox to start
- build the project into `builddir-docker`
- launch `./builddir-docker/overlaylib`

The script now keeps the WebKitGTK sandbox enabled and instead adjusts the
container runtime so WebKit can create the namespaces it needs. In practice
that means running the container with `seccomp=unconfined`, and additionally
`apparmor=unconfined` on AppArmor hosts, because Docker's defaults can block
the namespace syscalls that bubblewrap relies on.

If your host globally disables unprivileged user namespaces, WebKitGTK may
still fail to start its sandbox. In that case you will need to allow user
namespaces on the host rather than disabling the WebKit sandbox in the
container.

### Run a different command

You can override the default container command if you want a shell or a custom build step:

```bash
CONTAINER_CMD='bash' ./scripts/run-docker-wayland.sh
```

or

```bash
CONTAINER_CMD='meson compile -C builddir-docker' ./scripts/run-docker-wayland.sh
```
