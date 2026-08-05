#pragma once

#include "../screens/ScreenManager.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include "AppContext.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace flachead::dap
{
constexpr float kStatusH = 38.0f;
constexpr float kHeaderH = 42.0f;
constexpr float kFooterH = 26.0f;

// Shared base for all DAP screens. Carries the service bundle, handles event
// subscription lifecycle, and offers the common chrome (status bar, header,
// footer hints, list rows, empty state, now-playing bar) so screens stay small
// and consistent. Every screen reloads its cached data in RefreshData(), which
// runs on enter and whenever any subscribed event arrives.
class DapScreen : public flachead::screens::Screen
{
public:
    DapScreen(const AppContext& context, std::string title);
    ~DapScreen() override;

    void OnEnter() override;
    void OnUpdate(float deltaSeconds) override;
    bool NeedsRender() const override;

    static std::string FormatDuration(double seconds);
    static Color ArtPlaceholder(const std::string& seed);

protected:
    const AppContext& Ctx() const { return m_Ctx; }
    AppContext& Ctx() { return m_Ctx; }
    const std::string& Title() const { return m_Title; }

    // Reload cached list data. Called on enter and after subscribed events.
    virtual void RefreshData() {}

    // Subscribe to an event type for the lifetime of the screen. Arrivals mark
    // the screen dirty (RefreshData + re-render).
    int Subscribe(flachead::events::Type type);
    // As above, but also runs `handler` (e.g. to capture event payloads).
    int Subscribe(flachead::events::Type type, flachead::events::EventBus::Handler handler);

    // View size remembered from the last Render so event handlers (mouse hit
    // tests) can use the same geometry. UpdateViewSize is called by screens
    // at the top of Render.
    void UpdateViewSize(int width, int height)
    {
        m_ViewWidth = width;
        m_ViewHeight = height;
    }
    int ViewWidth() const { return m_ViewWidth; }
    int ViewHeight() const { return m_ViewHeight; }

    // Chrome helpers.
    void DrawBackground(flachead::ui::Canvas& canvas, int width, int height) const;
    void DrawStatusBar(flachead::ui::Canvas& canvas, int width) const;
    void DrawHeader(flachead::ui::Canvas& canvas, int width, const std::string& title,
                    const std::string& subtitle = {}) const;
    void DrawFooter(flachead::ui::Canvas& canvas, int width, int height,
                    const std::string& hints) const;
    void DrawEmpty(flachead::ui::Canvas& canvas, int width, int height,
                   const std::string& message) const;
    void DrawRow(flachead::ui::Canvas& canvas, int width, float y, float rowH,
                 const std::string& primary, const std::string& secondary,
                 bool selected) const;
    void DrawNowPlayingBar(flachead::ui::Canvas& canvas, int width, int height) const;

    void RequestRefresh() { m_NeedsRender = true; m_DataDirty = true; }

private:
    void OnRefreshEvent(const flachead::events::Event& event);

    AppContext m_Ctx;
    std::string m_Title;
    std::vector<int> m_Subscriptions;
    bool m_DataDirty{false};
    bool m_NeedsRender{false};
    int m_ViewWidth{0};
    int m_ViewHeight{0};
};
} // namespace flachead::dap
