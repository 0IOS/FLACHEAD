# 03_GPIO_BUTTON_INTEGRATION.md

# FLACHEAD UI — GPIO Button Integration Specification

## 1. Purpose

This document defines how FLACHEAD integrates physical buttons using Raspberry Pi GPIO.

Physical controls are a core part of the FLACHEAD experience.

The GPIO system provides:

* navigation input
* playback control
* hardware reliability
* low-latency interaction

Primary principle:

> Hardware controls should feel instant and dependable, like a dedicated audio device.

---

# 2. Design goals

GPIO button integration must be:

* reliable
* low latency
* hardware independent
* power efficient
* easy to configure

---

# 3. Hardware assumptions

Target hardware:

```text id="x7m4q9"
Board:

Raspberry Pi Zero W


Input:

GPIO Buttons


Controls:

Navigation

Playback

System Actions
```

---

# 4. GPIO architecture

Data flow:

```text id="m5x8q3"
GPIO Pin

↓

GPIO Driver

↓

Button Manager

↓

Input Manager

↓

UI Event System
```

---

The UI must never communicate directly with GPIO pins.

---

# 5. Button abstraction

Required interface:

```text id="v8m3x5"
Button Device

├── Initialize

├── Read State

├── Detect Press

├── Detect Release

├── Detect Hold

└── Emit Event
```

---

# 6. GPIO configuration

Recommended:

Use:

```text id="c6m4x9"
Internal Pull-up

or

Internal Pull-down
```

---

Avoid:

Floating inputs.

---

# 7. Button mapping

Example:

```text id="r7m3x8"
GPIO 1

HOME


GPIO 2

BACK


GPIO 3

SELECT


GPIO 4

UP


GPIO 5

DOWN
```

---

Actual pins should remain configurable.

---

# 8. Debouncing

Mechanical switches create noise.

Required:

Software debounce.

---

Recommended:

```text id="n8m4x6"
20-50ms
```

---

Process:

```text id="w5m3x7"
GPIO Signal

↓

Debounce Filter

↓

Valid Button Event
```

---

# 9. Button event types

Supported:

```text id="a7m4x8"
Pressed

Released

Held

Repeated
```

---

Example:

```text id="p6m8x2"
UP Press

↓

Move Selection
```

---

# 10. Long press detection

Required for:

* HOME task overview
* seeking
* advanced actions

---

Timing:

```text id="u5m3x9"
700-1000ms
```

---

# 11. Key repeat

Used for:

* scrolling lists
* settings adjustment

---

Example:

Hold DOWN:

```text id="k8m4x5"
DOWN

↓

Repeat

↓

Scroll
```

---

Recommended:

Initial delay:

500ms

Repeat interval:

100ms

---

# 12. Interrupt vs polling

Preferred:

GPIO interrupts.

---

Reason:

* lower CPU usage
* faster response
* efficient on Pi Zero W

---

Polling may be used for:

* prototypes
* debugging

---

# 13. Power behavior

GPIO monitoring should:

* consume minimal CPU
* remain active during playback
* survive display sleep

---

Buttons should wake the device if supported.

---

# 14. Error handling

GPIO failure:

Display:

```text id="d7m3x8"
Hardware controls unavailable

Use touchscreen
```

---

Invalid pin:

Log error.

---

# 15. Testing requirements

Test:

* single press
* long press
* rapid presses
* simultaneous presses
* noisy switches

---

# 16. Performance requirements

Button response:

```text id="h5m8x2"
<50ms
```

---

CPU usage:

Near zero when idle.

---

GPIO handling must never:

* block audio
* delay rendering
* freeze UI

---

# 17. Acceptance criteria

Implementation is complete when:

* all buttons generate events
* debounce works
* long presses work
* mapping is configurable
* failures recover safely

---

# 18. Architectural recommendations

Required modules:

```text id="s6m4x9"
GPIO Manager

├── Pin Configuration

├── Interrupt Handler

├── Debounce Logic

├── Button Mapping

└── Event Dispatcher
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Keep GPIO code isolated.
* Use an event-driven approach.
* Avoid polling loops.
* Make pins configurable.
* Test hardware separately from UI.

The GPIO system is what transforms FLACHEAD from software running on a Pi into an actual dedicated music player.
