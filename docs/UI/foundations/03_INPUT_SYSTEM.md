# 03_INPUT_SYSTEM.md

# FLACHEAD UI — Input System Foundation

## 1. Purpose

The Input System defines how FLACHEAD receives, processes, prioritizes, and routes user interactions.

FLACHEAD is designed as a dedicated digital audio player, not a touchscreen tablet.

The input architecture must support:

* 2.8" touchscreen interaction
* physical navigation buttons
* playback controls
* rotary volume control
* future hardware controls

The system must allow operation even when one input method is unavailable.

Primary principle:

> Hardware controls are first-class inputs, not secondary shortcuts.

---

# 2. Design goals

The Input System must:

* Provide instant response.
* Support multiple input devices.
* Separate hardware detection from UI logic.
* Allow global controls anywhere.
* Respect lock states.
* Prevent conflicting inputs.
* Remain lightweight on Raspberry Pi Zero W.

The Input System should never:

* directly control screens
* contain business logic
* depend on specific applications

---

# 3. Input architecture

Input follows a layered design.

```text id="v8pk0s"
Hardware Input

(Button / Touch / Encoder)

        |

        v

Input Drivers

        |

        v

Input Manager

        |

        v

Input Event System

        |

        v

Navigation / Screens / Services
```

---

# 4. Supported input devices

## 4.1 Touchscreen

Primary visual interaction method.

Used for:

* selecting items
* scrolling
* adjusting values
* navigation

Capabilities:

* tap
* long press
* swipe
* drag

---

## 4.2 Physical buttons

Recommended controls:

```text id="p4w0l8"
UP

DOWN

SELECT

BACK

HOME
```

Used for:

* navigation
* playback control
* system shortcuts

---

## 4.3 Playback buttons

Dedicated music controls:

```text id="m1q7as"
PLAY / PAUSE

NEXT

PREVIOUS
```

These should work globally.

---

## 4.4 Rotary encoder

Used primarily for:

* volume control

Possible future:

* scrolling
* menu navigation

---

# 5. Hardware abstraction

UI must not know hardware details.

Bad:

```text id="i7r0da"
Music Player

↓

GPIO Read

↓

Button Press
```

Good:

```text id="2w9fpm"
GPIO Driver

↓

Input Manager

↓

ButtonPressed Event
```

---

# 6. Input event model

All input becomes standardized events.

Example:

```text id="0w4m2n"
Input Event

type

source

action

timestamp

state
```

---

Examples:

```text id="q7k4n8"
Touch Tap

Button Press

Encoder Rotation

Long Press
```

---

# 7. Event types

Recommended events:

## Navigation

```text id="6b9v3x"
NAV_UP

NAV_DOWN

NAV_SELECT

NAV_BACK

NAV_HOME
```

---

## Playback

```text id="d9y5m4"
PLAY_PAUSE

NEXT_TRACK

PREVIOUS_TRACK
```

---

## System

```text id="f2w9p1"
LOCK

UNLOCK

BRIGHTNESS_CHANGE

VOLUME_CHANGE
```

---

# 8. Input priority system

Some inputs must override others.

Priority:

```text id="n8g4p0"
1. Emergency/System

        |

2. Hardware Playback

        |

3. Navigation

        |

4. Screen Interaction

        |

5. Application Input
```

---

Example:

While music player is running:

Volume knob always works.

A dialog cannot block volume control.

---

# 9. Global controls

Some controls exist everywhere.

Global:

```text id="j0x4m8"
Volume

Play/Pause

Next Track

Previous Track

Home

Back
```

These are handled before screen input.

---

# 10. Navigation system

Navigation is centralized.

Architecture:

```text id="p7z6yx"
Input Event

        |

Navigation Manager

        |

Screen Manager

        |

Current Screen
```

Screens do not decide navigation rules.

---

# 11. Home button behavior

Defined FLACHEAD behavior:

## Tap HOME

Open:

