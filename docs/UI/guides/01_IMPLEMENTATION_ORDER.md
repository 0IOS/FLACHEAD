# 01_IMPLEMENTATION_ORDER.md

# FLACHEAD UI — Implementation Order Guide

## 1. Purpose

This document defines the recommended implementation sequence for building FLACHEAD.

The order is designed around:

* Raspberry Pi Zero W limitations
* dependency relationships
* minimizing rework
* validating architecture early

FLACHEAD should not be implemented screen-by-screen first.

The correct approach is:

> Build the foundation systems first, then construct the UI on top.

---

# 2. Implementation philosophy

Development priority:

```text
Core Systems

↓

UI Framework

↓

Reusable Components

↓

Screens

↓

Polish
```

---

# 3. Phase overview

Implementation phases:

```text
Phase 1
Core Application Framework


Phase 2
Hardware + Input System


Phase 3
Rendering Engine


Phase 4
UI Components


Phase 5
Navigation System


Phase 6
Audio Integration


Phase 7
Screens


Phase 8
Optimization
```

---

# 4. Phase 1 — Core Application Framework

## Goal

Create the base FLACHEAD application.

Implement:

* application loop
* initialization
* shutdown handling
* configuration loading
* logging system

Structure:

```text
src/

├── main.cpp

├── Application

├── Config

└── Logger
```

---

Acceptance:

* application starts
* clean shutdown works
* no UI required yet

---

# 5. Phase 2 — Hardware and Input System

## Goal

Make the hardware usable.

Implement:

* GPIO abstraction
* touchscreen input
* buttons
* encoder

Architecture:

```text
Hardware

↓

Input Backend

↓

Input Manager

↓

Events
```

---

Test:

* button presses
* touch coordinates
* encoder movement

---

Acceptance:

Every physical control generates the correct event.

---

# 6. Phase 3 — Rendering Engine

## Goal

Create the graphics foundation.

Implement:

* SDL initialization
* display handling
* renderer
* frame loop
* asset manager

---

Required tests:

Display:

* correct orientation
* correct resolution
* stable FPS

---

Acceptance:

A blank FLACHEAD screen renders smoothly.

---

# 7. Phase 4 — UI Components

## Goal

Create reusable building blocks.

Implement:

First:

```text
Text

Button

Icon

Container
```

---

Then:

```text
Card

List

Slider

Progress Bar

Artwork View
```

---

Every component must follow:

```text
Component API Contract
```

---

Acceptance:

Components render independently.

---

# 8. Phase 5 — Navigation System

## Goal

Connect screens.

Implement:

* Screen Manager
* screen stack
* transitions
* lifecycle handling

---

Test:

```text
Home

↓

Launcher

↓

Back
```

---

Acceptance:

Screens open and close correctly.

---

# 9. Phase 6 — Audio Integration

## Goal

Connect the music engine.

Implement:

* Audio Session API
* Playback Engine bridge
* Queue Manager connection
* metadata communication

---

First test:

```text
Play

Pause

Next

Previous

Volume
```

---

Acceptance:

Music works without depending on UI.

---

# 10. Phase 7 — Core Screens

Implement screens in this order:

---

## 1. Home Screen

Why first:

* system foundation
* navigation entry point

---

## 2. Mini Player

Why:

* always visible
* tests audio integration

---

## 3. Full Music Player

Why:

* primary FLACHEAD experience

---

## 4. Music Library

Why:

* database interaction

---

## 5. Queue

Why:

* playback management

---

## 6. Lyrics View

Why:

* synchronized display

---

## 7. Settings

Why:

* configuration system

---

## 8. Launcher

Why:

* application framework

---

# 11. Phase 8 — System Overlays

Implement:

```text
Notifications

Dialogs

Volume Overlay

Brightness Overlay
```

---

Reason:

These interact globally.

---

# 12. Phase 9 — Boot Experience

Implement:

* startup animation
* loading states
* recovery screen

---

Requirements:

Boot must not block audio initialization.

---

# 13. Phase 10 — Optimization

Optimization happens after functionality.

Measure:

* FPS
* memory
* CPU
* startup time

---

Optimize:

* rendering
* assets
* database
* animations

---

# 14. Testing milestones

## Milestone 1

Application boots.

---

## Milestone 2

Touch/buttons work.

---

## Milestone 3

UI renders.

---

## Milestone 4

Audio works.

---

## Milestone 5

Complete player works.

---

## Milestone 6

Long-term stability test.

---

# 15. Things NOT to implement early

Avoid:

## Advanced animations

Until renderer is stable.

---

## Theme engine

Until components exist.

---

## App ecosystem

Until core player works.

---

## Network features

FLACHEAD is offline-first.

---

# 16. Recommended development order

Final order:

```text
1. Application Core

2. Input System

3. Event Bus

4. Renderer

5. Component System

6. Screen Manager

7. Theme Engine

8. Audio Session

9. Player Screens

10. Library

11. System Screens

12. Optimization
```

---

# 17. AI coding agent instructions

When implementing:

* Follow this order.
* Do not skip foundations.
* Do not create temporary architecture.
* Do not mix UI and backend.
* Test every subsystem before moving forward.
* Prefer stable simple systems over complicated ones.

---

# 18. Final goal

The implementation should result in:

* fast boot
* instant controls
* stable audio playback
* smooth UI
* maintainable architecture

FLACHEAD should be built like embedded hardware software, not like a desktop application.

