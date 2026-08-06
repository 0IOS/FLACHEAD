# Status Bar

Component ID

STATUS_BAR

Category

System UI

Status

Stable

Depends On

• TEXT

• ICON

• IMAGE

• THEME

• ANIMATION

• SYSTEM_MANAGER

---

# Purpose

The Status Bar displays persistent system information across FLACHEAD.

It provides glanceable information without interrupting content.

It should disappear into the background until needed.

---

# Philosophy

The Status Bar informs.

It never distracts.

Every icon must earn its place.

---

# Position

Portrait Only

Top Edge

Always visible.

Excluded during immersive fullscreen.

---

# Dimensions

Height

32dp

Top Safe Area

Included

Horizontal Padding

12dp

Vertical Padding

6dp

---

# Layout

┌──────────────────────────────────────┐

09:41        ♪ FLAC       🔋92%

└──────────────────────────────────────┘

Left

Clock

Center

Playback Status

Right

System Indicators

---

# Left Section

Clock

12 Hour

24 Hour

Configurable.

Never display seconds.

---

# Center Section

Playback

Stopped

↓

Nothing

Playing

↓

Music Note Icon

Paused

↓

Pause Icon

Streaming

↓

Network Icon

Developer Mode

↓

Developer Badge

Only one indicator at a time.

---

# Right Section

Battery

Charging

Wi-Fi

Bluetooth

Storage Warning

Developer Indicator

Notification Dot

Priority Order

Battery

↓

Charging

↓

Wi-Fi

↓

Bluetooth

↓

Notifications

↓

Developer

---

# Dynamic Theme

Wallpaper Palette

↓

Status Bar

Album Art Palette

Never used.

---

# Transparency

Default

Transparent

Blur

Disabled

Opacity

100%

Status Bar should never blur.

---

# Visibility

Home

Visible

Library

Visible

Launcher

Visible

Task Overview

Visible

Player

Visible

Fullscreen Apps

Hidden

Developer Override

Supported

---

# Battery

Percentage

Optional

Charging Animation

Supported

Critical

Red

Low

Orange

Normal

Theme color

---

# Wi-Fi

Connected

Full icon

Weak Signal

Reduced bars

Disconnected

Hidden

---

# Bluetooth

Connected

Visible

Disconnected

Hidden

Audio Device

Accent color

---

# Notifications

Small dot.

Maximum

3 indicators.

Never display long text.

---

# Developer Mode

FPS

Optional

CPU Usage

Optional

Memory

Optional

Temperature

Future

Disabled by default.

---

# Input

Status Bar ignores touch.

Long press

Future

Quick Settings

---

# Animation

Battery

Smooth fill

Wi-Fi

Fade

Bluetooth

Fade

Clock

No animation

Notifications

Scale + Fade

Theme

Crossfade

---

# Performance Budget

Draw

<0.05 ms

Memory

<4 KB

Animation

<0.02 ms

No allocations during rendering.

---

# Accessibility

Clock

Readable

Icons

Minimum 16dp

High Contrast

Supported

Reduced Motion

Supported

---

# Rendering Order

Background

↓

Clock

↓

Playback Indicator

↓

System Icons

↓

Developer Overlay

---

# Error Handling

Unknown Battery

↓

Hide

No Wi-Fi

↓

Hide

Clock Failure

↓

"--:--"

Never crash.

---

# Developer Overlay

Display

Current Theme

Battery %

RSSI

Bluetooth Device

FPS

Memory

CPU Usage

---

# Acceptance Criteria

✓ Always readable

✓ Minimal

✓ Theme aware

✓ Pi Zero W budget

✓ Zero allocations

✓ No layout shifts

---

# Future

Weather

VPN

USB DAC Status

Download Indicator

Plugin Icons

Custom Widgets

---

# Final Principle

The Status Bar should communicate system health in a single glance without stealing attention from the content.