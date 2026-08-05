#pragma once

#include "../Widget.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::ui
{
class Canvas;

namespace overlay
{
enum class Layer
{
    Notification = 0,
    Sheet,
    Dialog,
    System,
};

enum class OverlayKind
{
    Toast,
    Panel,
};

// A lightweight stack entry. Overlays render above the current screen and are
// dismissed by id; the manager fades/removes them without the screen knowing.
struct OverlayEntry
{
    std::string id;
    OverlayKind kind{OverlayKind::Toast};
    Layer layer{Layer::Notification};
    float alpha{1.0f};
    std::shared_ptr<Widget> widget;
};

// Owns the overlay stack. Screens and services push toasts/panels through the
// manager; Update drives fade-in/out, Draw renders topmost-first.
class OverlayManager
{
public:
    using DrawFn = std::function<void(Canvas&)>;

    void PushToast(std::string id, std::shared_ptr<Widget> widget, Layer layer = Layer::Notification);
    void PushPanel(std::string id, std::shared_ptr<Widget> widget, Layer layer);
    void Dismiss(std::string_view id);
    void DismissAll();
    bool Has(std::string_view id) const;
    bool IsEmpty() const { return m_Entries.empty(); }

    void Update(float deltaSeconds);
    void Draw(Canvas& canvas);

    float FadeInTime() const { return m_FadeInTime; }
    void SetFadeInTime(float seconds) { m_FadeInTime = seconds; }

private:
    void Push(std::string id, std::shared_ptr<Widget> widget, OverlayKind kind, Layer layer);
    void RemoveFinished();

    struct Entry
    {
        OverlayEntry entry;
        float elapsed{0.0f};
        float lifetime{-1.0f};
        bool removing{false};
        float removalElapsed{0.0f};
    };

    std::vector<Entry> m_Entries;
    float m_FadeInTime{0.16f};
    float m_FadeOutTime{0.12f};
    float m_ToastLifetime{2.6f};
};
} // namespace flachead::ui::overlay
} // namespace flachead::ui
