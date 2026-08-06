# 24_VOLUME_OVERLAY.md

# FLACHEAD UI — Volume Overlay

## 1. Purpose

The Volume Overlay provides immediate visual feedback for audio volume changes.

Volume control is one of the most important interactions in FLACHEAD because the device is designed as a dedicated music player.

The overlay must support:

* physical volume dial/encoder input
* touchscreen volume adjustment
* hardware button integration if available
* real-time feedback without interrupting playback

The volume system should feel similar to a high-end digital audio player (DAP), where changing volume is fast, tactile, and always available.

---

# 2. Design goals

The Volume Overlay must:

* Appear instantly after volume changes.
* Never stop or pause playback.
* Work globally across all screens.
* Require minimal CPU/GPU usage.
* Provide clear volume level feedback.
* Support the TANCHJIM BUNNY DSP output chain.
* Avoid looking like a smartphone volume popup.

Primary principle:

> Volume adjustment should always feel like a hardware interaction, not a software action.

---

# 3. Architecture

The Volume Overlay is a global overlay managed by the Overlay Manager.

Architecture:

```text id="2ccw8k"
Volume Input
     |
     v
Input Manager
     |
     v
Audio Service
     |
     +----------------+
                      |
                      v
              Volume State Manager
                      |
                      v
              Overlay Manager
                      |
                      v
              Volume Renderer
```

---

# 4. Dependencies

The Volume Overlay depends on:

* Input Manager
* Audio Service
* Playback Engine
* Media Session Manager
* TANCHJIM BUNNY DSP integration layer
* Theme Engine
* Overlay Manager
* Configuration Service

---

# 5. Volume control sources

FLACHEAD supports multiple volume inputs.

## 5.1 Physical encoder

Primary method.

Example:

```text id="c9w1mz"
Rotate clockwise
        |
        v
Increase volume

Rotate counter-clockwise
        |
        v
Decrease volume
```

Advantages:

* instant feedback
* works without looking
* preserves DAP feeling

---

## 5.2 Touch control

Optional.

Possible uses:

* volume slider
* tap adjustment
* temporary touch interaction

Touch should not replace the physical control philosophy.

---

## 5.3 Hardware buttons

If configured:

```text
Volume Up
Volume Down
```

should trigger the same overlay.

---

# 6. Layout

The overlay should appear near the bottom portion of the display where it does not cover important content.

Preferred layout:

```text id="9g1p0h"
┌─────────────────────┐
│                     │
│    Current Screen   │
│                     │
│                     │
│ ┌─────────────────┐ │
│ │ 🔊 Volume       │ │
│ │                 │ │
│ │ ███████░░░░     │ │
│ │                 │ │
│ │       72%       │ │
│ └─────────────────┘ │
└─────────────────────┘
```

---

# 7. Visual elements

The overlay contains:

## Volume icon

States:

* muted
* low
* medium
* high

The icon should not require multiple complex assets.

---

## Progress indicator

Options:

* horizontal bar
* circular ring
* segmented indicator

Recommended for FLACHEAD:

Horizontal bar.

Reason:

* cheaper rendering
* clearer on small screen
* easier with touch

---

## Numeric value

Display:

```text
0-100%
```

Optional:

```text
42 / 100
```

Avoid decibel calculations unless DSP integration exposes reliable values.

---

# 8. Volume levels

Internal representation:

Recommended:

```text
0-100 integer scale
```

Mapping layer:

```text
FLACHEAD Volume
        |
        v
Audio Backend
        |
        v
DSP Hardware Volume
```

The UI should not directly control USB hardware commands.

---

# 9. Mute behavior

Mute should be treated separately.

States:

```text
Normal

Volume:
50%

↓

Mute

Volume:
50%
(output disabled)

↓

Unmute

Volume:
50%
```

Important:

Mute does not reset volume.

---

# 10. Appearance timing

The overlay appears when:

* volume changes
* mute toggled
* audio device changes
* volume calibration occurs

---

Recommended visibility:

Normal adjustment:

2 seconds

Rapid adjustment:

Extend timer while changing

