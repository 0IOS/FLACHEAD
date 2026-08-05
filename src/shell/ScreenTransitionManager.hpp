#pragma once

namespace flachead::shell
{
// Fullscreen fade applied around screen pushes and pops. Application calls
// BeginFade at every navigation point and paints a fullscreen overlay using
// Alpha() (a single rect, cheap on the Pi's software renderer); Update
// advances the fade so the new screen appears to fade in.
class ScreenTransitionManager
{
public:
    void BeginFade(float durationSeconds = 0.18f);
    void Update(float deltaSeconds);

    bool Active() const { return m_Elapsed < m_Duration; }
    bool NeedsRender() const { return m_Elapsed < m_Duration || m_Alpha > 0.0f; }

    // Overlay opacity in [0, 1]; 1 means fully covered.
    float Alpha() const { return m_Alpha; }

private:
    float m_Elapsed{0.0f};
    float m_Duration{0.0f};
    float m_Alpha{0.0f};
};
} // namespace flachead::shell
