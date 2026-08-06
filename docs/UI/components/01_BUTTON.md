# Button Component

Component ID

BUTTON

Category

Composite

Status

Stable

Depends On

• TEXT

• ICON

• THEME

• ANIMATION

• INPUT

---

# Purpose

The Button component executes an action initiated by the user.

Buttons are the primary interaction mechanism throughout FLACHEAD.

Every button must feel immediate, tactile and responsive.

---

# Philosophy

Buttons invite interaction.

A button should always communicate

• It is interactive

• It can be focused

• It is currently pressed

• It is disabled

without requiring text.

---

# Responsibilities

Display text.

Display optional icon.

Handle actions.

Animate.

Handle focus.

Handle accessibility.

Handle state.

Nothing else.

---

# Widget Composition

Button

├── Background

├── Icon (Optional)

├── Text

└── Focus Ring

---

# Public API

SetText()

SetIcon()

SetEnabled()

SetLoading()

SetPrimary()

SetSecondary()

SetDanger()

SetOutline()

SetFilled()

SetSize()

SetAction()

SetTooltip()

Press()

Release()

Cancel()

---

# Button Types

Primary

Secondary

Outline

Ghost

Danger

Success

Icon Left

Icon Right

Loading

Developer

Never invent one-off button styles.

---

# Standard Sizes

XS

32dp

Small

40dp

Medium

48dp

Large

56dp

XL

64dp

Touch targets are never smaller than 48dp.

---

# Width

Content

Preferred.

Full Width

Allowed.

Fixed Width

Only when necessary.

---

# Internal Layout

Horizontal Padding

16dp

Vertical Padding

12dp

Gap

8dp

Centered vertically.

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

No additional states.

---

# Idle

Normal elevation.

Normal colors.

No animation.

---

# Focused

Focus ring.

Slight scale.

Accent highlight.

No shadow changes.

---

# Pressed

Scale

0.97

Brightness

-8%

Duration

80ms

Input feedback

Immediate.

---

# Released

Spring back.

120ms

Ease Out.

---

# Disabled

Reduced opacity.

No interaction.

No hover.

No press animation.

---

# Loading

Spinner replaces icon.

Text remains.

Button width never changes.

Input disabled.

---

# Error

Optional shake.

Theme.Error.

Returns to Idle.

---

# Theme Tokens

Background

Surface

Primary

Secondary

Danger

Border

Focus

Text

Disabled

Never use RGB values.

---

# Typography

Uses

Body Medium

Uppercase

Disabled

Sentence Case

Preferred.

---

# Icons

Optional.

Always aligned to text baseline.

Never oversized.

Maximum

24dp.

---

# Alignment

Centered

Default.

Leading

Allowed for list buttons.

Never right-aligned alone.

---

# Animation

Appear

Fade + Scale

Focus

Glow + Scale

Press

Scale

Loading

Spinner Rotation

Disable

Fade

Theme Change

Crossfade

Animations are interruptible.

---

# Input

Supported

Touch

GPIO

Keyboard

Encoder

Bluetooth Remote

Every method produces identical behaviour.

---

# Touch Behaviour

Tap

↓

Press

↓

Release

↓

Action

Long Press

Optional.

---

# GPIO Behaviour

Focus

↓

Press

↓

Release

↓

Action

Repeat disabled by default.

---

# Keyboard

Enter

↓

Action

Space

↓

Action

Escape

↓

Cancel

---

# Encoder

Click

↓

Action

Rotate

↓

Focus Navigation

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

Minimum Touch Target

48dp

High Contrast

Supported

Reduced Motion

Supported

---

# Focus Rules

One focused button only.

Focus ring owned by Button.

Parent manages navigation.

---

# Draw Order

Shadow

↓

Background

↓

Border

↓

Icon

↓

Text

↓

Focus Ring

↓

Debug

---

# Performance Budget

Measure

<0.02 ms

Layout

<0.03 ms

Draw

<0.08 ms

Memory

<4 KB

---

# Memory

Button owns

Animation State

Local Layout Cache

Nothing else.

Text/Icon remain shared.

---

# Error Handling

Missing Icon

↓

Text Only

Missing Text

↓

Icon Only

Missing Both

↓

Developer Warning

Never crash.

---

# Developer Overlay

Display

Button ID

Current State

Focus

Animation

Theme Tokens

Render Cost

Action ID

Bounds

---

# Testing

Touch

GPIO

Keyboard

Encoder

Loading

Disabled

Rapid Press

Theme Switch

Animation Interrupt

Memory

---

# Anti Patterns

❌ Tiny buttons

❌ RGB colors

❌ Different press animations

❌ Multiple focus rings

❌ Layout shifts while loading

❌ Blocking actions

❌ Hardcoded spacing

---

# Acceptance Criteria

✓ Theme aware

✓ Shared animations

✓ Shared typography

✓ Shared icons

✓ Zero allocations during draw

✓ Pi Zero W budget

✓ Works with every input device

---

# Future

Ripple Effects

Pressure-sensitive buttons

Voice activation

Custom button themes

Adaptive sizing

---

# Final Principle

A button should feel responsive enough that users trust it before they even see the resulting action.