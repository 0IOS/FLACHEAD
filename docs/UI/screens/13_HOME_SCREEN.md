# Home Screen

Component ID

HOME_SCREEN

Category

System Screen

Status

Stable

Depends On

• STATUS_BAR

• NAVIGATION_BAR

• MINI_PLAYER

• THEME_ENGINE

• WALLPAPER_ENGINE

• WIDGET_ENGINE

• INPUT_SYSTEM

• WINDOW_MANAGER

---

# Purpose

The Home Screen is the central hub of FLACHEAD.

It should feel calm, elegant and alive.

Unlike Android launchers, the Home Screen is not an application launcher.

It is the place users return to between tasks.

---

# Philosophy

Home is a place to breathe.

No clutter.

No advertisements.

No unnecessary information.

Every element exists because it improves usability.

---

# Goals

Fast.

Minimal.

Beautiful.

Wallpaper-first.

Music-first.

One-handed.

Instant.

---

# Layout

┌──────────────────────────────┐

Status Bar

──────────────────────────────

Wallpaper

Widgets

Mini Player (optional)

──────────────────────────────

Navigation Bar

└──────────────────────────────┘

---

# Wallpaper

Wallpaper occupies the full display.

No dimming.

No blur.

No overlays.

Wallpaper is always rendered at native resolution.

---

# Dynamic Theme

Wallpaper

↓

Palette Extraction

↓

Theme Generation

↓

Entire System

The wallpaper defines

Accent

Primary

Secondary

Surface

Text

Shadow

Focus

Album Art never changes the Home theme.

---

# Widgets

Widgets are optional.

Maximum

3

Recommended

2

Widgets never overlap.

Widgets never scroll.

---

# Supported Widgets

Clock

Weather (Future)

Storage

Battery

Music

Calendar (Future)

System Monitor

Developer HUD

Plugins (Future)

---

# Widget Placement

Top

Middle

Bottom

Snap Grid

Widgets align automatically.

No arbitrary placement.

---

# Widget Sizes

Small

1×1

Medium

2×1

Large

2×2

Full Width

Allowed

---

# Home Interaction

Touch

↓

Widget Interaction

Double Tap Home Button

↓

Return Home

Tap Launcher

↓

Open Launcher

Hold Home Button

↓

Task Overview

Swipe

Disabled by default.

---

# Empty State

Wallpaper only.

Perfectly valid.

Home should never require widgets.

---

# Mini Player

Optional.

Always docked above Navigation Bar.

Never overlaps widgets.

Automatically appears during playback.

---

# Wallpaper Engine

Supports

JPEG

PNG

WEBP

Future

Animated Wallpapers

Wallpaper cached.

Never decoded every frame.

---

# Animation

Boot

Fade

Wake

Fade

Sleep

Fade

Wallpaper Change

Crossfade

Widget Update

Fade

Mini Player

Slide

Navigation

Independent

Animations interruptible.

---

# Performance Budget

Frame Time

<16 ms

Wallpaper Draw

<1 ms

Widgets

<2 ms

Memory

<20 MB

---

# Rendering Order

Wallpaper

↓

Widgets

↓

Mini Player

↓

Status Bar

↓

Navigation Bar

↓

Developer Overlay

---

# Accessibility

Large Text

Supported

High Contrast

Supported

Reduced Motion

Supported

Screen Reader

Future

---

# Error Handling

Wallpaper Missing

↓

Default Wallpaper

Widget Failure

↓

Hide Widget

Theme Failure

↓

Default Theme

Never crash.

---

# Acceptance Criteria

✓ Instant wake

✓ Zero wallpaper flicker

✓ Pi Zero W budget

✓ Dynamic theme

✓ Widget support

✓ One-handed

✓ No unnecessary UI

---

# Future

Live Wallpapers

Smart Widgets

Plugin Widgets

Adaptive Themes

Weather

Calendar

AI Widgets

---

# Final Principle

The Home Screen should make users want to stay there even when they have nothing to do.