# List Component

Component ID

LIST

Category

Container

Status

Stable

Depends On

• CARD

• TEXT

• IMAGE

• ICON

• SCROLLVIEW

• INPUT

• ANIMATION

---

# Purpose

The List component efficiently displays ordered collections of items.

Lists power almost every major screen inside FLACHEAD.

A List never owns data.

It visualizes data supplied by a model.

---

# Philosophy

A list should feel infinite.

Whether it contains

10 items

or

100,000 items

scrolling should feel identical.

---

# Responsibilities

Display items.

Virtualize.

Recycle widgets.

Handle selection.

Handle focus.

Handle scrolling.

Handle keyboard navigation.

Nothing else.

---

# Architecture

Data Model

↓

List Adapter

↓

Virtual List

↓

Visible Items

↓

Renderer

The List never talks directly to storage.

---

# Widget Tree

List

├── ScrollView

│   ├── ListItem

│   ├── ListItem

│   ├── ListItem

│   └── ...

└── Scroll Bar (Optional)

---

# Public API

SetModel()

SetDelegate()

Reload()

ScrollTo()

ScrollBy()

Select()

Focus()

CurrentIndex()

VisibleRange()

ItemCount()

Invalidate()

---

# Orientation

Vertical

Horizontal

Grid (future)

Default

Vertical

---

# Item Source

Model Interface

Required.

The List owns no data.

---

# Virtualization

Always enabled.

Only visible items exist.

Off-screen widgets are recycled.

Never create thousands of widgets.

---

# Recycling

Visible

↓

Leaving viewport

↓

Reset

↓

Recycle Pool

↓

Reuse

No heap allocations during scrolling.

---

# Selection Modes

None

Single

Multiple

Range (future)

Default

Single

---

# Focus

Exactly one focused item.

Focus survives scrolling.

Focus restored after navigation.

---

# Navigation

Touch

GPIO

Keyboard

Encoder

Remote

Every input method behaves identically.

---

# Scrolling

Smooth.

Pixel based.

Momentum optional.

Never jump unexpectedly.

---

# Scroll Position

Stored.

Returning to a screen restores

Position

Selection

Focus

---

# Item Spacing

Theme controlled.

Default

8dp

Never hardcoded.

---

# Padding

Top

Bottom

Leading

Trailing

Theme controlled.

---

# Empty State

No items

↓

Placeholder View

↓

Optional Action Button

---

# Loading State

Skeleton Items

Progress Indicator

Dimensions remain stable.

---

# Error State

Error View

Retry Action

Developer Log

---

# Theme

Background

Selection

Focus

Divider

Scrollbar

Placeholder

Inherited automatically.

---

# Divider

Optional.

1dp.

Theme controlled.

Never RGB.

---

# Scroll Bar

Overlay.

Auto-hide.

Fade animation.

Never occupies layout space.

---

# Animation

Insert

Fade + Slide

Remove

Fade

Move

Translate

Selection

Color Transition

Theme

Crossfade

Animations interruptible.

---

# Accessibility

List Role

Supported

Item Count

Supported

Current Item

Supported

Future Screen Reader

Supported

---

# Performance Budget

Layout

<0.1 ms

Draw

<0.2 ms

Memory

Visible Items Only

Scrolling

Zero allocations

---

# Memory

Owns

Visible Widgets

Recycle Pool

Scroll State

Nothing else.

---

# Error Handling

Null Model

↓

Placeholder

Missing Delegate

↓

Developer Warning

Invalid Index

↓

Ignore

Never crash.

---

# Developer Overlay

Display

Item Count

Visible Count

Recycled Count

FPS

Scroll Position

Memory

Pool Size

Layout Time

Draw Time

---

# Testing

Large Lists

100,000 Items

Rapid Scroll

Theme Change

Selection

Keyboard

GPIO

Encoder

Memory

Animation

Accessibility

---

# Anti Patterns

❌ Creating one widget per item

❌ Synchronous loading

❌ Rebuilding entire list

❌ Per-frame allocations

❌ Manual scrolling

❌ Hardcoded spacing

---

# Acceptance Criteria

✓ Virtualized

✓ Recycled

✓ Zero allocations while scrolling

✓ Pi Zero W budget

✓ Theme aware

✓ Animation aware

✓ Input independent

---

# Future

Sticky Headers

Grouped Lists

Section Index

Infinite Loading

Drag & Drop

Animated Reordering

Nested Lists

---

# Final Principle

A list should feel exactly as fast with one hundred thousand items as it does with ten.