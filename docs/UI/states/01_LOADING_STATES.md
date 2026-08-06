# 01_LOADING_STATES.md

# FLACHEAD UI — Loading States Specification

## 1. Purpose

This document defines how FLACHEAD represents loading and initialization states.

FLACHEAD relies on several hardware and software services:

* Raspberry Pi Zero W boot process
* storage detection
* music library loading
* DSP initialization
* metadata loading
* UI preparation

Because the hardware is limited, loading states must be carefully designed to communicate progress without slowing the system.

Primary principle:

> Loading screens should inform the user, not make the device feel slow.

---

# 2. Design goals

Loading states must be:

* lightweight
* informative
* non-blocking
* consistent
* optimized for Pi Zero W

---

# 3. Loading architecture

Data flow:

```text
Service Event

↓

State Manager

↓

UI State Layer

↓

Renderer

↓

User Feedback
```

---

The UI must never guess loading status.

All states come from:

* services
* managers
* hardware events

---

# 4. Loading state categories

FLACHEAD uses:

```text
1. Boot Loading

2. Hardware Initialization

3. Library Loading

4. Screen Loading

5. Background Operation Loading
```

---

# 5. Boot loading

Displayed during startup.

Purpose:

Provide immediate feedback after power-on.

---

Layout:

```text
+----------------------+
|                      |
|      FLACHEAD        |
|                      |
|   Initializing...    |
|                      |
|      ◌ ◌ ◌           |
|                      |
+----------------------+
```

---

Must show:

* logo/device identity
* current operation

---

Avoid:

* long animations
* heavy graphics

---

# 6. Hardware initialization

Services:

* display
* touchscreen
* buttons
* DSP
* storage

---

Example:

```text
Initializing Hardware

✓ Display

✓ Buttons

✓ Storage

◌ Audio Device
```

---

States:

```text
Pending

Initializing

Ready

Failed
```

---

# 7. DSP initialization state

Important for FLACHEAD.

Example:

```text
Audio System

Detecting:

TANCHJIM BUNNY DSP
```

---

Success:

```text
Audio Ready
```

---

Failure:

```text
DSP Not Found

Using fallback output
```

---

The UI must still boot.

---

# 8. Library loading

Occurs when:

* startup
* storage inserted
* database rebuilt

---

Display:

```text
Loading Music Library

542 Tracks Found
```

---

The user should still be able to:

* access settings
* control playback
* cancel operations

---

# 9. Screen loading

Used when opening:

* large libraries
* lyrics
* album views

---

Example:

```text
Loading Album...
```

---

Duration target:

<200ms

---

# 10. Background loading indicators

For long operations:

Use:

* small progress indicators
* notifications
* status icons

---

Avoid full-screen blocking.

---

Examples:

Library scan:

```text
Scanning Music...
```

---

Artwork:

```text
Updating Artwork
```

---

# 11. Loading animations

Allowed:

* spinner
* progress bar
* small movement

---

Duration:

100-200ms

---

Avoid:

* animated backgrounds
* large transitions
* GPU-heavy effects

---

# 12. Loading cancellation

Long operations must support cancellation.

Examples:

* library scan
* artwork processing

---

Short operations:

No cancellation required.

---

# 13. Loading priority

Priority order:

```text
Audio

↓

Input

↓

Rendering

↓

Background Tasks
```

---

Never allow:

* scanning
* metadata processing
* artwork generation

to interrupt playback.

---

# 14. Performance requirements

Loading UI:

Must maintain:

```text
30 FPS minimum
```

Preferred:

```text
60 FPS
```

---

CPU usage:

Keep background operations limited.

---

# 15. Error handling

Loading failure:

Display:

```text
Unable to load

Retry

Continue
```

---

Hardware failure:

Continue with degraded mode.

---

Example:

No DSP:

```text
Basic audio mode enabled
```

---

# 16. Acceptance criteria

Implementation is complete when:

* every async operation has a state
* loading feedback is consistent
* UI never freezes
* playback priority is preserved
* failures recover gracefully

---

# 17. Architectural recommendations

Required service:

```text
State Manager

├── Loading States

├── Error States

├── Empty States

├── Transition States

└── Recovery States
```

---

Required component:

```text
LoadingIndicator

ProgressBar

StatusMessage

StateOverlay
```

---

# 18. Implementation notes for AI coding agent

When implementing:

* Never use blocking loading calls on the UI thread.
* Keep loading states event-driven.
* Prefer partial loading over waiting for everything.
* Show useful information.
* Prioritize audio playback over visual operations.

A good FLACHEAD loading experience should feel like a dedicated hardware player powering on, not an application waiting to open.
