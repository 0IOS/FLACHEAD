# 03_GESTURE_SYSTEM.md

# FLACHEAD UI — Gesture System Specification

## 1. Purpose

This document defines gesture behavior within FLACHEAD.

Gestures provide additional touchscreen interaction while keeping the interface simple and predictable.

Gestures are optional enhancements and must never replace essential controls.

Primary principle:

> Every important action must remain available through buttons or direct touch controls.

---

# 2. Design goals

The gesture system must be:

* minimal
* discoverable
* consistent
* low CPU usage
* optimized for a 2.8" display

---

# 3. Gesture architecture

Data flow:

```text id="x7m4q9"
Touch Events

↓

Gesture Detector

↓

Gesture Recognition

↓

Input Manager

↓

UI Action
```

---

Gesture processing must happen independently from screen rendering.

---

# 4. Supported gestures

Initial supported gestures:

```text id="m5x8q2"
Tap

Long Press

Swipe

Double Tap
```

---

Avoid complex gestures.

Not supported:

* pinch zoom
* multi-touch controls
* rotating gestures

Reason:

Small screen + dedicated player design.

---

# 5. Tap gesture

## Single Tap

Primary interaction.

Used for:

* selecting items
* opening screens
* toggling controls

---

Requirements:

Response:

```text id="q8m3x5"
<100ms
```

---

# 6. Double Tap

Used only for high-value actions.

Supported:

## Home area

Double tap:

```text id="v6m3x8"
Return Home
```

---

## Player

Optional:

Toggle expanded controls.

---

Timing:

```text id="c7m4x9"
300-500ms
```

---

Avoid:

Using double tap everywhere.

---

# 7. Long Press

Duration:

```text id="r5m8x2"
700ms
```

---

Allowed uses:

## Track items

Open context menu.

Example:

```text id="n8m3x6"
Play Next

Add Queue

Add Playlist
```

---

## Settings

Open advanced options.

---

Not allowed:

Critical navigation.

---

# 8. Swipe gestures

## Vertical Swipe

Primary scrolling gesture.

Used in:

* music library
* playlists
* settings
* lyrics

---

Behavior:

* momentum scrolling
* bounded movement
* smooth deceleration

---

## Horizontal Swipe

Limited use.

Allowed:

### Full Player

Swipe left:

Next track.

Swipe right:

Previous track.

---

This is optional.

Physical buttons remain primary.

---

# 9. Gesture thresholds

Recommended:

Minimum swipe distance:

```text id="u6m3x7"
40 pixels
```

---

Minimum velocity:

```text id="p8m4x2"
150 px/s
```

---

Purpose:

Prevent accidental gestures.

---

# 10. Gesture conflicts

Priority:

```text id="k7m3x9"
Button Input

↓

Tap

↓

Swipe

↓

Long Press
```

---

Example:

A scroll gesture should not trigger a button.

---

# 11. Music player gestures

Supported:

Swipe artwork:

* next/previous track

Tap artwork:

* show player controls

Double tap:

* favorite track

---

Optional feature.

Can be disabled.

---

# 12. Lyrics gestures

Supported:

Vertical swipe:

Scroll lyrics.

---

Tap:

Center current line.

---

Long press:

No action.

---

# 13. List gestures

Supported:

Swipe:

Scroll.

---

Tap:

Open item.

---

Long press:

Context menu.

---

# 14. Gesture feedback

Allowed:

* pressed state
* highlight
* subtle movement

---

Avoid:

* large animations
* bounce effects

---

# 15. Performance requirements

Gesture processing:

Must not affect:

* audio playback
* rendering FPS

---

Target:

```text id="z5m8x3"
Gesture recognition <10ms
```

---

# 16. Error handling

Invalid gesture:

Ignore.

---

Gesture detector failure:

Fallback to:

* touch buttons
* physical controls

---

# 17. Acceptance criteria

Implementation is complete when:

* scrolling works smoothly
* gestures do not conflict
* important actions have alternatives
* CPU usage remains low
* touch feels responsive

---

# 18. Architectural recommendations

Required service:

```text id="a8m4x6"
Gesture Manager

├── Detect Gesture

├── Calculate Direction

├── Apply Thresholds

├── Resolve Conflicts

└── Dispatch Event
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Keep gestures optional.
* Never hide important actions behind gestures.
* Optimize for small screen usage.
* Prefer simple gestures over smartphone-like interactions.
* Test with real hardware.

FLACHEAD should feel intentional: gestures enhance the experience but never complicate it.
