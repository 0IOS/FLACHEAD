# Task Overview

Component ID

TASK_OVERVIEW

Category

System Screen

Status

Stable

Depends On

• STATUS_BAR
• NAVIGATION_BAR
• CARD
• LIST
• WINDOW_MANAGER
• APPLICATION_MANAGER
• THEME_ENGINE
• WALLPAPER_ENGINE
• ANIMATION_ENGINE

---

# Purpose

Task Overview displays all currently running applications.

It allows users to

• Switch applications

• Close applications

• Inspect running state

without interrupting their workflow.

---

# Philosophy

Switching applications should feel instantaneous.

The user should always know what is running.

The overview should never feel cluttered.

---

# Entry

Hold Home Button

↓

Task Overview Opens

Release

↓

Remain Open

Tap Outside (Optional)

↓

Close

Back

↓

Close

---

# Exit

Select Application

↓

Bring To Front

Home

↓

Home Screen

Back

↓

Previous Screen

---

# Layout

┌────────────────────────────┐

Status Bar

────────────────────────────

Running Applications

(List)

────────────────────────────

Memory Summary

────────────────────────────

Navigation Bar

└────────────────────────────┘

---

# Background

Wallpaper

↓

Blur

↓

Dark Overlay

↓

Task Cards

Blur Radius

20dp

Opacity

80%

---

# Running Application Card

Contains

Application Icon

Application Name

Running Time

Memory Usage

State Indicator

Optional Thumbnail

---

# Application State

Foreground

Accent Border

Background

Muted Accent

Suspended

Gray

Audio Active

Music Badge

Updating

Spinner

Crash

Warning Icon

---

# Thumbnail

Optional.

Default

Disabled

Reason

Pi Zero W optimization.

Future

Low Resolution Preview

Maximum

160×90

Cached.

---

# Memory Summary

Bottom section.

Displays

Used RAM

Free RAM

Running Apps

Swap Usage

Developer Mode

Additional metrics

---

# Focus

Exactly one application focused.

Focus restored after reopening.

---

# Navigation

Touch

Tap Card

GPIO

Vertical Navigation

Keyboard

Arrow Keys

Encoder

Rotate

Selection wraps.

---

# Input

Touch

GPIO

Keyboard

Encoder

Remote

Future

Mouse

---

# Touch

Tap

↓

Switch App

Long Press

↓

Context Menu

Swipe Left

↓

Close App

Swipe Right

↓

No Action

---

# Context Menu

Resume

Force Stop

Application Info

Developer Info

Future

Pin

---

# Closing Applications

User closes app

↓

Graceful Shutdown

↓

Timeout

↓

Force Terminate

Never immediately kill unless necessary.

---

# Sorting

Foreground

↓

Background Audio

↓

Recently Used

↓

Suspended

Default sorting is automatic.

---

# Search

Future Feature.

Search running applications.

---

# Theme

Wallpaper Palette

↓

Task Overview

Wallpaper remains blurred.

Album palette never used.

---

# Animations

Open

Scale + Fade

Close

Fade

Switch

Zoom

Close App

Slide Out

Focus

Glow

Duration

140ms

Animations interruptible.

---

# Performance Budget

Open

<60ms

Switch

<20ms

Draw

<0.3ms

Memory

<10MB

---

# Rendering Order

Wallpaper

↓

Blur

↓

Cards

↓

Memory Summary

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

Touch Target

48dp

---

# Error Handling

Missing App

↓

Hide Card

Crash During Switch

↓

Toast

Invalid State

↓

Refresh List

Never crash.

---

# Developer Overlay

Display

Running Apps

Foreground PID

RSS Memory

CPU Usage

GPU Usage

Frame Time

Window State

---

# Acceptance Criteria

✓ Opens under 60ms

✓ App switch under 20ms

✓ Pi Zero W optimized

✓ Blur cached

✓ Zero allocations while idle

✓ Theme aware

---

# Future

Split Screen

Pinned Apps

Application Groups

Thumbnail Preview

Plugin Tasks

Task Search

---

# Final Principle

Task Overview exists to reduce friction between tasks, not to showcase running applications.