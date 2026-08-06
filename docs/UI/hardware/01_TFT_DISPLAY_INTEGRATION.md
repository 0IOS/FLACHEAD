# 01_TFT_DISPLAY_INTEGRATION.md

# FLACHEAD UI — TFT Display Integration Specification

## 1. Purpose

This document defines how FLACHEAD communicates with and optimizes rendering for the target TFT display.

The display is one of the most important hardware components because all UI decisions must respect:

* limited screen size
* Raspberry Pi Zero W performance
* refresh limitations
* power consumption

Primary principle:

> The display should deliver a premium audio player experience while using the minimum resources required.

---

# 2. Hardware assumptions

Target display:

```text id="x7m4q9"
Type:

2.8" TFT


Orientation:

Portrait


Controller:

Depends on hardware module


Resolution:

320 x 240 target
```

---

The UI layer must not depend on one specific display driver.

---

# 3. Display architecture

Data flow:

```text id="m5x8q3"
UI Components

↓

Renderer

↓

Display Backend

↓

Framebuffer / SPI / Driver

↓

TFT Panel
```

---

The renderer should remain hardware independent.

---

# 4. Display backend abstraction

Required interface:

```text id="v8m3x5"
Display Backend

├── Initialize

├── Draw Frame

├── Update Region

├── Set Brightness

├── Sleep Display

└── Wake Display
```

---

Possible backends:

* framebuffer
* direct SPI rendering
* SDL testing backend

---

# 5. Resolution handling

FLACHEAD targets portrait mode.

Internal coordinate system:

```text id="c6m4x9"
Width:

240


Height:

320
```

---

All UI layouts should use:

* relative positioning
* scalable components
* safe margins

---

# 6. Safe display margins

Required:

```text id="r7m3x8"
Top:

8-12px


Sides:

8-12px


Bottom:

8-12px
```

---

Reason:

Avoid edge clipping and touch issues.

---

# 7. Rendering strategy

Recommended:

Partial rendering.

---

Instead of:

```text id="n8m4x6"
Redraw entire screen
```

Use:

```text id="w5m3x7"
Update Changed Regions Only
```

---

Examples:

Volume overlay:

Only update overlay area.

---

Track change:

Only update:

* title
* artwork
* controls

---

# 8. Frame rate targets

Target:

```text id="a7m4x8"
60 FPS preferred
```

---

Minimum:

```text id="p6m8x2"
30 FPS stable
```

---

The UI must prioritize:

* responsiveness
* stable frame timing

over maximum effects.

---

# 9. Color handling

FLACHEAD is not monochrome.

Support:

* full RGB rendering
* album-based themes
* system themes

---

However:

Avoid expensive:

* real-time filters
* dynamic shaders

---

# 10. Image handling

Artwork is expensive.

Requirements:

* cache decoded images
* resize before rendering
* avoid decoding during frame updates

---

Pipeline:

```text id="u5m3x9"
FLAC Artwork

↓

Image Cache

↓

Scaled Bitmap

↓

Renderer
```

---

# 11. Font rendering

Requirements:

* bitmap-friendly fonts
* readable at small sizes
* low rendering cost

---

Avoid:

* expensive font effects
* unnecessary anti-aliasing

---

Recommended:

Cache frequently used text.

---

# 12. Refresh management

Display updates should be event-based.

Examples:

Track change:

Refresh player.

---

No event:

Do not redraw unnecessarily.

---

# 13. Display sleep behavior

When inactive:

Options:

```text id="k8m4x5"
Dim

↓

Sleep

↓

Wake
```

---

Playback continues.

---

# 14. Brightness control

Brightness handled through:

```text id="d7m3x8"
Display Manager
```

---

The UI requests:

```text id="h5m8x2"
Set Brightness
```

---

It should not access hardware directly.

---

# 15. Touch alignment considerations

Display coordinates must match:

* touch coordinates
* UI coordinates

---

Calibration data stored separately.

---

# 16. Performance requirements

Rendering must:

* avoid unnecessary allocations
* reuse buffers
* minimize copies
* avoid full redraws

---

Memory target:

Keep renderer lightweight.

---

# 17. Error handling

Display initialization failure:

Show:

```text id="s6m4x9"
Display Error

Restart Renderer
```

---

Fallback:

Attempt safe mode.

---

# 18. Acceptance criteria

Implementation is complete when:

* TFT initializes correctly
* rendering is stable
* touch aligns correctly
* refresh performance is acceptable
* display sleep works

---

# 19. Architectural recommendations

Required modules:

```text id="b8m3x5"
Display Manager

Renderer

Display Backend

Image Cache

Font Cache
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep rendering independent from hardware.
* Optimize for Pi Zero W first.
* Use dirty-region updates.
* Cache expensive resources.
* Avoid smartphone-style graphical effects.

FLACHEAD should feel smooth because it is efficient, not because it is overloaded with animations.