Example:

```text
Rotate encoder continuously

Overlay remains visible

Stop rotation

Timer starts

Overlay disappears
```

---

# 11. Interaction

## During volume adjustment

Input priority:

```text
Volume Input
      |
      v
Volume Overlay
      |
      v
Background Screen
```

Only volume-related controls should affect the overlay.

---

## Touch interaction

Supported:

* tap bar position
* drag volume slider

Optional.

---

## Back button

Behavior:

* closes overlay
* does not modify volume

---

# 12. Animation

Animations must be extremely lightweight.

## Opening

Recommended:

* fade in
* slight upward movement

Duration:

100-150ms

---

## Updating

When value changes:

* update indicator immediately
* animate only if cheap

Do not animate every percentage step.

---

## Closing

Fade out:

100ms

---

Avoid:

* large scaling
* blur
* bouncing
* glowing effects

---

# 13. Performance budget

Volume overlay is one of the most frequently used overlays.

Requirements:

* Response latency:
  <50ms

* Rendering:
  partial redraw only

* CPU impact:
  negligible

* Memory:
  static allocation preferred

---

Optimization recommendations:

* Keep overlay surface cached.
* Update only changed volume region.
* Avoid font regeneration.
* Avoid loading icons dynamically.

---

# 14. Theme integration

The overlay follows current application context.

## Music player

Use:

* album-art-derived accent
* dark readable card

---

## System screens

Use:

* wallpaper-derived colors

---

## Third-party apps

Use:

* app theme if available
* fallback system style

---

Readability has priority over theme matching.

---

# 15. Audio backend integration

The UI must communicate through an abstraction layer.

Recommended:

```text id="0y2rjk"
Volume UI

    |
    v

Audio Manager

    |
    +---- Software Volume
    |
    +---- DSP Volume
    |
    +---- Hardware Output
```

Do not allow UI code to directly access USB audio commands.

---

# 16. Special TANCHJIM BUNNY DSP considerations

The DSP may expose:

* hardware volume
* DSP profiles
* gain states

Future support:

```text
Volume

+
Gain Mode

+
DSP Profile
```

However:

Do not add these into the normal volume overlay.

Keep advanced audio controls inside:

Settings → Audio

---

# 17. Error handling

## DSP disconnected

Show:

```text
Audio Device Lost

Volume unavailable
```

Use notification/dialog depending on severity.

---

## Volume command failure

Behavior:

* keep displayed value
* retry once
* report failure
* avoid UI freeze

---

## Invalid volume state

Example:

* value >100
* negative value

Fallback:

```text
Normalize to valid range
```

---

# 18. Acceptance criteria

Implementation is complete when:

* volume changes feel instant
* overlay appears globally
* playback never stops
* encoder input works correctly
* touchscreen control works if enabled
* volume value updates correctly
* mute preserves previous volume
* DSP communication is abstracted
* rendering remains smooth on Pi Zero W
* overlay does not interfere with screens

---

# 19. Future improvements

Possible additions:

* volume curve customization
* headphone impedance profiles
* per-device volume memory
* left/right balance
* replay gain support
* DSP preset indicator
* physical encoder acceleration

---

# 20. Architectural recommendations

## Volume Manager

Create:

```text
Volume Manager

├── getVolume()
├── setVolume()
├── increase()
├── decrease()
├── toggleMute()
├── saveState()
└── notifyChange()
```

---

## State model

Example:

```text
Volume State

current_volume
previous_volume
muted
audio_device
hardware_volume_supported
```

---

## Event system

Volume changes should emit:

```text
VolumeChangedEvent
```

Consumed by:

* Volume Overlay
* Mini Player
* Full Player
* Settings

---

# 21. Implementation notes for AI coding agent

When implementing:

* Treat this as a global system overlay.
* Prioritize physical encoder support.
* Keep volume changes extremely responsive.
* Separate UI volume state from audio hardware implementation.
* Never block rendering while communicating with DSP.
* Avoid mobile-style volume controls.

The final result should feel like a premium dedicated audio player: immediate, tactile, and invisible when not needed.
