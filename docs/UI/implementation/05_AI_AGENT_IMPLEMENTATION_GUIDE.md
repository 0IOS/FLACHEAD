# 05_AI_AGENT_IMPLEMENTATION_GUIDE.md

# FLACHEAD UI — AI Agent Implementation Guide

## 1. Purpose

This document defines how an AI coding agent should implement FLACHEAD UI using the complete documentation set.

The purpose is to prevent inconsistent generated code and ensure the implementation follows the intended architecture.

This document acts as the final instruction layer between:

```text
FLACHEAD Documentation

↓

AI Coding Agent

↓

Working Implementation
```

Primary principle:

> The AI must implement the architecture, not invent a new one.

---

# 2. Implementation philosophy

The AI agent must prioritize:

1. Correct architecture
2. Stability
3. Performance
4. Maintainability
5. Visual polish

---

Never prioritize:

* unnecessary effects
* fast prototype shortcuts
* large dependencies
* desktop-only solutions

---

# 3. Required implementation order

Implementation should happen in this order:

```text
1. Core Systems

↓

2. Hardware Abstraction

↓

3. Renderer

↓

4. UI Components

↓

5. Screens

↓

6. Applications

↓

7. Advanced Features
```

---

# 4. Phase 1 — Core systems

Implement first:

```text id="x7m4q9"
Event System

State Machine

Configuration Manager

Logger

Resource Manager
```

---

Reason:

Every other system depends on these.

---

# 5. Phase 2 — Hardware abstraction

Implement:

```text id="m5x8q3"
Display Backend

Input Backend

GPIO Interface

Audio Interface
```

---

Rules:

* no UI code inside drivers
* no application logic inside hardware code

---

# 6. Phase 3 — Renderer

Implement:

```text id="v8m3x5"
Frame Manager

Layer System

Component Renderer

Display Backend
```

---

Requirements:

* support desktop testing
* support TFT deployment
* minimize memory usage

---

# 7. Phase 4 — Components

Implement reusable components:

Order:

```text id="c6m4x9"
Base Component

Text

Button

Image

List

Slider

Navigation Elements
```

---

Each component must follow:

```text id="r7m3x8"
Lifecycle

↓

Layout

↓

Input

↓

Render
```

---

# 8. Phase 5 — Screens

Implement screens using components.

Order:

```text id="n8m4x6"
Home

Launcher

Music Library

Player

Lyrics

Settings
```

---

Screens must:

* use services
* use events
* avoid direct hardware access

---

# 9. Phase 6 — Audio integration

Connect:

```text id="w5m3x7"
Audio Engine

↓

Event System

↓

UI Synchronization

↓

Player Screen
```

---

Never:

```text id="a7m4x8"
UI

↓

Controls Decoder Directly
```

---

# 10. Code quality rules

Generated code must:

* have clear names
* avoid unnecessary complexity
* include error handling
* follow existing architecture

---

Avoid:

* giant classes
* duplicate systems
* hidden dependencies

---

# 11. Dependency rules

Before adding a library:

Ask:

```text id="p6m8x2"
Is this required?

Does it improve reliability?

Can native code solve it?
```

---

Avoid unnecessary dependencies.

---

# 12. Performance rules

The AI must always consider:

Target:

```text id="u5m3x9"
Raspberry Pi Zero W
```

---

Avoid:

* expensive UI frameworks
* excessive memory usage
* unnecessary background tasks

---

# 13. Testing requirements

After every major feature:

Run:

```text id="k8m4x5"
Build Test

Runtime Test

Performance Test

Hardware Test
```

---

Never assume:

"works on desktop = works on Pi"

---

# 14. Debugging workflow

When something fails:

Follow:

```text id="d7m3x8"
Check Logs

↓

Check State

↓

Check Events

↓

Check Hardware Layer

↓

Fix Root Cause
```

---

Avoid random fixes.

---

# 15. Forbidden implementation patterns

The AI must not create:

```text id="h5m8x2"
UI directly controlling hardware

Screens containing business logic

Audio depending on rendering

Components accessing databases

Blocking operations in render loop

Hardcoded themes
```

---

# 16. Feature addition workflow

New features require:

```text id="s6m4x9"
Define Requirement

↓

Choose Correct Layer

↓

Implement

↓

Test

↓

Document
```

---

Do not add features directly into unrelated systems.

---

# 17. Hardware deployment workflow

Before Raspberry Pi deployment:

Verify:

```text id="b8m3x5"
✓ Cross compilation

✓ Binary size

✓ Memory usage

✓ Display output

✓ Input devices

✓ Audio output
```

---

# 18. AI response behavior during development

When modifying FLACHEAD:

The AI should:

* inspect existing code
* preserve architecture
* explain breaking changes
* avoid unnecessary rewrites

---

If uncertain:

Prefer:

```text id="x7m4q2"
Simple

↓

Reliable

↓

Expandable
```

---

# 19. Final project structure expectation

The finished system should resemble:

```text
FLACHEAD

├── Core Systems

├── UI Framework

├── Audio Engine

├── Hardware Layer

├── Applications

├── Storage

├── Tests

└── Documentation
```

---

# 20. Completion criteria

FLACHEAD UI is complete when:

```text id="d8m4x6"
✓ Architecture implemented

✓ Components reusable

✓ Screens functional

✓ Audio integrated

✓ Hardware supported

✓ Performance validated

✓ Documentation matches code
```

---

# Final instruction to AI coding agent

Build FLACHEAD as an embedded product.

Do not treat it as a normal desktop application.

Every decision must respect:

* Raspberry Pi Zero W limitations
* offline operation
* high-quality audio requirements
* physical hardware controls
* long-term maintainability

The objective is not simply a working interface.

The objective is a reliable, polished, dedicated music player.