```text id="6q2x7p"
App Launcher
```

---

## Double Tap HOME

Open:

```text id="a4k7s1"
Home Screen
```

---

## Hold HOME

Open:

```text id="m0w5a8"
Task Overview
```

---

# 12. Back button behavior

Universal:

```text id="s4x7p2"
BACK

↓

Current State

↓

Previous State
```

Priority:

1. Close dialog
2. Close overlay
3. Exit screen
4. Return previous screen

---

# 13. Touch handling

Touch events follow:

```text id="t1x8c5"
Touch Input

↓

Coordinate Mapping

↓

Gesture Detection

↓

Input Event

↓

Target Component
```

---

Supported gestures:

## Tap

Selection.

---

## Long press

Context actions.

---

## Swipe

Scrolling.

---

Avoid:

* multi-touch complexity

Reason:

Small display size.

---

# 14. Rotary encoder behavior

Default:

```text id="j3v8y6"
Rotate

↓

Volume Change
```

---

Optional future:

Context-based operation:

```text id="7r0zq5"
Music Library

Encoder = Scroll


Settings

Encoder = Adjust Value
```

---

# 15. Lock system integration

Input filtering occurs before UI.

Architecture:

```text id="c9w1m2"
Input Event

        |

Lock Manager

        |

Allowed?

        |

+-------------+

YES          NO

 |            |

UI         Ignore
```

---

Allowed while locked:

```text id="h4p0s9"
Volume

Playback

Unlock
```

---

Blocked:

```text id="a6m3z8"
Settings

Delete

Navigation
```

---

# 16. Input timing

Important for responsiveness.

Targets:

Button response:

<50ms

Touch response:

<100ms

Encoder response:

instant

---

# 17. Debouncing

Physical buttons require filtering.

Example:

```text id="r5m8k1"
Mechanical Bounce

↓

Debouncer

↓

Single Event
```

---

Recommended:

10-50ms debounce.

---

# 18. Long press detection

Example:

```text id="x5p8v2"
Button Down

↓

Timer

↓

Threshold reached

↓

Long Press Event
```

---

Recommended:

500-800ms.

---

# 19. Input errors

## Button disconnected

Continue:

* touchscreen operation

---

## Touch failure

Continue:

* hardware controls

---

## Encoder failure

Continue:

* touchscreen volume

---

## Complete failure

Enter:

Recovery Input Mode

---

# 20. Performance requirements

Input processing must be:

* event-driven
* lightweight
* non-blocking

Avoid:

* polling every frame
* unnecessary allocations

---

# 21. Acceptance criteria

Implementation is complete when:

* all hardware inputs are abstracted
* touchscreen works
* buttons work
* encoder works
* global controls work
* lock filtering works
* navigation rules are centralized
* input latency is low
* failures degrade gracefully

---

# 22. Future improvements

Possible additions:

* custom button mapping
* Bluetooth remote support
* headphone controls
* gesture customization
* external controllers

---

# 23. Architectural recommendations

## Input Manager

Create:

```text id="b2k7x4"
Input Manager

├── registerDevice()
├── processEvent()
├── dispatchEvent()
├── applyFilters()
├── handleGlobalInput()
└── updateState()
```

---

## Device drivers

Separate:

```text id="y6p3m8"
Touch Driver

Button Driver

Encoder Driver

GPIO Driver

USB Controller Driver
```

---

## Event bus integration

Flow:

```text id="m7q9p0"
Hardware

↓

Driver

↓

Input Manager

↓

Event Bus

↓

UI / Services
```

---

# 24. Implementation notes for AI coding agent

When implementing:

* Treat physical controls as equal to touchscreen.
* Never hardcode GPIO logic inside UI.
* Keep navigation centralized.
* Make global playback controls always available.
* Use events instead of direct calls.
* Optimize for low latency.
* Design for future hardware revisions.

The final input system should make FLACHEAD feel like a real dedicated music player: physical, responsive, and reliable.
