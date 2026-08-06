# 06_HARDWARE_INPUT_MAP.md

# FLACHEAD UI — Hardware Input Map Contract

## 1. Purpose

This document defines how FLACHEAD hardware controls communicate with the UI system.

FLACHEAD is designed as a dedicated music device.

The interface must support:

* 2.8" touchscreen
* physical navigation buttons
* playback controls
* rotary volume control
* future hardware expansion

Hardware input must remain independent from UI implementation.

Primary principle:

> Hardware generates input events; the UI decides what those events mean.

---

# 2. Hardware input architecture

Input flow:

```text id="h5m8x2"
Physical Hardware

+

Touchscreen

        |

        v

Input Driver Layer

        |

        v

Input Manager

        |

        v

Event Bus

        |

        v

UI / Services
```

---

# 3. Input responsibilities

Input Layer handles:

* GPIO reading
* touchscreen events
* button debouncing
* encoder movement
* gesture detection

Input Layer does not handle:

* navigation decisions
* playback commands
* screen logic

---

# 4. Supported inputs

FLACHEAD hardware:

```text id="x7m3q9"
Touchscreen

Navigation Buttons

Playback Buttons

Volume Encoder
```

---

# 5. Physical button layout

Primary controls:

```text id="p4m8x1"
[ UP ]

[ DOWN ]

[ SELECT ]


[ HOME ]

[ BACK ]

[ PLAY ]
```

---

Additional controls may be added later.

---

# 6. Navigation buttons

## UP button

Event:

```text id="q8m2x5"
BUTTON_UP
```

Default behavior:

* move selection upward
* scroll lists upward

---

## DOWN button

Event:

```text id="v5m9x3"
BUTTON_DOWN
```

Default behavior:

* move selection downward
* scroll lists downward

---

## SELECT button

Event:

```text id="m3x7q8"
BUTTON_SELECT
```

Default behavior:

* confirm selection
* open item

---

# 7. Home button

Event:

```text id="n6m2x9"
BUTTON_HOME
```

Special behavior:

## Single press

Open launcher.

Flow:

```text id="k4x8m1"
Current Screen

↓

Launcher
```

---

## Double press

Return home.

Flow:

```text id="r7m3x5"
Any Screen

↓

Home Screen
```

---

## Long press

Open task overview.

Flow:

```text id="z2m8x4"
Hold Home

↓

Task Overview
```

---

# 8. Back button

Event:

```text id="w9m4x6"
BUTTON_BACK
```

Default behavior:

```text id="c3x7m2"
Current Screen

↓

Previous Screen
```

---

Exceptions:

Dialog open:

```text id="p8m5x1"
Close Dialog
```

---

Overlay open:

```text id="a4m9x7"
Close Overlay
```

---

# 9. Play button

Event:

```text id="y6m3x8"
BUTTON_PLAY
```

Default action:

```text id="f2x8m5"
togglePlayback()
```

Works globally.

---

# 10. Touchscreen input

Touch events:

```text id="m7x4q9"
TOUCH_DOWN

TOUCH_UP

TOUCH_MOVE

TOUCH_TAP

TOUCH_SWIPE
```

---

# 11. Touch coordinate system

The touchscreen uses:

```text id="v8m2x6"
Portrait Coordinates

X

Y
```

---

Input manager converts:

```text id="q5x9m3"
Raw Touch

↓

UI Coordinates
```

---

# 12. Touch gestures

Supported gestures:

## Tap

Used for:

* buttons
* lists
* controls

---

## Swipe

Used for:

* scrolling
* navigation

Directions:

```text id="x3m8p7"
UP

DOWN

LEFT

RIGHT
```

---

## Long press

Used for:

* contextual actions

Duration:

~700ms

---

# 13. Volume encoder

Hardware:

Rotary encoder.

Input:

```text id="b7m4x9"
ENCODER_CLOCKWISE

ENCODER_COUNTERCLOCKWISE
```

---

Mapping:

Clockwise:

```text id="n8x2m5"
Volume +
```

Counter-clockwise:

```text id="s6m3q8"
Volume -
```

---

# 14. Encoder acceleration

Optional feature.

Slow rotation:

```text id="r4m9x2"
small volume changes
```

---

Fast rotation:

```text id="k7x3m6"
larger volume changes
```

---

# 15. Input priority system

Input handling order:

```text id="d8m2x5"
Emergency/System Controls

↓

Global Controls

↓

Screen Controls

↓

Component Controls
```

---

Example:

Volume encoder:

Always active.

---

# 16. Button debouncing

Mechanical buttons require filtering.

Requirements:

* ignore accidental repeats
* prevent double triggering

Recommended debounce:

10-50ms

---

# 17. Input state model

Input Manager tracks:

```text id="p5m8x7"
Button State

pressed

released

held

duration
```

---

Encoder:

```text id="q9x3m6"
direction

steps

timestamp
```

---

# 18. Hardware abstraction layer

The UI must not know GPIO details.

Architecture:

```text id="x8m4q2"
GPIO Driver

↓

Input Backend

↓

Input Manager

↓

Events
```

---

Future hardware can replace:

* Raspberry Pi GPIO
* Arduino controller
* custom PCB

---

# 19. Input mapping configuration

Mappings should be configurable.

Example:

```text id="m2x7p8"
HOME = GPIO17

BACK = GPIO18

PLAY = GPIO27
```

---

Never hardcode pins inside UI code.

---

# 20. Touchscreen performance

Touch processing:

* low latency
* lightweight
* event based

Avoid:

* polling every frame

---

Preferred:

```text id="v7m3x9"
Touch Event

↓

Input Queue
```

---

# 21. Error handling

Input failure examples:

## Touch failure

```text id="c6m8x2"
Touch unavailable

↓

Continue with buttons
```

---

## Button failure

```text id="z4m7x1"
Disable control

↓

Notify system
```

---

# 22. Testing requirements

Test:

* every button
* long press
* double press
* encoder direction
* touchscreen accuracy
* simultaneous inputs

---

# 23. Acceptance criteria

Implementation is complete when:

* hardware is abstracted
* events are consistent
* buttons work globally
* touch is responsive
* encoder controls volume
* failures do not crash UI

---

# 24. Architectural recommendations

## Input Manager

```text id="g9m5x2"
Input Manager

├── readInput()

├── debounce()

├── detectGesture()

├── createEvent()

└── dispatch()
```

---

## Hardware Backend

```text id="w3x8m6"
Input Backend

├── GPIO

├── Touch

├── Encoder

└── Future Devices
```

---

# 25. Implementation notes for AI coding agent

When implementing:

* Never connect UI directly to GPIO.
* Convert hardware actions into events.
* Keep mappings configurable.
* Make global controls always available.
* Prioritize responsiveness.
* Design for Raspberry Pi Zero W limitations.

The final input system should make FLACHEAD feel like a real dedicated digital audio player: physical, immediate, and reliable.
