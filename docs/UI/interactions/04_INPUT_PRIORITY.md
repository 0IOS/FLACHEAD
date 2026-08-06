# 04_INPUT_PRIORITY.md

# FLACHEAD UI — Input Priority Specification

## 1. Purpose

This document defines how FLACHEAD handles multiple input sources.

FLACHEAD has several control methods:

* touchscreen
* physical buttons
* volume dial
* background hardware events

A clear priority system is required to prevent conflicts and ensure predictable behavior.

Primary principle:

> The user should always know which control method has authority.

---

# 2. Input sources

FLACHEAD input sources:

```text id="x8m4q2"
1. Physical Buttons

2. Touchscreen

3. Volume Dial

4. System Events

5. External Hardware Events
```

---

# 3. Input architecture

Data flow:

```text id="m5x8q3"
Input Sources

↓

Input Manager

↓

Priority Resolver

↓

Event System

↓

UI / Playback Services
```

---

No input device communicates directly with UI screens.

---

# 4. Priority hierarchy

Default priority:

```text id="q7m3x9"
1. Physical Playback Controls

2. Physical Navigation Controls

3. Touch Input

4. Software Events

5. Background Events
```

---

Reason:

FLACHEAD is a hardware music player.

---

# 5. Playback priority

Playback controls always have highest priority.

Examples:

During:

* settings
* library browsing
* dialogs

PLAY button:

```text id="v5m8x2"
Toggle Playback
```

---

FORWARD:

```text id="c8m4x7"
Next Track
```

---

BACKWARD:

```text id="r6m3x5"
Previous Track
```

---

# 6. Volume priority

Volume dial:

Always active.

Works during:

* playback
* settings
* dialogs
* scanning

---

The volume overlay appears temporarily.

---

Volume changes must never:

* block UI
* pause playback
* interrupt DSP output

---

# 7. Navigation priority

Navigation controls depend on current state.

Normal:

```text id="n8m4x6"
UP/DOWN

↓

Selection Movement
```

---

Dialog:

```text id="u7m3x9"
UP/DOWN

↓

Dialog Selection
```

---

Text input:

```text id="k5m8x2"
UP/DOWN

↓

Character Selection
```

---

# 8. Touch priority

Touch input is active when:

* no hardware override exists
* screen is interactive

---

Example:

User touches a list.

Result:

```text id="a6m8x4"
Touch Selection
```

---

If a hardware button is pressed simultaneously:

Hardware wins.

---

# 9. Conflict handling

Example:

User:

* swipes list
* presses DOWN button

Result:

```text id="w8m3x5"
Button Event Processed

Swipe Cancelled
```

---

Reason:

Avoid unpredictable UI movement.

---

# 10. Background events

Examples:

* DSP connection change
* SD card removal
* scan completion
* battery events

---

Priority:

Lowest.

---

Background events should:

* notify user
* avoid interrupting actions

---

Example:

During playback:

SD card warning:

Show notification.

Do not force screen change.

---

# 11. Modal states

Certain screens temporarily capture input.

Examples:

* confirmation dialogs
* text input
* setup wizard

---

Input routing:

```text id="p9m4x7"
Current Modal

↓

Input Manager

↓

Modal Handler
```

---

Playback controls remain global.

---

# 12. Lock screen behavior

Future:

Locked state:

Allowed:

* volume
* play/pause
* next/previous

Restricted:

* navigation
* settings changes

---

# 13. Debouncing and timing

All hardware input requires:

* debounce
* event filtering
* repeat handling

---

Recommended:

Button debounce:

20-50ms

Gesture filtering:

10ms+

---

# 14. Performance requirements

Input processing:

Target:

```text id="z6m3x8"
<10ms
```

---

Must not:

* block renderer
* affect audio thread
* create frame drops

---

# 15. Error handling

Input device failure:

Fallback:

```text id="h5m8x2"
Remaining inputs continue
```

---

Touch failure:

Buttons remain functional.

---

Button failure:

Touch remains functional.

---

# 16. Acceptance criteria

Implementation is complete when:

* input conflicts are predictable
* hardware controls always work
* volume is always accessible
* modal screens behave correctly
* no input source freezes UI

---

# 17. Architectural recommendations

Required service:

```text id="d7m4x9"
Input Manager

├── Touch Handler

├── Button Handler

├── Dial Handler

├── Priority Resolver

└── Event Dispatcher
```

---

# 18. Implementation notes for AI coding agent

When implementing:

* Never handle input directly inside screens.
* Keep input routing centralized.
* Prioritize hardware controls.
* Preserve playback control globally.
* Make new input devices easy to add.

The input system is what makes FLACHEAD feel like a dedicated audio device instead of a touchscreen application.
