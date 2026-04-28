# ovl_runtime

`ovl_runtime` is a small C++ overlay runtime for Wayland desktops. It creates a
transparent GTK4 layer-shell window, hosts WebKitGTK content inside it, and loads
user overlay components from the local data directory. 

The current sample starts a fullscreen click-through overlay and loads
`ovl_host/index.html`. The host page receives component manifests from C++,
creates iframe-backed overlay windows, and reports the interactive rectangles
back to GTK so pointer input can pass through everywhere else.

## Features

- Wayland overlay windows through `gtk4-layer-shell`
- Transparent WebKitGTK 6 WebView content
- QOL support for building and adding overlays with [ovl_dev]("https://github.com/Diuhd/ovl_dev") and [ovl]("https://github.com/Diuhd/ovl)
- Fullscreen or bounded overlay placement
- Optional click-through behavior with DOM-driven input regions
- Local component discovery from `~/.local/share/ovl/overlays`

## Requirements

Runtime/build dependencies:

- C++20 compiler
- Meson
- Ninja
- `pkgconf`
- `gtk4`
- `gtk4-layer-shell`
- `webkitgtk-6.0`
- `nlohmann-json`
- `tomlplusplus`

On Arch-based systems:

```bash
sudo pacman -S --needed base-devel meson ninja pkgconf gtk4 gtk4-layer-shell webkitgtk-6.0 nlohmann-json tomlplusplus
```

## Build

```bash
meson setup builddir
meson compile -C builddir
```

## Component Layout

Components are loaded from:

```text
~/.local/share/ovl/overlays/<component-name>/
```

Each component directory must contain an `ovl_config.toml` file. The runtime
uses that file to find the component entry point and to size/place its iframe in
the overlay host.

Example:

```toml
[project]
name = "clock"
entry_dir = "web"
entry_file = "index.html"

[window]
width = 320
height = 180
x = 40
y = 40
always_on_display = true
movable = true
move_element = "titlebar"
```

With that config, the component entry file would be:

```text
~/.local/share/ovl/overlays/clock/web/index.html
```

The generated manifest is exposed to the host page through
`window.ovlRuntime.getManifests()`.

## Input Regions

When `set_passthrough(true)` is enabled, the C++ runtime injects a small script
into the WebView. The script finds every `div.overlay-elem`, converts those DOM
rectangles into native input regions, and sends them back through WebKit's
message handler.

Only those rectangles receive pointer input. The rest of the transparent overlay
passes clicks through to the desktop underneath.

The host also dispatches `ovl:input-region-changed` when component windows move
or resize so the native input region can be recalculated.

## Docker / CachyOS WebKitGTK Workaround

Some CachyOS systems can hit host-package WebKitGTK problems with
`webkitgtk-6.0`. In this project that shows up while starting the WebView, with
errors mentioning WebKitGTK/JavaScriptCore and the `Time` module. If the overlay
builds but the WebView fails at runtime with a Time-module-related WebKitGTK
error, treat it as a host `webkitgtk-6.0` packaging/runtime issue first.

The repository includes an Arch Linux Docker runner so you can build and launch
the sample against the official Arch packages instead of the host CachyOS
package:

```bash
chmod +x scripts/run-docker-wayland.sh
./scripts/run-docker-wayland.sh
```

The script will:

- build an `archlinux:base-devel` image
- install `gtk4`, `gtk4-layer-shell`, `meson`, `ninja`, `pkgconf`,
  `webkitgtk-6.0`, `nlohmann-json`, and `tomlplusplus`
- mount this repository into the container
- forward the host Wayland socket
- set writable `HOME` and `XDG_CACHE_HOME` directories inside the container
- relax Docker security policy enough for WebKitGTK's bubblewrap sandbox
- build the project into `builddir-docker`
- launch `./builddir-docker/ovl_runtime`

The WebKitGTK sandbox remains enabled. The container is instead started with
relaxed Docker security options so WebKit can create the namespaces it needs:
`seccomp=unconfined`, plus `apparmor=unconfined` on AppArmor hosts.

If the host globally disables unprivileged user namespaces, WebKitGTK may still
fail to start its sandbox. In that case, allow user namespaces on the host
rather than disabling the WebKit sandbox.

You can also run a different command inside the container:

```bash
CONTAINER_CMD='bash' ./scripts/run-docker-wayland.sh
```

or:

```bash
CONTAINER_CMD='meson compile -C builddir-docker' ./scripts/run-docker-wayland.sh
```

## Troubleshooting

### Meson cannot find `webkitgtk-6.0`

Install the WebKitGTK 6 development package for your distribution and verify
that `pkg-config` can see it:

```bash
pkg-config --modversion webkitgtk-6.0
```

### The overlay starts but no component windows appear

Check that components are installed under `~/.local/share/ovl/overlays`, and
that every component has a valid `ovl_config.toml` with `project.name`,
`project.entry_dir`, and `project.entry_file`.

Invalid components are skipped and reported on stderr.

### Clicks do not pass through

Make sure the runtime is using:

```cpp
config.window().set_passthrough(true);
```

The host page must also create overlay windows with the `overlay-elem` class,
because those elements define the native input region.
