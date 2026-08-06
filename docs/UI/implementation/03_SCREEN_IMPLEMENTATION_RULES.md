# 03_SCREEN_IMPLEMENTATION_RULES.md

# FLACHEAD UI — Screen Implementation Rules

## 1. Purpose

This document defines the rules for implementing FLACHEAD screens.

Screens are complete user-facing views built by combining components and connecting them to application services.

Examples:

* Home
* Music Library
* Player
* Lyrics
* Settings
* Launcher

Primary principle:

> Screens organize components and data. They do not own the systems behind that data.

---

# 2. Screen architecture

Every screen follows:

```text id="x7m4q9"
Screen

├── State

├── Components

├── Data Binding

├── Event Handling

├── Lifecycle

└── Rendering
```

---

# 3. Screen responsibilities

A screen may:

* arrange components
* request data
* react to events
* manage temporary UI state

A screen must not:

* decode audio
* access GPIO
* directly modify hardware
* manage databases

---

# 4. Screen lifecycle

Every screen follows:

```text id="m5x8q3"
Create

↓

Enter

↓

Active

↓

Pause

↓

Exit

↓

Destroy
```

---

# 5. Create phase

Used for:

* creating components
* preparing layout
* registering events

Avoid:

* heavy loading
* blocking operations

---

# 6. Enter phase

Runs when the user opens the screen.

Examples:

Player Screen:

```text id="v8m3x5"
Request Current Track

Update Controls
```

Library Screen:

```text id="c6m4x9"
Request Visible Songs
```

---

# 7. Active phase

During active use:

Handles:

* input
* animations
* updates
* rendering

---

The screen should remain lightweight.

---

# 8. Pause phase

A screen may remain loaded but inactive.

Example:

```text id="r7m3x8"
Player

↓

Settings

↓

Player resumes
```

---

Possible actions:

* stop animations
* reduce updates
* keep important state

---

# 9. Exit phase

When leaving:

Release:

* temporary resources
* listeners
* animations

---

Keep:

* persistent user data
* application state

---

# 10. Screen state

Screen state contains:

Temporary UI information.

Examples:

Library:

```text id="n8m4x6"
Scroll Position

Selected Item

Filter
```

---

Player:

```text id="w5m3x7"
Current Animation

Display Mode
```

---

# 11. Data loading

Screens receive data through services.

Flow:

```text id="a7m4x8"
Screen Request

↓

Service

↓

Event

↓

Screen Update
```

---

Never:

```text id="p6m8x2"
Screen

↓

Database Query
```

---

# 12. Screen navigation

Navigation is controlled by:

```text id="u5m3x9"
UI State Machine
```

---

Screens should request navigation.

Example:

```text id="k8m4x5"
Button Press

↓

OPEN_LIBRARY Event

↓

State Machine
```

---

# 13. Screen composition

Example:

Player screen:

```text id="d7m3x8"
Player Screen

├── Background

├── Album Artwork

├── Track Information

├── Progress Bar

├── Playback Controls

├── Lyrics Preview

└── Bottom Navigation
```

---

---

# 14. Rendering rules

Screens should:

* arrange components
* trigger rendering

They should not:

* draw every primitive manually
* duplicate component logic

---

# 15. Screen updates

Not every event requires full refresh.

Example:

Volume change:

Update:

```text id="h5m8x2"
Volume Indicator
```

---

Do not redraw:

```text id="s6m4x9"
Entire Screen
```

---

# 16. Data caching

Screens may cache:

* recently displayed information
* temporary layout data

---

Screens must not become storage systems.

---

# 17. Error handling

Screens should handle:

```text id="b8m3x5"
Loading State

Empty State

Error State

Offline State
```

---

Example:

Library unavailable:

```text id="x7m4q2"
Show Recovery Message
```

---

# 18. Performance rules

Screens must:

* avoid unnecessary updates
* reuse components
* minimize calculations

---

Target:

Stable rendering:

```text id="d8m4x6"
30-60 FPS
```

---

# 19. Testing requirements

Each screen should test:

```text id="m5x8q3"
Opening

Closing

Navigation

Input

Data Updates

Error States
```

---

# 20. Acceptance criteria

A screen is complete when:

* lifecycle is implemented
* navigation works
* components are reusable
* data comes through services
* performance targets are met

---

# 21. Implementation notes for AI coding agent

When implementing:

* Build screens from existing components.
* Keep screens as coordinators, not giant classes.
* Follow the state machine rules.
* Use events for updates.
* Test every screen on Raspberry Pi Zero W.

Screens are the visible layer of FLACHEAD, but keeping them lightweight is what allows the entire system to remain fast and reliable.
