#include "overlay_config.h"

#include <utility>

OverlayWindowConfig& OverlayWindowConfig::set_bounds(
    const size_t x,
    const size_t y,
    const size_t width,
    const size_t height
) {
    bounds_ = OverlayBounds{
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };
    return *this;
}

OverlayWindowConfig& OverlayWindowConfig::set_fullscreen(const bool enabled) {
    fullscreen_ = enabled;
    return *this;
}

OverlayWindowConfig& OverlayWindowConfig::set_passthrough(const bool enabled) {
    passthrough_ = enabled;
    return *this;
}

const OverlayBounds& OverlayWindowConfig::bounds() const noexcept {
    return bounds_;
}

bool OverlayWindowConfig::is_fullscreen() const noexcept {
    return fullscreen_;
}

bool OverlayWindowConfig::is_passthrough_enabled() const noexcept {
    return passthrough_;
}

OverlayWebViewConfig& OverlayWebViewConfig::set_html_path(std::filesystem::path html_path) {
    html_path_ = std::move(html_path);
    return *this;
}

OverlayWebViewConfig& OverlayWebViewConfig::set_developer_mode(const bool enabled) {
    developer_mode_ = enabled;
    return *this;
}

const std::filesystem::path& OverlayWebViewConfig::html_path() const noexcept {
    return html_path_;
}

bool OverlayWebViewConfig::is_developer_mode_enabled() const noexcept {
    return developer_mode_;
}

OverlayConfig::OverlayConfig(std::string application_id, std::filesystem::path html_path)
    : application_id_(std::move(application_id)) {
    webview_config_.set_html_path(std::move(html_path));
}

OverlayConfig& OverlayConfig::set_application_id(std::string application_id) {
    application_id_ = std::move(application_id);
    return *this;
}

const std::string& OverlayConfig::application_id() const noexcept {
    return application_id_;
}

OverlayWindowConfig& OverlayConfig::window() noexcept {
    return window_config_;
}

const OverlayWindowConfig& OverlayConfig::window() const noexcept {
    return window_config_;
}

OverlayWebViewConfig& OverlayConfig::webview() noexcept {
    return webview_config_;
}

const OverlayWebViewConfig& OverlayConfig::webview() const noexcept {
    return webview_config_;
}
