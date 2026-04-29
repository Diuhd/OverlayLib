# ovllib_runtime

`ovllib_runtime` is a small C++ overlay runtime for Wayland desktops. It creates a
transparent GTK4 layer-shell window, hosts WebKitGTK content inside it, and loads
user overlay components from the local data directory. 

The current sample starts a fullscreen click-through overlay and loads
`ovllib_host/index.html`. The host page receives component manifests from C++,
creates iframe-backed overlay windows, and reports the interactive rectangles
back to GTK so pointer input can pass through everywhere else.

## Features

- Wayland overlay windows through `gtk4-layer-shell`
- Transparent WebKitGTK 6 WebView content
- QOL support for building and adding overlays with [ovllib_dev]("https://github.com/Diuhd/ovllib_dev") and [ovllib]("https://github.com/Diuhd/ovllib)
- Fullscreen or bounded overlay placement
- Optional click-through behavior with DOM-driven input regions
- Local component discovery from `~/.local/share/ovllib/overlays`

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

## Installation

**Flatpak:**

Download flatpak from releases. Then:

```bash
flatpak install --user ./ovllib_runtime.flatpak
flatpak run io.github.Diuhd.ovllib_runtime
```

**Raw build:**
```bash
meson setup builddir
meson compile -C builddir
```

## Component Layout

Components are loaded from:

```text
~/.local/share/ovllib/overlays/<component-name>/
```

Each component directory must contain an `ovllib_config.toml` file. The runtime
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
movable = true
move_element = "titlebar"
```

With that config, the component entry file would be:

```text
~/.local/share/ovllib/overlays/clock/web/index.html
```

The generated manifest is exposed to the host page through
`window.ovllibRuntime.getManifests()`.

## Input Regions

When `set_passthrough(true)` is enabled, the C++ runtime injects a small script
into the WebView. The script finds every `div.overlay-elem`, converts those DOM
rectangles into native input regions, and sends them back through WebKit's
message handler.

Only those rectangles receive pointer input. The rest of the transparent overlay
passes clicks through to the desktop underneath.

The host also dispatches `ovllib:input-region-changed` when component windows move
or resize so the native input region can be recalculated.

## Troubleshooting

### Meson cannot find `webkitgtk-6.0`

Install the WebKitGTK 6 development package for your distribution and verify
that `pkg-config` can see it:

```bash
pkg-config --modversion webkitgtk-6.0
```

### The overlay starts but no component windows appear

Check that components are installed under `~/.local/share/ovllib/overlays`, and
that every component has a valid `ovllib_config.toml` with `project.name`,
`project.entry_dir`, and `project.entry_file`.

Invalid components are skipped and reported on stderr.

### Clicks do not pass through

Make sure the runtime is using:

```cpp
config.window().set_passthrough(true);
```

The host page must also create overlay windows with the `overlay-elem` class,
because those elements define the native input region.
