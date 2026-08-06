# Card Component

Component ID

CARD

Category

Container

Status

Stable

Depends On

• TEXT

• ICON

• IMAGE

• BUTTON

• THEME

• ANIMATION

---

# Purpose

Cards group related information into a single interactive surface.

Cards establish visual hierarchy, organize content, and reduce cognitive load.

They are the primary composition primitive throughout FLACHEAD.

---

# Philosophy

A card is not decoration.

A card exists because multiple elements belong together.

If removing the card improves readability,

the card should not exist.

---

# Responsibilities

Contain widgets.

Provide spacing.

Provide elevation.

Provide clipping.

Provide interaction.

Provide animations.

Nothing else.

---

# Widget Tree

Card

├── Background

├── Content Container

│   ├── Text

│   ├── Icons

│   ├── Images

│   ├── Buttons

│   └── Custom Widgets

└── Focus Ring

---

# Public API

SetStyle()

SetRadius()

SetPadding()

SetElevation()

SetBorder()

SetInteractive()

SetSelectable()

SetShadow()

SetContent()

Measure()

Layout()

Paint()

---

# Card Types

Surface Card

Album Card

Playlist Card

Artist Card

Application Card

Queue Card

Settings Card

Developer Card

Future Cards

Never create custom one-off cards.

---

# Standard Sizes

Small

96×96dp

Medium

160×160dp

Large

240×240dp

Flexible

Content Driven

---

# Padding

Small

8dp

Medium

12dp

Large

16dp

Extra Large

24dp

---

# Corner Radius

Small

8dp

Default

12dp

Large

16dp

Full

Circular

Never mix arbitrary radii.

---

# Elevation

Flat

Raised

Floating

Overlay

Default

Flat

Elevation should be subtle.

---

# Borders

Optional.

Theme controlled.

1dp.

Never use hardcoded colors.

---

# Shadows

Soft.

Cached.

Never animated independently.

---

# Background

Theme.Surface

Theme.SurfaceVariant

Theme.Player

Theme.Launcher

Never RGB values.

---

# Content Alignment

Top

Center

Bottom

Stretch

Default

Top

---

# Content Layout

Vertical

Horizontal

Grid

Stack

Custom

Cards never position children manually.

---

# States

Idle

Focused

Pressed

Selected

Loading

Disabled

Hidden

Collapsed

Error

---

# Focus

Accent outline.

Small scale

1.02

Glow

Optional.

---

# Pressed

Scale

0.98

Duration

80ms

Brightness

-5%

---

# Selected

Accent border.

Accent surface tint.

Selection indicator optional.

---

# Disabled

Opacity

50%

Ignore input.

---

# Loading

Skeleton placeholder.

Content hidden.

Card dimensions fixed.

---

# Error

Error border.

Optional icon.

Developer warning.

---

# Theme Integration

Surface

Border

Shadow

Focus

Selection

Text

Icons

Inherited automatically.

---

# Animation

Appear

Fade + Scale

Disappear

Fade

Focus

Glow

Press

Compress

Theme

Crossfade

Selection

Border Fade

Animations interruptible.

---

# Clipping

Children clipped to rounded bounds.

Overflow hidden by default.

---

# Scrolling

Cards never scroll.

ScrollView owns scrolling.

---

# Accessibility

Accessible Name

Required

Role

Group

Description

Optional

Children individually accessible.

---

# Rendering Order

Shadow

↓

Surface

↓

Border

↓

Content

↓

Overlay

↓

Focus Ring

↓

Debug

---

# Performance Budget

Measure

<0.05 ms

Layout

<0.05 ms

Paint

<0.12 ms

Memory

<6 KB

---

# Memory

Owns

Layout Cache

Animation State

Clip Geometry

No duplicated child resources.

---

# Error Handling

Empty Card

↓

Placeholder

Missing Child

↓

Continue Rendering

Invalid Layout

↓

Developer Warning

Never crash.

---

# Developer Overlay

Display

Card ID

Children

Padding

Bounds

Render Cost

Animation State

Theme Tokens

Memory

---

# Testing

Theme

Focus

Selection

Loading

Animation

Large Content

Nested Cards

Accessibility

Memory

Performance

---

# Anti Patterns

❌ Cards inside cards inside cards

❌ Decorative shadows

❌ Arbitrary padding

❌ RGB colors

❌ Per-frame shadow generation

❌ Child positioning inside card

❌ Inconsistent corner radius

---

# Acceptance Criteria

✓ Theme aware

✓ Animation aware

✓ Shared layout system

✓ Shared clipping

✓ Cached shadows

✓ Pi Zero W budget

✓ Zero allocations during paint

---

# Future

Glass cards

Dynamic blur cards

Adaptive elevation

Collapsible cards

Animated borders

Plugin-defined cards

---

# Final Principle

A card should organize information so naturally that users stop noticing the container and focus only on the content.