# 01_UI_STATE_MACHINE.md

# FLACHEAD UI — UI State Machine Specification

## 1. Purpose

This document defines the core state management architecture of FLACHEAD UI.

The UI state machine controls:

* current screen
* navigation transitions
* modal states
* overlays
* system states
* recovery states

The purpose is to prevent screens from directly controlling each other and create a predictable UI flow.

Primary principle:

> Every UI condition must exist as a known state with defined transitions.

---

# 2. Design goals

The UI state machine must be:

* deterministic
* lightweight
* event-driven
* easy to debug
* optimized for Raspberry Pi Zero W

---

# 3. State machine architecture

Data flow:

```text
User Input

↓

Input Manager

↓

Event System

↓

UI State Machine

↓

Screen Controller

↓

Renderer
```

---

No screen should directly open another screen.

---

# 4. Core state types

FLACHEAD contains:

```text
1. Screen States

2. Overlay States

3. Modal States

4. System States

5. Recovery States
```

---

# 5. Screen states

Main screens:

```text
HOME

LAUNCHER

TASK_OVERVIEW

MUSIC_LIBRARY

QUEUE

FULL_PLAYER

LYRICS

SETTINGS
```

---

Each screen is independent.

---

# 6. State structure

Each state contains:

```text
State

├── Enter()

├── Update()

├── HandleEvent()

├── Render()

└── Exit()
```

---

Example:

```text
MusicPlayerState

Enter:
Load player data

Update:
Update progress

Render:
Draw player UI

Exit:
Release temporary resources
```

---

# 7. Navigation stack

FLACHEAD uses stack-based navigation.

Example:

```text
HOME

↓

LIBRARY

↓

ALBUM

↓

PLAYER
```

---

Back button:

```text
POP current state
```

---

Result:

```text
ALBUM
```

---

# 8. Global states

Some states exist above screens.

Examples:

* volume overlay
* notifications
* DSP warnings
* errors

---

These are global.

---

# 9. Overlay system

Overlays do not replace screens.

Examples:

```text
Volume Overlay

Brightness Overlay

Notification Banner
```

---

Layer:

```text
Screen

↓

Overlay

↓

Input Layer
```

---

# 10. Modal states

Modal states capture input.

Examples:

* confirmation dialog
* delete confirmation
* settings reset

---

Example:

```text
PLAYER

↓

Delete Dialog

↓

PLAYER
```

---

# 11. System states

System-level states:

```text
BOOT

INITIALIZING

READY

SLEEP

SHUTDOWN
```

---

System states have highest priority.

---

# 12. Transition system

Transitions must be controlled.

Structure:

```text
Current State

↓

Transition Event

↓

New State
```

---

Example:

```text
HOME

Tap Library

↓

LIBRARY
```

---

# 13. Transition animations

Allowed:

* fade
* slide
* small movement

Duration:

```text
100-200ms
```

---

Avoid:

* heavy transitions
* 3D effects

---

# 14. State persistence

Persistent:

```text
Current Track

Volume

Theme

Settings
```

---

Temporary:

```text
Current Screen

Scroll Position
```

---

# 15. Task overview integration

Task overview stores active states.

Example:

```text
Player

Library

Settings
```

---

Selecting one restores state.

---

# 16. Background state updates

Screens may receive:

* playback updates
* notifications
* hardware changes

---

Example:

Library open:

Playback continues updating mini-player.

---

# 17. Performance requirements

State changes:

Target:

```text
<50ms
```

---

State machine must:

* avoid unnecessary allocations
* reuse screens where possible
* release unused resources

---

# 18. Error handling

Invalid transition:

Example:

```text
PLAYER

↓

UNKNOWN STATE
```

---

Recovery:

Return to safe state:

```text
HOME
```

---

# 19. Acceptance criteria

Implementation is complete when:

* all screens are states
* navigation is predictable
* overlays work globally
* errors recover safely
* no screen directly controls another

---

# 20. Architectural recommendations

Required module:

```text
UI State Machine

├── State Registry

├── Current State

├── Navigation Stack

├── Transition Manager

└── Event Handler
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Keep state logic separate from rendering.
* Use events instead of direct function calls.
* Avoid creating a new screen object every navigation.
* Keep global overlays independent.
* Design for future apps and plugins.

The UI state machine is the foundation that allows FLACHEAD to grow without becoming difficult to maintain.
