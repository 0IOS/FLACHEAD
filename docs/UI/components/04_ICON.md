# Icon Component

Component ID

ICON

Category

Primitive

Status

Stable

---

# Purpose

The Icon component renders scalable symbolic graphics throughout FLACHEAD.

Icons should communicate meaning instantly while remaining visually consistent across the entire operating system.

Icons must be lightweight enough to render smoothly on Raspberry Pi Zero W hardware.

---

# Philosophy

Icons communicate.

They never decorate without purpose.

Every icon must be understandable even without text.

---

# Responsibilities

Render icons.

Scale correctly.

Theme correctly.

Animate.

Support accessibility.

Cache geometry.

Nothing else.

---

# Widget Tree

Icon

(No children)

---

# Public API

SetIcon()

SetSize()

SetColor()

SetWeight()

SetRotation()

SetOpacity()

SetAnimated()

Measure()

PreferredSize()

---

# Icon Source

Primary

SVG

Internal vector format

Secondary

Monochrome bitmap cache

Generated automatically.

Never ship bitmap assets manually.

---

# Icon Style

Rounded

Simple

Filled

Geometric

Consistent stroke weight

No realistic artwork.

No gradients.

No shadows baked into assets.

---

# Supported Sizes

16dp

20dp

24dp

28dp

32dp

40dp

48dp

64dp

Scaling outside these sizes should be avoided.

---

# Standard Icons

Navigation

Back

Home

Launcher

Search

Settings

Music

Play

Pause

Previous

Next

Shuffle

Repeat

Queue

Volume

Mute

Library

Album

Artist

Playlist

Folder

Storage

Bluetooth

Wi-Fi

Battery

Charging

Sleep

Power

Brightness

Theme

Developer

Performance

Warning

Error

Success

Info

Loading

Future icons must follow the same visual language.

---

# Rendering

Vector icons are rasterized once.

Generated textures are cached.

Rendering never rasterizes every frame.

---

# Icon Cache

Shared globally.

Reference counted.

LRU eviction.

Never duplicate identical icons.

---

# Theme Integration

Icons receive colors from

Theme.IconPrimary

Theme.IconSecondary

Theme.IconDisabled

Theme.IconAccent

Theme.Error

Theme.Warning

Never use hardcoded colors.

---

# Alignment

Icons align to pixel boundaries whenever possible.

Avoid half-pixel rendering.

---

# Animation

Supported

Fade

Rotate

Scale

Pulse

Morph (future)

Never bounce.

Never overshoot.

---

# Focus

Focused icons receive

Brightness increase

Slight scale

Focus ring supplied by parent widget.

---

# Disabled State

Reduced opacity.

No animation.

Ignore input.

---

# Accessibility

Every icon exposes

Accessible Name

Description

Role

Icons that are purely decorative should be marked decorative.

---

# Performance Budget

Measure

<0.01 ms

Draw

<0.03 ms

Memory

<1 KB per cached instance

---

# Draw Order

Shadow (optional)

↓

Icon

↓

Debug Overlay

---

# Error Handling

Missing icon

↓

Fallback "Unknown" icon

Invalid asset

↓

Warning in developer mode

Never crash.

---

# Developer Overlay

Display

Icon ID

Cache Hit

Texture Size

Render Time

Source Asset

---

# Testing

Scaling

Rotation

Opacity

Theme Switching

Animation

Cache Reuse

Missing Assets

Pixel Alignment

---

# Anti Patterns

Hardcoded colors

Bitmap-only icons

Multiple copies of same icon

Large detailed artwork

Photorealistic icons

Random stroke widths

---

# Acceptance Criteria

✓ Cached rendering

✓ Theme aware

✓ Animation aware

✓ Pixel aligned

✓ Shared cache

✓ Pi Zero W budget respected

---

# Future

Animated SVG

Variable stroke weight

Theme-generated icons

Custom icon packs

User icon themes

---

# Final Principle

Icons should be recognizable in less than a second and should never distract from the content they support.