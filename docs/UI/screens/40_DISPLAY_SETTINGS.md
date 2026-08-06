# 40_DISPLAY_SETTINGS.md

# FLACHEAD UI — Display Settings Screen Specification

## 1. Purpose

The Display Settings Screen controls the visual behavior of FLACHEAD.

It manages:

* screen brightness
* theme behavior
* animation preferences
* display timeout
* orientation configuration
* visual performance options

The screen is designed around the limitations of the Raspberry Pi Zero W and the 2.8" TFT display.

Primary principle:

> The display should look premium while consuming as little power and processing as possible.

---

# 2. Design goals

The Display Settings Screen must be:

* simple
* responsive
* power efficient
* safe for small screens
* optimized for long playback sessions

---

# 3. Dependencies

Required systems:

* Display Manager
* Theme Engine
* Animation Manager
* Configuration Service
* Power Manager
* Screen Renderer
* Brightness Controller
* Screen Manager

---

# 4. Architecture

Data flow:

```text id="x8m4q2"
User Setting

↓

Configuration Service

↓

Display Manager

↓

Renderer / Hardware

↓

Screen Update
```

---

The UI does not directly control:

* framebuffer
* backlight hardware
* rendering pipeline

---

# 5. Screen layout

Portrait layout:

```text id="q7m5x8"
+----------------------+
| < Back               |
|                      |
| Display Settings     |
|                      |
| Brightness           |
| ███████░░ 70%        |
|                      |
| Theme                |
| System               |
|                      |
| Animations           |
| Enabled              |
|                      |
| Timeout              |
| 30 Seconds           |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Brightness control

Controls:

* screen brightness level
* automatic brightness (future)

Range:

```text id="m4x8q3"
0% - 100%
```

---

Default:

70%

Reason:

Balance between:

* visibility
* power consumption

---

# 7. Brightness behavior

Brightness changes:

* apply immediately
* persist after reboot

---

No confirmation required.

---

# 8. Theme settings

Available:

## System Theme

Uses:

```text id="v5m8x2"
Wallpaper-derived colors
```

---

## Music Theme

Uses:

```text id="n8m3q7"
Album artwork-derived colors
```

---

## Custom Theme

Future support.

---

# 9. Animation settings

Controls:

* transitions
* fades
* UI effects

Options:

```text id="c6m4x9"
Full

Reduced

Off
```

---

Recommended default:

Reduced.

Reason:

* Pi Zero W performance
* battery savings
* faster interaction

---

# 10. Display timeout

Controls:

When display dims or turns off.

Options:

```text id="w7m2x5"
Never

15 Seconds

30 Seconds

1 Minute

5 Minutes
```

---

Behavior:

During playback:

Optional dim mode.

---

# 11. Orientation

FLACHEAD is portrait-only.

Display:

```text id="r8m4x6"
Portrait Locked
```

---

No rotation controls.

---

# 12. Performance mode

Advanced option.

Controls visual workload.

Options:

```text id="p5m8x3"
Balanced

Performance

Power Saving
```

---

## Balanced

Default.

---

## Performance

Prioritizes:

* animations
* refresh rate

---

## Power Saving

Reduces:

* animation
* refresh frequency

---

# 13. Touch calibration

Future option.

Accessible through:

```text id="k7m3x9"
Advanced Display
```

---

Used for:

* touchscreen alignment
* rotation correction

---

# 14. Physical control support

UP/DOWN:

Navigate options.

SELECT:

Change value.

BACK:

Return.

HOME:

Launcher.

---

# 15. Theme behavior

The Display Settings screen itself uses:

```text id="b4m8x7"
System Theme
```

---

Preview:

Optional.

Avoid expensive live previews.

---

# 16. Animations

Allowed:

* slider movement
* toggle transition
* small fades

Duration:

100-200ms

---

Avoid:

* live wallpaper preview
* real-time color processing

---

# 17. Performance requirements

Targets:

Opening:

<200ms

Setting change:

instant

Memory:

minimal

---

Display settings must not:

* reload UI unnecessarily
* restart renderer
* interrupt playback

---

# 18. Error handling

Brightness unavailable:

Fallback:

```text id="y6m3x8"
Hardware brightness control unavailable
```

---

Invalid configuration:

Reset to safe defaults.

---

Renderer failure:

Attempt recovery.

---

# 19. Acceptance criteria

Implementation is complete when:

* brightness works
* settings persist
* themes switch correctly
* animations can be reduced
* display remains stable during playback

---

# 20. Architectural recommendations

Required service:

```text id="u8m4x2"
Display Manager

├── Brightness Control

├── Theme Selection

├── Animation Settings

├── Timeout Handling

└── Performance Mode
```

---

Required components:

```text id="s5m7x3"
SettingRow

Slider

Toggle

Dropdown

SectionHeader
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Keep display settings independent from rendering internals.
* Never restart the UI for small changes.
* Apply changes dynamically.
* Prioritize responsiveness.
* Keep GPU/CPU usage low.

The Display Settings system should allow FLACHEAD to feel polished while respecting the hardware limits of the Raspberry Pi Zero W.
