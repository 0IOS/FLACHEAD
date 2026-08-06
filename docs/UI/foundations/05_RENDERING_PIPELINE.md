# 05_RENDERING_PIPELINE.md

# FLACHEAD UI — Rendering Pipeline Foundation

## 1. Purpose

This document defines the graphics rendering architecture used by FLACHEAD.

The rendering system is responsible for converting UI state into pixels on the 2.8" portrait TFT display while maintaining smooth performance on Raspberry Pi Zero W hardware.

FLACHEAD uses a custom lightweight renderer designed around:

* SDL-based rendering
* low memory usage
* predictable frame timing
* partial screen updates
* embedded hardware limitations

Primary principle:

> Render only what changes, and never waste processing power.

---

# 2. Design goals

The Rendering Pipeline must:

* Maintain stable 30-45 FPS.
* Reach 60 FPS where possible.
* Minimize CPU usage.
* Avoid unnecessary redraws.
* Support dynamic themes.
* Support animations.
* Handle small embedded displays efficiently.

The renderer should avoid:

* desktop-style rendering pipelines
* expensive effects
* real-time image processing
* unnecessary GPU assumptions

---

# 3. Rendering architecture

High-level flow:

```text id="m3x8k1"
Application State

        |

        v

UI Tree

        |

        v

Layout System

        |

        v

Renderer

        |

        v

SDL Backend

        |

        v

TFT Display
```

---

# 4. Renderer responsibilities

The Renderer handles:

* drawing components
* managing surfaces
* compositing layers
* applying transformations
* updating display regions

The Renderer does not handle:

* music playback
* input decisions
* navigation
* database operations

---

# 5. Frame lifecycle

Each frame follows:

```text id="q7p2n4"
Input Events

↓

Update State

↓

Calculate Changes

↓

Layout Update

↓

Render Changes

↓

Display Update
```

---

# 6. Main render loop

Recommended structure:

```text id="a8m5x3"
while(application_running)

{

    processInput();

    updateState();

    updateAnimations();

    render();

    present();

}
```

---

# 7. Frame timing

Target:

```text id="n5k8z2"
Ideal:

60 FPS

16.6ms/frame


Minimum:

30 FPS

33.3ms/frame
```

---

For Raspberry Pi Zero W:

Stable performance is more important than maximum FPS.

---

# 8. Rendering layers

FLACHEAD uses layered rendering.

Order:

```text id="w3q7m9"
Layer 0

Background


Layer 1

Screen Content


Layer 2

Persistent UI


Layer 3

Mini Player


Layer 4

Overlays


Layer 5

Debug Information
```

---

# 9. Dirty rectangle rendering

Full-screen redraws should be avoided.

Instead:

```text id="p9m4k7"
State Change

↓

Find Changed Area

↓

Redraw Region Only
```

Examples:

Volume change:

Bad:

```text id="z5n8x1"
Redraw entire screen
```

Good:

```text id="h2q6m9"
Update volume overlay area
```

---

# 10. Surface management

Assets should be loaded into reusable surfaces.

Examples:

* icons
* album artwork
* backgrounds
* fonts

Avoid:

```text id="x6v9m2"
Load image

↓

Render

↓

Delete

Every frame
```

---

Use:

```text id="c8m3p7"
Load once

↓

Cache

↓

Reuse
```

---

# 11. Asset cache

Required cache systems:

## Image Cache

Stores:

* artwork
* icons
* backgrounds

---

## Font Cache

Stores:

* loaded font faces
* rendered text surfaces

---

## Theme Cache

Stores:

* generated palettes
* processed assets

---

Example:

```text id="j5n2q8"
Asset Cache

├── images
├── fonts
├── artwork
└── themes
```

---

# 12. UI component rendering

Components follow:

```text id="f8m1x5"
Component

|

+-- update()

|

+-- layout()

|

+-- render()

|

+-- handleInput()
```

---

Components should draw only themselves.

---

# 13. Layout system

FLACHEAD uses a predictable layout system.

Priority:

1. fixed positioning
2. calculated regions
3. responsive scaling

Because:

* display size is known
* hardware is fixed

Avoid unnecessary responsive web-style layouts.

---

# 14. Coordinate system

Base resolution:

```text id="k3w7m2"
2.8" TFT

Portrait

Fixed coordinate space
```

Example:

```text id="p4x8n6"
Width

240px


Height

320px
```

(Actual resolution depends on final TFT model.)

---

# 15. Animation system

Animations should be lightweight.

Supported:

* fade
* slide
* scale
* progress movement

Avoid:

* particle systems
* blur
* complex physics

---

# 16. Animation architecture

Animations should be time-based.

Example:

```text id="m8q5v1"
Animation

start_time

duration

current_value

easing_function
```

---

Never tie animations directly to FPS.

---

# 17. Text rendering

Text is expensive.

Optimization:

* cache common labels
* avoid unnecessary font changes
* reuse rendered surfaces

---

Dynamic text:

Examples:

* song title
* time
* volume value

should update only when changed.

---

# 18. Image handling

Artwork pipeline:

```text id="b7n2x8"
Original Artwork

↓

Resize

↓

Cache

↓

Render
```

---

Never:

* decode large images during rendering
* resize every frame

---

# 19. Theme rendering

Themes provide tokens:

Example:

```text id="g4m9x2"
Primary Color

Background

Text

Accent
```

Renderer consumes these values.

The renderer does not generate themes.

---

# 20. Touch and rendering synchronization

Input and rendering are separate.

Flow:

```text id="q8m5p3"
Touch Event

↓

State Change

↓

Renderer Update

↓

Display Refresh
```

---

# 21. Performance optimization rules

Required:

* reuse memory
* avoid allocations inside render loop
* cache surfaces
* use partial redraws
* minimize texture operations

---

Avoid:

```text id="u6p2k9"
new Object()

inside:

while(rendering)
```

---

# 22. Debug rendering

Optional debug layer:

Shows:

* FPS
* frame time
* memory usage
* active screen

Example:

```text id="s3x7m8"
FPS: 45

Frame: 22ms

Memory: 90MB
```

---

Should be disabled in production.

---

# 23. Error handling

## Renderer failure

Attempt:

* restart renderer
* rebuild surfaces

---

## Missing asset

Fallback:

* placeholder icon
* default color

Never crash.

---

# 24. Acceptance criteria

Implementation is complete when:

* renderer maintains stable FPS
* UI updates are efficient
* assets are cached
* animations remain smooth
* screen redraws are minimized
* memory usage stays controlled
* rendering is independent from audio
* Pi Zero W remains responsive

---

# 25. Future improvements

Possible additions:

* hardware accelerated rendering
* framebuffer direct mode
* GPU-assisted effects
* OLED optimization
* adaptive quality mode
* waveform rendering

---

# 26. Architectural recommendations

## Renderer

Create:

```text id="v7n3m5"
Renderer

├── beginFrame()
├── drawComponent()
├── drawLayer()
├── updateRegion()
├── present()
└── cleanup()
```

---

## Render Scheduler

Create:

```text id="p8k2x6"
Render Scheduler

├── requestUpdate()
├── calculateDirtyRegions()
├── prioritizeUpdates()
└── controlFPS()
```

---

## Implementation notes for AI coding agent

When implementing:

* Optimize for Raspberry Pi Zero W first.
* Assume CPU limitations.
* Avoid unnecessary redraws.
* Keep the renderer deterministic.
* Separate UI state from rendering.
* Cache aggressively.
* Prefer simple visuals over expensive effects.

The final renderer should feel instant and fluid like a dedicated hardware music player while remaining lightweight enough for embedded hardware.
