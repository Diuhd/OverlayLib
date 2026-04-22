#ifndef OVERLAY_CONFIG_H
#define OVERLAY_CONFIG_H

#include <cstddef>
#include <filesystem>
#include <string>

struct OverlayBounds {
    size_t x = 0;
    size_t y = 0;
    size_t width = 960;
    size_t height = 540;
};

class OverlayWindowConfig {
    public:
        OverlayWindowConfig& set_bounds(size_t x, size_t y, size_t width, size_t height);
        OverlayWindowConfig& set_fullscreen(bool enabled);
        OverlayWindowConfig& set_passthrough(bool enabled);

        [[nodiscard]] const OverlayBounds& bounds() const noexcept;
        [[nodiscard]] bool is_fullscreen() const noexcept;
        [[nodiscard]] bool is_passthrough_enabled() const noexcept;

    private:
        OverlayBounds bounds_{};
        bool fullscreen_ = false;
        bool passthrough_ = true;
};

class OverlayWebViewConfig {
    public:
        OverlayWebViewConfig& set_html_path(std::filesystem::path html_path);
        OverlayWebViewConfig& set_developer_mode(bool enabled);

        [[nodiscard]] const std::filesystem::path& html_path() const noexcept;
        [[nodiscard]] bool is_developer_mode_enabled() const noexcept;

    private:
        std::filesystem::path html_path_;
        bool developer_mode_ = false;
};

class OverlayConfig {
    public:
        OverlayConfig() = default;
        OverlayConfig(std::string application_id, std::filesystem::path html_path);

        OverlayConfig& set_application_id(std::string application_id);

        [[nodiscard]] const std::string& application_id() const noexcept;

        [[nodiscard]] OverlayWindowConfig& window() noexcept;
        [[nodiscard]] const OverlayWindowConfig& window() const noexcept;

        [[nodiscard]] OverlayWebViewConfig& webview() noexcept;
        [[nodiscard]] const OverlayWebViewConfig& webview() const noexcept;

    private:
        std::string application_id_;
        OverlayWindowConfig window_config_{};
        OverlayWebViewConfig webview_config_{};
};

#endif // OVERLAY_CONFIG_H
