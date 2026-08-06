# 02_PHYSICAL_BUTTON_MAPPING.md

# FLACHEAD UI — Physical Button Mapping Specification

## 1. Purpose

This document defines the behavior of FLACHEAD's physical hardware controls.

Physical controls are a core part of the FLACHEAD experience.

The device should remain fully usable without relying on the touchscreen.

Primary principle:

> A dedicated music player must be controllable by touch, but should never depend on touch.

---

# 2. Hardware control assumptions

FLACHEAD contains:

```text id="h7m3q8"
Navigation:

UP

DOWN

SELECT


System:

HOME

BACK


Playback:

PLAY/PAUSE

FORWARD

BACKWARD


Volume:

Physical Dial
```

---

# 3. Input architecture

Data flow:

```text id="q8m5x2"
GPIO Event

↓

Input Manager

↓

Button Mapper

↓

UI Event System

↓

Screen Action
```

---

Physical buttons must not directly call screen functions.

---

# 4. Button priority

Priority:

```text id="x4m8q3"
Hardware Buttons

↓

Touch Input

↓

Software Events
```

---

If both occur:

Physical input wins.

---

# 5. Navigation buttons

## UP

Default action:

Move selection upward.

Used in:

* lists
* settings
* menus
* queues

---

Hold behavior:

Continuous scrolling.

Repeat delay:

```text id="m5q8x1"
500ms
```

---

## DOWN

Default action:

Move selection downward.

Used everywhere.

---

Hold behavior:

Continuous scrolling.

---

## SELECT

Default:

Confirm current selection.

Actions:

* open screen
* play track
* toggle option
* confirm dialog

---

# 6. System buttons

## HOME Button

Actions:

### Single press

Open launcher.

---

### Double press

Return to home screen.

Timing:

```text id="v8m4x6"
<500ms
```

---

### Hold

Open task overview.

Duration:

```text id="p7m3x9"
1000ms
```

---

## BACK Button

Universal action:

Return to previous screen.

---

If in:

Dialog:

Close dialog.

---

If playing:

Never stop playback.

---

# 7. Playback controls

## PLAY/PAUSE

Single press:

Toggle playback.

---

When paused:

Resume.

---

When playing:

Pause.

---

## FORWARD

Single press:

Next track.

---

Long press:

Seek forward.

---

## BACKWARD

Single press:

Previous track.

---

Long press:

Seek backward.

---

# 8. Volume dial

The volume dial is independent from UI navigation.

Actions:

Rotate:

Change volume.

---

Requirements:

* instant response
* no screen loading
* works during playback

---

Display overlay:

When adjusted:

```text id="w5m8x2"
Volume

70%
```

---

# 9. Button debounce

Required.

Reason:

Mechanical switches can produce multiple signals.

---

Debounce time:

```text id="c8m3x7"
20-50ms
```

---

# 10. Button states

Supported:

```text id="n4m8x6"
Pressed

Released

Held

Repeated
```

---

# 11. Screen-independent behavior

Buttons must work globally.

Example:

Music playing:

User enters settings.

PLAY button:

Still controls playback.

---

# 12. Dialog behavior

Buttons:

UP/DOWN:

Change selection.

SELECT:

Confirm.

BACK:

Cancel.

---

Example:

```text id="r6m3x8"
Delete playlist?

[Cancel]

[Confirm]
```

---

# 13. Lock screen behavior

Future:

Physical playback controls remain active.

Navigation buttons may unlock device.

---

# 14. Error handling

Button failure:

System should:

* log error
* continue touchscreen operation

---

Input manager failure:

Attempt restart.

---

# 15. Performance requirements

Button response:

```text id="u7m4x2"
<50ms
```

---

No button event should:

* block rendering
* interrupt audio

---

# 16. Acceptance criteria

Implementation is complete when:

* every button has defined behavior
* controls work globally
* playback works without touch
* long press actions work
* debounce prevents duplicates

---

# 17. Architectural recommendations

Required service:

```text id="d8m5x9"
Button Manager

├── GPIO Reading

├── Debouncing

├── Long Press Detection

├── Mapping

└── Event Dispatch
```

---

# 18. Implementation notes for AI coding agent

When implementing:

* Keep button mapping configurable.
* Separate hardware GPIO code from UI.
* Use events instead of direct calls.
* Prioritize playback controls.
* Test without touchscreen connected.

FLACHEAD should feel like a physical audio device first and a touchscreen device second.
