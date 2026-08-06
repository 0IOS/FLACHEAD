# 03_RENDERING_PIPELINE.md

# FLACHEAD UI — Rendering Pipeline Specification

## 1. Purpose

This document defines how FLACHEAD converts UI state into pixels on the 2.8" TFT display.

The rendering pipeline controls:

* frame generation
* component drawing
* optimization
* display updates
* resource usage

Because FLACHEAD targets Raspberry Pi Zero W, rendering must prioritize efficiency over visual complexity.

Primary principle:

> Render only what changes, and never waste hardware resources.

---

# 2. Design goals

The rendering system must be:

* fast
* predictable
* modular
* low memory
* hardware-independent

---

# 3. Rendering architecture

Pipeline:

```text id="x7m4q9"
UI State

↓

Screen Components

↓

Layout Engine

↓

Renderer

↓

Display Backend

↓

TFT Display
```

---

# 4. Renderer responsibilities

The renderer handles:

* drawing components
* managing layers
* updating regions
* compositing images
* rendering text

---

The renderer does not handle:

* business logic
* playback control
* input processing

---

# 5. Rendering layers

FLACHEAD uses layered rendering:

```text id="m5x8q3"
Layer 0

Background


Layer 1

Screen Content


Layer 2

Mini Player


Layer 3

Overlays


Layer 4

Dialogs


Layer 5

System Indicators
```

---

Higher layers render above lower layers.

---

# 6. Frame generation

Frame lifecycle:

```text id="v8m3x5"
State Update

↓

Layout Calculation

↓

Component Render

↓

Frame Buffer

↓

Display Update
```

---

# 7. Framebuffer strategy

Recommended:

Double buffering.

```text id="c6m4x9"
Buffer A

↓

Display


Buffer B

↓

Next Frame
```

---

Benefits:

* avoids tearing
* smoother animation
* predictable timing

---

# 8. Dirty rectangle rendering

Full redraws should be avoided.

Instead:

```text id="r7m3x8"
Changed Area

↓

Update Only Region
```

---

Examples:

Volume change:

Update:

```text id="n8m4x6"
Volume Overlay
```

Not entire screen.

---

# 9. Component rendering

Each component provides:

```text id="w5m3x7"
Component

├── Layout()

├── Update()

├── Render()

└── Destroy()
```

---

Components should be reusable.

---

# 10. Text rendering

Text rendering is expensive.

Optimization:

* cache fonts
* cache common strings
* avoid unnecessary redraws

---

Example:

Playback controls:

Render once.

Update only state.

---

# 11. Image rendering

Artwork pipeline:

```text id="a7m4x8"
Artwork File

↓

Decoder

↓

Resize Cache

↓

Renderer

↓

Display
```

---

Never decode images during frame rendering.

---

# 12. Animation system

Supported:

* fades
* small movements
* progress animations

---

Animation updates:

```text id="p6m8x2"
60 FPS preferred

30 FPS minimum
```

---

Avoid:

* particle effects
* blur
* shader effects

---

# 13. Frame timing

Renderer tracks:

```text id="u5m3x9"
Frame Start

Update Time

Render Time

Display Time
```

---

Goal:

Stable frame pacing.

---

# 14. Performance priority

Priority:

```text id="k8m4x5"
Audio

↓

Input

↓

Rendering

↓

Animations
```

---

Visual effects are always lowest priority.

---

# 15. Memory management

Renderer should:

* reuse buffers
* avoid allocations per frame
* release unused textures

---

Avoid:

```text id="d7m3x8"
new object every frame
```

---

# 16. Display backend abstraction

Renderer communicates through:

```text id="h5m8x2"
Display Backend

├── SDL Backend

├── Framebuffer Backend

└── TFT Driver Backend
```

---

This allows desktop testing.

---

# 17. Error handling

Rendering failure:

Attempt:

```text id="s6m4x9"
Restart Renderer

↓

Restore Current State
```

---

Playback continues.

---

# 18. Performance requirements

Target:

```text id="b8m3x5"
60 FPS when possible

30-45 FPS minimum
```

---

Frame time:

Preferred:

```text id="x7m4q2"
<16ms
```

---

Acceptable:

```text id="d8m4x6"
<33ms
```

---

# 19. Acceptance criteria

Implementation is complete when:

* rendering is smooth
* dirty updates work
* components render independently
* memory stays stable
* display backend can change

---

# 20. Architectural recommendations

Required modules:

```text id="m5x8q3"
Renderer

├── Frame Manager

├── Layer Manager

├── Component Renderer

├── Image Cache

├── Font Cache

└── Display Backend
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Keep renderer separate from UI logic.
* Prefer simple drawing primitives.
* Cache expensive resources.
* Use dirty rendering.
* Profile on actual Raspberry Pi Zero W hardware.

The rendering pipeline is the performance core of FLACHEAD. A simple optimized renderer will outperform a visually complex but inefficient one on the target hardware.
