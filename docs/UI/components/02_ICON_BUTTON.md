# Icon Button Component

Component ID

ICON_BUTTON

Category

Composite

Status

Stable

Depends On

• ICON

• THEME

• ANIMATION

• INPUT

---

# Purpose

The Icon Button component provides compact, icon-only actions throughout FLACHEAD.

Unlike a standard Button, Icon Buttons prioritize speed, clarity, and muscle memory.

They are used for navigation, playback controls, toolbar actions, and quick actions.

---

# Philosophy

One icon.

One action.

Zero ambiguity.

If a user hesitates about what an icon means,

it is the wrong icon.

---

# Responsibilities

Display an icon.

Handle input.

Animate.

Display focus.

Expose accessibility.

Nothing else.

---

# Widget Composition

Icon Button

├── Background

├── Icon

└── Focus Ring

---

# Public API

SetIcon()

SetSize()

SetStyle()

SetEnabled()

SetToggle()

SetChecked()

SetAction()

SetTooltip()

Press()

Release()

Cancel()

---

# Standard Sizes

Small

40dp

Medium

48dp

Large

56dp

Playback

64dp

Touch target

Never below 48dp.

---

# Shapes

Circle

Rounded Square

Square

Pill

Default

Circle

---

# Styles

Filled

Outline

Ghost

Primary

Danger

Success

Developer

---

# Common Uses

Navigation

Back

Home

Search

Launcher

Settings

Playback

Play

Pause

Previous

Next

Shuffle

Repeat

Queue

Like

Volume

Bluetooth

Sleep

Power

Brightness

Developer

Performance

Logs

Profiler

---

# States

Idle

Focused

Pressed

Checked

Disabled

Loading

Error

Hidden

Collapsed

---

# Idle

Normal colors.

No animation.

---

# Focused

Focus ring.

Scale

1.03

Accent glow.

---

# Pressed

Scale

0.94

Duration

70ms

Brightness

-10%

Immediate feedback.

---

# Released

Return

100ms

Ease Out.

---

# Toggle Mode

Supported.

Examples

Repeat

Shuffle

Favourite

Bluetooth

Sleep Timer

Checked state uses accent color.

---

# Disabled

Reduced opacity.

Ignore input.

No animation.

---

# Loading

Spinner replaces icon.

Button dimensions remain unchanged.

---

# Error

Brief shake.

Error color.

Return to previous state.

---

# Theme Tokens

Surface

Accent

Primary

Secondary

Disabled

Error

Focus

Border

No hardcoded colors.

---

# Icon Rules

Maximum size

24dp

Centered.

Always pixel aligned.

Never stretch icons.

---

# Input

Supported

Touch

GPIO

Keyboard

Encoder

Remote

Identical behaviour across all devices.

---

# Touch

Tap

↓

Action

Long Press

Optional

Double Tap

Optional

---

# GPIO

Focus

↓

Press

↓

Action

---

# Keyboard

Enter

↓

Action

Space

↓

Action

---

# Encoder

Rotate

↓

Focus

Click

↓

Action

Hold

↓

Context Menu (Future)

---

# Accessibility

Accessible Name

Required

Role

Button

Description

Optional

Touch Target

48dp minimum

---

# Rendering Order

Shadow

↓

Surface

↓

Border

↓

Icon

↓

Focus Ring

↓

Debug

---

# Animation

Appear

Fade

Focus

Scale

Press

Compress

Loading

Spinner

Theme

Crossfade

Disappear

Fade

Animations are interruptible.

---

# Performance Budget

Measure

<0.01 ms

Layout

<0.02 ms

Draw

<0.05 ms

Memory

<2 KB

---

# Memory

Owns

Animation State

Nothing else.

Icons remain shared.

---

# Error Handling

Missing Icon

↓

Unknown Icon

Missing Action

↓

Developer Warning

Never crash.

---

# Developer Overlay

Display

Component ID

Icon

Action

State

Focus

Animation

Draw Time

Memory

---

# Testing

Rapid Press

Toggle

Disabled

Loading

Theme

Animation Interrupt

Focus

Accessibility

Memory

---

# Anti Patterns

❌ Tiny touch targets

❌ Decorative icons

❌ Hardcoded colors

❌ Multiple meanings

❌ Different animations

❌ Different sizing

---

# Acceptance Criteria

✓ Theme aware

✓ Animation aware

✓ Shared icon cache

✓ Shared input system

✓ Shared focus system

✓ Pi Zero W budget

✓ Zero allocations during draw

---

# Future

Animated icons

Pressure sensitivity

User icon packs

Haptic feedback

Adaptive sizing

---

# Final Principle

An Icon Button should become muscle memory.

Users should stop seeing the icon and simply perform the action.