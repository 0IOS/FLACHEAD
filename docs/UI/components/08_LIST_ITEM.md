# List Item Component

Component ID

LIST_ITEM

Category

Composite

Status

Stable

Depends On

• CARD

• TEXT

• IMAGE

• ICON

• BUTTON

• THEME

• ANIMATION

---

# Purpose

ListItem represents one logical entry inside a List.

It is responsible only for presenting data.

It never owns the underlying data.

It never performs scrolling.

---

# Philosophy

Every row should communicate exactly one piece of information.

Users should understand the purpose of a row within a fraction of a second.

Density should never reduce readability.

---

# Responsibilities

Display content.

Handle selection.

Handle focus.

Handle interaction.

Animate state changes.

Expose accessibility.

Nothing else.

---

# Widget Tree

List Item

├── Leading
│   ├── Image
│   └── Icon
│
├── Content
│   ├── Primary Text
│   ├── Secondary Text
│   └── Metadata
│
├── Trailing
│   ├── Icon
│   ├── Button
│   └── Badge
│
└── Focus Ring

---

# Public API

SetLeading()

SetPrimaryText()

SetSecondaryText()

SetMetadata()

SetTrailing()

SetSelectable()

SetEnabled()

SetStyle()

SetAction()

Update()

Invalidate()

---

# Layout

Leading

↓

Content

↓

Trailing

Always.

Never rearranged arbitrarily.

---

# Height

Compact

56dp

Default

72dp

Comfortable

88dp

Album

96dp

Dynamic

Allowed.

---

# Leading Area

Album Art

Artist Image

Application Icon

Folder Icon

Playlist Cover

Status Icon

Size

40dp

56dp

72dp

Theme controlled.

---

# Content Area

Primary Text

Required.

Secondary Text

Optional.

Metadata

Optional.

Maximum

3 text lines.

---

# Trailing Area

Chevron

Play Button

More Button

Toggle

Badge

Progress

Never overflow.

---

# Alignment

Vertical Center

Default.

Multi-line items align from top.

---

# Selection

Background Tint

Accent Strip

Optional Checkmark

Selection survives scrolling.

---

# Focus

Accent outline.

Scale

1.01

No large movement.

---

# States

Idle

Focused

Pressed

Selected

Disabled

Loading

Error

Hidden

Collapsed

---

# Pressed

Background darkens.

Scale

0.99

Duration

70ms

Immediate feedback.

---

# Disabled

Opacity

50%

Ignore input.

---

# Loading

Skeleton placeholders.

Dimensions fixed.

No layout shift.

---

# Error

Error icon.

Developer warning.

Optional retry action.

---

# Theme

Background

Text

Secondary Text

Icons

Selection

Focus

Divider

Inherited automatically.

---

# Dividers

Optional.

1dp.

Inset supported.

Theme controlled.

---

# Animation

Appear

Fade

Disappear

Fade

Insert

Slide

Remove

Slide

Selection

Crossfade

Focus

Glow

Animations interruptible.

---

# Accessibility

Role

List Item

Name

Primary Text

Description

Secondary Text

Position

Supported

Selection State

Supported

---

# Input

Supported

Touch

GPIO

Keyboard

Encoder

Remote

Identical behaviour.

---

# Touch

Tap

↓

Primary Action

Long Press

↓

Context Menu

Double Tap

↓

Optional

---

# GPIO

Focus

↓

Press

↓

Primary Action

---

# Keyboard

Enter

↓

Action

Space

↓

Selection

---

# Encoder

Rotate

↓

Navigation

Click

↓

Action

Hold

↓

Context Menu

---

# Rendering Order

Background

↓

Leading

↓

Content

↓

Trailing

↓

Focus Ring

↓

Debug Overlay

---

# Performance Budget

Measure

<0.02 ms

Layout

<0.04 ms

Draw

<0.10 ms

Memory

<5 KB

---

# Memory

Owns

Animation State

Layout Cache

Nothing else.

Images and text remain shared.

---

# Error Handling

Missing Image

↓

Placeholder

Missing Text

↓

Developer Warning

Missing Action

↓

Ignore

Never crash.

---

# Developer Overlay

Display

Item ID

Index

Visible

Focused

Selected

Draw Time

Layout Time

Memory

Theme

---

# Testing

Large Lists

Rapid Scroll

Selection

Focus

Accessibility

Theme Change

Animation Interrupt

Memory

Performance

---

# Anti Patterns

❌ More than three text lines

❌ Inconsistent spacing

❌ Different heights

❌ Decorative icons

❌ Hardcoded colors

❌ Large touch targets overlapping

❌ Per-frame allocations

---

# Acceptance Criteria

✓ Theme aware

✓ Virtualization compatible

✓ Shared typography

✓ Shared image cache

✓ Shared icon cache

✓ Pi Zero W budget

✓ Zero allocations during draw

---

# Future

Expandable rows

Swipe actions

Inline editing

Drag & Drop

Pinned rows

Animated metadata

Live progress indicators

---

# Final Principle

A ListItem should be reusable enough that nearly every scrolling interface in FLACHEAD can be built from it without modification.