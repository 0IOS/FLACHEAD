# App Launcher

Component ID

APP_LAUNCHER

Category

System Screen

Status

Stable

Depends On

• STATUS_BAR
• NAVIGATION_BAR
• GRID
• SEARCH_BAR
• WINDOW_MANAGER
• APP_MANAGER
• THEME_ENGINE
• WALLPAPER_ENGINE
• ANIMATION_ENGINE

---

# Purpose

The App Launcher provides fast access to every installed application.

Unlike traditional mobile launchers, the FLACHEAD Launcher is optimized for a focused music-first workflow.

It should feel lightweight, responsive, and disappear immediately after an application is launched.

---

# Philosophy

Launching an application should require as little thought as possible.

The launcher should never feel like a separate application.

It is an extension of the Home Screen.

---

# Goals

Instant

Minimal

Search-first

One-handed

Wallpaper-aware

No unnecessary animations

---

# Entry

Tap Launcher Button

↓

Launcher Opens

Tap Again

↓

Launcher Closes

Back Button

↓

Close Launcher

Launch App

↓

Launcher Closes Automatically

---

# Layout

┌──────────────────────────────┐

Status Bar

──────────────────────────────

Search Bar

──────────────────────────────

Application Grid

──────────────────────────────

Navigation Bar

└──────────────────────────────┘

---

# Background

Wallpaper remains visible.

Blur Radius

16dp

Opacity

85%

Dynamic wallpaper colors.

Album colors are never used.

---

# Search

Always visible.

Auto-focused only when keyboard exists.

Supports

Application Name

Aliases

Categories

Developer Mode Apps

Future Plugins

Future Settings Search

---

# Search Behaviour

Typing

↓

Instant Filtering

No Search

↓

Full Grid

Empty Result

↓

No Results View

Search never blocks the UI thread.

---

# Grid

Adaptive columns.

Default

3 Columns

Large displays

4 Columns

Cell Size

96dp

Spacing

Theme controlled.

Virtualized.

---

# Application Card

Contains

Icon

Application Name

Optional Badge

Optional Running Indicator

Nothing else.

---

# Running Applications

Running apps display

Small Accent Dot

Bottom Right

Never animate continuously.

---

# Recently Used

Optional section.

Maximum

6 Apps

Sorted by launch frequency and recency.

Displayed above the main grid.

---

# Categories

Future Feature.

Music

Settings

Tools

Developer

Plugins

Media

---

# Focus

Touch

Direct

GPIO

Grid Navigation

Keyboard

Directional

Encoder

Directional

Focus wraps correctly.

---

# Input

Touch

GPIO

Keyboard

Encoder

Bluetooth Remote

Future

Voice

---

# Launch

Tap

↓

Open Application

Close Launcher

↓

Animation

↓

Application Appears

No intermediate loading screen.

---

# Closing

Tap Launcher

↓

Close

Tap Outside (Optional)

↓

Close

Back

↓

Close

Home

↓

Home

---

# Theme

Wallpaper Palette

↓

Launcher

Icons keep original colors.

Cards use system surface colors.

---

# Animations

Open

Fade + Scale

Close

Fade

Launch

Zoom

Search

Crossfade

Focus

Glow

Duration

120ms

Animations interruptible.

---

# Performance Budget

Launch Delay

<40ms

Draw

<0.3ms

Animation

<0.2ms

Memory

<15MB

No allocations during idle.

---

# Accessibility

48dp Touch Targets

Large Text

High Contrast

Reduced Motion

Keyboard Navigation

Screen Reader (Future)

---

# Error Handling

Missing Icon

↓

Default Icon

Launch Failure

↓

Toast

Missing Application

↓

Hide Entry

Never crash.

---

# Developer Overlay

Applications

Visible

Running

Focused

Grid Size

Search Time

FPS

Memory

---

# Acceptance Criteria

✓ Opens instantly

✓ Search under 20ms

✓ Smooth scrolling

✓ Pi Zero W budget

✓ Wallpaper-aware

✓ One-handed

✓ Zero unnecessary UI

---

# Future

Folders

Pinned Apps

Widgets

Plugin Apps

Usage Analytics

Custom Categories

Alphabet Sidebar

---

# Final Principle

The launcher should feel like a natural extension of the Home Screen rather than a separate application.