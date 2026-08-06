# 25_BRIGHTNESS_OVERLAY.md

# FLACHEAD UI — Brightness Overlay

## 1. Purpose

The Brightness Overlay provides immediate control and feedback for display brightness adjustments.

Although FLACHEAD is primarily a music player, the 2.8" TFT touchscreen requires a simple and accessible brightness system for:

* comfortable viewing in different environments
* reducing power consumption
* preventing unnecessary screen usage
* supporting future display hardware variations

The brightness system must remain lightweight and should never interfere with playback.

---

# 2. Design goals

The Brightness Overlay must:

* Appear instantly when brightness changes.
* Work globally across all screens.
* Support touchscreen interaction.
* Support physical controls if mapped.
* Avoid heavy rendering.
* Preserve the music-first experience.
* Allow fast screen adjustments without entering settings.

Primary principle:

> Display controls should be immediate system actions, not buried configuration options.

---

# 3. Architecture

Brightness is a system-level overlay.

It should not belong to any specific application.

Architecture:

```text
Brightness Input
        |
        v
Input Manager
        |
        v
Display Service
        |
        v
Brightness State Manager
        |
        v
Overlay Manager
        |
        v
Brightness Renderer
```

---

# 4. Dependencies

The Brightness Overlay depends on:

* Overlay Manager
* Input Manager
* Display Service
* Configuration Service
* Theme Engine
* Power Management Service
* Screen Driver Interface

Recommended separation:

```text
UI Layer

Brightness Overlay

        |

System Layer

Display Controller

        |

Hardware Layer

TFT Driver / Backlight GPIO / PWM
```

The UI should never directly control GPIO or PWM.

---

# 5. Brightness control sources

## 5.1 Touchscreen

Primary software method.

Possible interactions:

* tap brightness slider
* drag brightness level
* quick adjustment gesture (future)

---

## 5.2 Hardware buttons

Optional mapping:

```text
Button shortcut:

UP + SELECT
        |
        v
Brightness increase


DOWN + SELECT
        |
        v
Brightness decrease
```

The exact shortcut should remain configurable.

---

## 5.3 Automatic brightness

Future support only.

Possible inputs:

* ambient light sensor
* power state
* battery mode

Not required for initial FLACHEAD hardware.

---

# 6. Layout

The overlay should appear near the bottom of the display, similar to the volume overlay.

Preferred layout:

```text
┌─────────────────────┐
│                     │
│    Current Screen   │
│                     │
│                     │
│ ┌─────────────────┐ │
│ │ ☀ Brightness    │ │
│ │                 │ │
│ │ █████░░░░░░     │ │
│ │                 │ │
│ │       50%       │ │
│ └─────────────────┘ │
└─────────────────────┘
```

---

# 7. Visual elements

The overlay contains:

## Brightness icon

States:

* low brightness
* medium brightness
* high brightness

The icon should be static or selected from a small cached set.

---

## Brightness indicator

Recommended:

Horizontal progress bar.

Reasons:

* low rendering cost
* readable on small display
* shared implementation with volume overlay

---

## Numeric value

Display:

```text
0-100%
```

Optional future:

```text
Night Mode
```

---

# 8. Brightness states

Internal representation:

```text
0-100 integer scale
```

Example:

```text
0
|
Minimum visible brightness
|
50
|
Normal brightness
|
100
|
Maximum brightness
```

The UI should not assume the hardware brightness curve is linear.

Mapping:

```text
FLACHEAD Brightness Value

        |

Display Service

        |

Hardware PWM / Backlight Controller
```

---

# 9. Appearance timing

Overlay appears when:

* brightness changes
* display profile changes
* night mode activates
* power mode changes brightness

Visibility:

Normal adjustment:

2 seconds

Continuous adjustment:

Timer resets while changing.

Example:

```text
Increase brightness

Overlay visible

Increase again

Timer resets

Stop

Overlay disappears
```

---

# 10. Interaction

## Touch

Supported:

* tap slider position
* drag brightness level

---

## Hardware input

Recommended:

```text
UP

Increase brightness


DOWN

Decrease brightness


SELECT

Confirm temporary brightness mode
```

---

## Back button

Behavior:

* dismiss overlay
* keep current brightness

---

# 11. Animation

Animations must remain simple.

Opening:

* fade in
* small upward movement

Duration:

100-150ms

Updating:

* indicator updates immediately

Closing:

* fade out

Duration:

100ms

Avoid:

* glowing effects
* animated sun icons
* complex transitions

---

# 12. Performance budget

Brightness Overlay must be extremely cheap.

Requirements:

Response:

<50ms

Rendering:

partial redraw only

Memory:

static allocation preferred

CPU impact:

negligible

---

Optimization:

* cache overlay surface
* reuse progress bar component
* avoid recalculating layout
* avoid loading assets during adjustment

---

# 13. Theme integration

Brightness Overlay follows the system theme.

## Home/System

Uses:

* wallpaper-derived colors
* system accent

---

## Music Player

Uses:

* album-art-derived colors

---

## Third-party applications

Uses:

* application theme if available
* fallback system styling

---

Brightness controls must always maintain readability.

---

# 14. Power management integration

Brightness affects battery usage.

The Display Service should communicate with:

* Power Manager
* Battery Monitor

Example:

```text
Battery low

        |

Power Manager

        |

Reduce brightness suggestion
```

Important:

Automatic reduction should not happen without user preference unless critical.

---

# 15. Error handling

## Display controller unavailable

Behavior:

* keep current value
* log error
* show notification

---

## Brightness command failure

Behavior:

* retry once
* keep UI responsive
* report failure

---

## Invalid brightness value

Example:

```text
-10%
150%
```

Normalize:

```text
0-100%
```

---

# 16. Acceptance criteria

Implementation is complete when:

* brightness changes instantly
* overlay works globally
* playback continues unaffected
* touch interaction works
* hardware controls can be mapped
* brightness state persists correctly
* display service is separated from UI
* rendering remains smooth on Pi Zero W
* no unnecessary screen redraw occurs

---

# 17. Future improvements

Possible additions:

* automatic brightness
* night mode
* scheduled brightness profiles
* screen timeout integration
* OLED-specific brightness handling
* per-app brightness profiles

---

# 18. Architectural recommendations

## Display Service

Create:

```text
Display Service

├── getBrightness()
├── setBrightness()
├── increase()
├── decrease()
├── saveState()
└── applyProfile()
```

---

## Brightness State Model

Example:

```text
Brightness State

current_level

previous_level

auto_mode

night_mode

display_profile
```

---

## Event System

Brightness changes emit:

```text
BrightnessChangedEvent
```

Consumed by:

* Brightness Overlay
* Settings
* Power Manager

---

# 19. Implementation notes for AI coding agent

When implementing:

* Build this as a global overlay.
* Reuse the Volume Overlay architecture where possible.
* Keep display hardware abstraction separate.
* Do not directly control TFT hardware from UI code.
* Prioritize fast response over visual effects.
* Maintain compatibility with Raspberry Pi Zero W limitations.

The final implementation should feel like an embedded premium audio device: fast, predictable, and invisible when not needed.
