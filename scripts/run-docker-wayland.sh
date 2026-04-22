#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE_NAME="${IMAGE_NAME:-overlaylib:arch-webkit}"
BUILD_DIR="${BUILD_DIR:-builddir-docker}"
CONTAINER_HOME="${CONTAINER_HOME:-/tmp/overlaylib-home}"
CONTAINER_CACHE="${CONTAINER_CACHE:-${CONTAINER_HOME}/.cache}"
CONTAINER_CMD="${CONTAINER_CMD:-mkdir -p '${CONTAINER_HOME}' '${CONTAINER_CACHE}'; meson setup ${BUILD_DIR} --reconfigure || meson setup ${BUILD_DIR}; meson compile -C ${BUILD_DIR}; ./${BUILD_DIR}/overlaylib}"

if [[ -z "${XDG_RUNTIME_DIR:-}" ]]; then
    echo "XDG_RUNTIME_DIR is not set. Start this from your Wayland session." >&2
    exit 1
fi

if [[ -z "${WAYLAND_DISPLAY:-}" ]]; then
    echo "WAYLAND_DISPLAY is not set. Start this from your Wayland session." >&2
    exit 1
fi

WAYLAND_SOCKET="${XDG_RUNTIME_DIR}/${WAYLAND_DISPLAY}"
if [[ ! -S "${WAYLAND_SOCKET}" ]]; then
    echo "Wayland socket not found at ${WAYLAND_SOCKET}" >&2
    exit 1
fi

docker build -t "${IMAGE_NAME}" "${ROOT_DIR}"

docker run --rm -it \
    --user "$(id -u):$(id -g)" \
    --network host \
    --device /dev/dri \
    -e HOME="${CONTAINER_HOME}" \
    -e WAYLAND_DISPLAY \
    -e XDG_RUNTIME_DIR \
    -e XDG_CACHE_HOME="${CONTAINER_CACHE}" \
    -e DISPLAY="${DISPLAY:-}" \
    -e DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-}" \
    -e WEBKIT_DISABLE_SANDBOX_THIS_IS_DANGEROUS=1 \
    -v "${ROOT_DIR}:/workspace" \
    -v "${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR}" \
    -w /workspace \
    "${IMAGE_NAME}" \
    bash -lc "${CONTAINER_CMD}"
