# 01_TOUCH_INTERACTION.md

# FLACHEAD UI — Touch Interaction Specification

## 1. Purpose

This document defines the touchscreen interaction rules for FLACHEAD.

FLACHEAD uses a small 2.8" portrait touchscreen combined with physical controls.

The touchscreen should provide:

* fast navigation
* simple interaction
* reliable input
* minimal accidental presses

Primary principle:

> Touch should feel like controlling a dedicated audio device, not using a smartphone.

---

# 2. Design goals

Touch interaction must be:

* predictable
* responsive
* low latency
* optimized for small displays
* usable with one hand

---

# 3. Hardware assumptions

Target:

```text
Display:

2.8 inch TFT

Orientation:

Portrait

Input:

Resistive/Capacitive touchscreen
```

---

Recommended resolution:

```text
320 x 240
```

---

# 4. Touch architecture

Data flow:

```text
Touch Event

↓

Input Manager

↓

Gesture Recognition

↓

UI Event System

↓

Screen Component

↓

Action
```

---

The UI must not directly process raw touch events.

All input goes through:

```text
Input Manager
```

---

# 5. Touch zones

Every interactive element requires:

* visible area
* touch area
* minimum size

---

Minimum touch target:

```text
44 x 44 pixels
```

---

Preferred:

```text
50 x 50 pixels
```

---

Reason:

Small screens increase input errors.

---

# 6. Tap interaction

Single tap:

Used for:

* opening screens
* selecting tracks
* toggles
* buttons

Response requirement:

```text
<100ms visual feedback
```

---

Example:

User taps:

```text
Track Row

↓

Highlight

↓

Open Player
```

---

# 7. Long press

Long press is limited.

Allowed:

* advanced actions
* contextual menus
* additional information

Duration:

```text
700ms
```

---

Avoid using long press for:

* primary navigation
* playback controls

---

# 8. Swipe gestures

Supported:

## Vertical swipe

Used for:

* scrolling lists
* lyrics movement
* library browsing

---

## Horizontal swipe

Limited usage.

Possible:

* player navigation
* album switching

---

Avoid:

* hidden actions
* critical controls

---

# 9. Touch feedback

Every touch action requires feedback.

Allowed:

* highlight
* small color change
* subtle animation

Duration:

```text
100-150ms
```

---

Avoid:

* vibration simulation
* heavy effects

---

# 10. Touch and physical controls priority

Priority order:

```text
Physical Hardware

↓

Touch Input

↓

Background Events
```

---

Reason:

Physical controls are essential for a dedicated music player.

---

# 11. Music player touch rules

Player screen:

Large controls.

Required:

* play/pause
* next
* previous
* seek
* favorite

---

Avoid:

tiny buttons.

---

# 12. Mini player touch rules

Mini player:

Tap:

Open full player.

---

Swipe:

Optional queue preview.

---

Long press:

No action initially.

---

# 13. Scroll behavior

Lists must use:

* momentum scrolling
* bounded movement
* lazy rendering

---

Required:

```text
60 FPS preferred

30 FPS minimum
```

---

# 14. Accidental touch prevention

Prevent:

* edge misclicks
* double activation
* accidental scrolling

Methods:

* debounce input
* minimum movement threshold
* clear spacing

---

# 15. Touch calibration

Future support:

```text
Settings

↓

Display

↓

Touch Calibration
```

---

Calibration data stored in:

```text
Configuration Service
```

---

# 16. Error handling

Touch failure:

Fallback:

```text
Physical controls remain functional
```

---

Incorrect calibration:

Allow recalibration.

---

# 17. Performance requirements

Touch processing must:

* avoid blocking renderer
* use lightweight event handling
* avoid excessive gesture calculations

---

Input latency target:

```text
<50ms
```

---

# 18. Acceptance criteria

Implementation is complete when:

* touch navigation works
* controls are easy to press
* scrolling is smooth
* accidental presses are minimized
* physical controls still work

---

# 19. Architectural recommendations

Required component:

```text
Input Manager

├── Touch Events

├── Button Events

├── Gesture Detection

├── Input Priority

└── Event Dispatch
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Never bind UI actions directly to touchscreen coordinates.
* Use reusable interaction components.
* Keep touch logic independent from screens.
* Optimize for 2.8" display size.
* Always provide physical control alternatives.

FLACHEAD should remain usable even when the touchscreen is not the preferred input method.
