# Image Component

Component ID

IMAGE

Category

Primitive

Status

Stable

---

# Purpose

The Image component is responsible for displaying every bitmap image in FLACHEAD.

This includes

• Album Art

• Wallpapers

• Artist Images

• Playlist Covers

• Placeholder Images

• Background Layers

• Future Visualizers

The Image component is one of the most performance-critical components in the entire system.

---

# Philosophy

Images support content.

They never reduce usability.

Performance always comes before image quality.

---

# Responsibilities

Load images.

Decode images.

Resize images.

Cache images.

Crop images.

Blur images.

Extract palettes.

Animate images.

Provide placeholders.

Handle failures gracefully.

Nothing else.

---

# Widget Tree

Image

(No children)

---

# Public API

SetSource()

SetPlaceholder()

SetScaling()

SetCropping()

SetCornerRadius()

SetBlur()

SetOpacity()

SetBorder()

SetShadow()

Reload()

Unload()

Measure()

PreferredSize()

---

# Supported Formats

Primary

JPEG

PNG

WEBP

Future

AVIF

HEIF

Animated WebP

Unsupported

GIF

BMP

TIFF

PSD

---

# Loading Pipeline

Filesystem

↓

Decoder

↓

Resize

↓

Color Space

↓

Palette Extraction

↓

Texture Upload

↓

Image Cache

↓

Renderer

---

# Image States

Empty

Loading

Loaded

Placeholder

Error

Destroyed

---

# Scaling Modes

Fit

Fill

Stretch

Center

Center Crop

Tile (Future)

Default

Center Crop

---

# Cropping

Manual

Automatic

Face Detection (Future)

Album Art

Always Square

Wallpaper

Preserve Aspect Ratio

---

# Corner Radius

None

Small

Medium

Large

Circular

Custom

Never use inconsistent radii.

---

# Borders

Optional.

Theme controlled.

No hardcoded colors.

---

# Shadows

Optional.

Used only where defined by design system.

Never baked into images.

---

# Blur

Gaussian Blur

Optimized

Cached

Never blur every frame.

Blur is generated once.

---

# Wallpaper Rules

Wallpaper exists only once.

Shared globally.

Never duplicate wallpaper textures.

Home

Sharp

Launcher

Blurred

Task Overview

Blurred

Player

Hidden

---

# Album Art Rules

Square.

Center Crop.

Dominant Palette Extracted.

Fallback

Embedded Art

↓

Folder Art

↓

Placeholder

---

# Placeholder Images

Album Placeholder

Artist Placeholder

Playlist Placeholder

Folder Placeholder

Unknown Artwork

Theme aware.

---

# Dynamic Theme

Album Art

↓

Palette Extraction

↓

Theme Generator

↓

Player Theme

Wallpaper

↓

Palette Extraction

↓

Global Theme

Image component never generates themes directly.

---

# Image Cache

Global

Reference Counted

LRU Eviction

Maximum configurable size

Target

95% cache hit rate

---

# Texture Cache

Separate from image cache.

One decoded image

↓

Multiple GPU textures

if necessary.

---

# Memory Budget

Thumbnail

≤64 KB

Album Art

≤512 KB

Wallpaper

≤2 MB

Global Cache

Configurable

---

# Performance Budget

Decode

Background Thread

Resize

Background Thread

Upload

Main Thread

Draw

<0.15 ms

---

# Threading

Filesystem

Worker Thread

Decode

Worker Thread

Resize

Worker Thread

Palette Extraction

Worker Thread

GPU Upload

UI Thread

Never block rendering.

---

# Animation

Supported

Fade

Crossfade

Zoom

Parallax (Wallpaper)

Rotation (Future)

Never animate decoding.

---

# Transition Rules

Album Change

Crossfade

Wallpaper Change

Fade

Placeholder

Instant

Theme Change

Synchronized

---

# Accessibility

Alternative Description

Supported

Decorative Images

Supported

Future Screen Reader

Supported

---

# Error Handling

Missing File

↓

Placeholder

Corrupt File

↓

Placeholder

Decode Failure

↓

Developer Warning

Renderer continues.

Never crash.

---

# Developer Overlay

Image ID

Resolution

Decode Time

Upload Time

Palette Time

Cache Status

Memory Usage

Texture ID

Blur Status

---

# Testing

Large Images

Small Images

Corrupt Files

Rapid Changes

Theme Changes

Cache Eviction

Blur

Palette Extraction

Thread Safety

---

# Anti Patterns

Per-frame decoding

Per-frame blur

Per-frame resizing

Duplicate textures

Blocking filesystem

Synchronous loading

Huge wallpapers

Hardcoded placeholders

---

# Acceptance Criteria

✓ Async loading

✓ Shared cache

✓ Shared wallpaper

✓ Palette extraction

✓ Cached blur

✓ Pi Zero W budget

✓ Graceful failure

✓ Zero allocations during draw

---

# Future

Progressive JPEG

Animated Album Art

HDR

HEIF

GPU Blur

Live Wallpapers

Vector Artwork

---

# Final Principle

Images should enrich the experience without ever becoming the reason a frame is dropped.