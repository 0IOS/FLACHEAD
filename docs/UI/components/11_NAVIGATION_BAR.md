# Navigation Bar

Component ID

NAVIGATION_BAR

Category

System UI

Status

Stable

Depends On

• BUTTON

• ICON_BUTTON

• THEME

• INPUT

• ANIMATION

• WINDOW_MANAGER

---

# Purpose

Navigation Bar is the permanent navigation surface of FLACHEAD.

It provides instant access to

• Home

• Launcher

• Back

It is visible on every screen except Fullscreen modes.

---

# Philosophy

The Navigation Bar should disappear into muscle memory.

Users should never think about it.

---

# Position

Portrait Only

Bottom of screen.

Always centered.

Never movable.

---

# Size

Height

72dp

Safe Area

Included

Corner Radius

16dp

Horizontal Padding

16dp

Bottom Padding

12dp

---

# Layout

┌────────────────────────────────────┐

◀          ○          △

└────────────────────────────────────┘

Back

Home

Launcher

Equal spacing.

---

# Button Mapping

Left

Back

Center

Home

Right

Launcher

No other permanent buttons.

---

# Back Button

Tap

↓

Navigate Back

Hold

↓

No Action

Double Tap

↓

No Action

Unavailable

↓

Disabled

---

# Home Button

Tap

↓

Go Home

Double Tap

↓

Return to Home immediately

Always.

Hold

↓

Open Task Overview

Release

↓

Remain in Task Overview

---

# Launcher Button

Tap

↓

Toggle Launcher

If launcher open

↓

Close Launcher

Hold

↓

Future

Quick Search

Double Tap

↓

No Action

---

# Visibility

Visible

Home

Library

Player

Settings

Launcher

Task Overview

Hidden

Video

Future Fullscreen Apps

Developer Override

Allowed.

---

# Background

Floating Surface

Blurred

Opacity

85%

Wallpaper visible underneath.

---

# Dynamic Theme

Uses wallpaper palette.

Never uses album art palette.

Album palette belongs exclusively to the Music Player.

---

# Theme Tokens

Navigation Surface

Navigation Border

Navigation Icons

Navigation Focus

Navigation Shadow

Navigation Blur

---

# Focus

GPIO

Keyboard

Remote

One focused button.

Focus wraps.

Touch ignores focus.

---

# Input

Touch

GPIO

Keyboard

Encoder

Bluetooth Remote

Future

Gamepad

Identical behaviour.

---

# Touch

Tap

↓

Action

Hold

↓

If Home

↓

Task Overview

Double Tap

↓

Home

---

# GPIO

Back Button

↓

Back

Home Button

↓

Home

Launcher Button

↓

Launcher

---

# Keyboard

Esc

↓

Back

Super

↓

Launcher

Home

↓

Home

---

# Encoder

Rotate

↓

Focus Buttons

Click

↓

Activate

Hold

↓

Task Overview

---

# Animations

Appear

Fade + Slide

Disappear

Fade

Focus

Glow

Press

Scale

Theme

Crossfade

Blur

Crossfade

Duration

150ms

---

# States

Visible

Hidden

Focused

Pressed

Disabled

Animating

---

# Performance Budget

Draw

<0.10ms

Animation

<0.05ms

Memory

<8KB

No allocations during rendering.

---

# Accessibility

Minimum Touch Target

48dp

Accessible Names

Required

High Contrast

Supported

Reduced Motion

Supported

---

# Developer Overlay

Show

Focused Button

Navigation State

Animation

Blur Cost

Draw Time

Memory

Theme Tokens

---

# Error Handling

Invalid Screen

↓

Remain Visible

Missing Icons

↓

Fallback Icons

Theme Failure

↓

Default Theme

Never crash.

---

# Acceptance Criteria

✓ Bottom anchored

✓ Dynamic wallpaper colors

✓ Floating appearance

✓ Blur cached

✓ Pi Zero W budget

✓ No layout shift

✓ Works with every input device

---

# Future

Gesture Navigation

Adaptive Layout

Landscape

Custom Navigation

Plugin Buttons

---

# Final Principle

Navigation should become instinctive.

Users should move through FLACHEAD without consciously thinking about the navigation bar.