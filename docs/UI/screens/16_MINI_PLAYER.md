# Mini Player

Component ID

MINI_PLAYER

Category

System Component

Status

Stable

Depends On

• AUDIO_MANAGER
• PLAYBACK_ENGINE
• THEME_ENGINE
• ANIMATION_ENGINE
• IMAGE
• TEXT
• BUTTON
• PROGRESS_BAR

---

# Purpose

Mini Player provides persistent playback controls throughout FLACHEAD.

It allows users to monitor and control music without leaving their current screen.

It should never interrupt the primary task.

---

# Philosophy

Always available.

Never distracting.

One glance tells the entire story.

---

# Visibility

Visible

• Home

• Library

• Launcher

• Settings

• Search

• Queue

• Task Overview

Hidden

• Full Music Player

• Boot

• Shutdown

• Future Fullscreen Apps

---

# Position

Above Navigation Bar.

Centered horizontally.

Bottom Safe Area respected.

Never overlaps content.

---

# Dimensions

Height

72dp

Corner Radius

18dp

Horizontal Margin

12dp

Bottom Margin

8dp

---

# Layout

┌────────────────────────────┐

Album Art

Song Title

Artist

Play/Pause

Progress Bar

└────────────────────────────┘

---

# Structure

MiniPlayer

├── Album Art

├── Metadata

│ ├── Title

│ └── Artist

├── Playback Button

├── Progress Bar

└── Touch Surface

---

# Album Art

48×48dp

Rounded

12dp radius

Loaded asynchronously

Crossfade

150ms

Fallback

Default Music Icon

---

# Metadata

Primary

Song Title

Secondary

Artist

Overflow

Marquee after 2 seconds

Maximum

1 line each

---

# Playback Button

States

Play

Pause

Buffering

Loading

Disabled

Only one button.

Previous / Next remain hardware buttons.

---

# Progress Bar

Height

3dp

Always visible

Accent color

Album Palette

Buffered region

Optional

Future

---

# Dynamic Colors

Mini Player ignores wallpaper colors.

Palette comes exclusively from

Current Album Art.

Generated

Once

Cached

Reused.

---

# Playback States

Stopped

Hidden

Playing

Visible

Paused

Visible

Buffering

Visible

Loading

Skeleton

Error

Retry Indicator

---

# Touch

Tap Body

↓

Open Full Player

Tap Play

↓

Play/Pause

Long Press

↓

Queue

Double Tap

↓

Favorite (Optional)

---

# GPIO

Play Button

↓

Play

Hardware Next

↓

Next Song

Hardware Previous

↓

Previous Song

Volume Dial

↓

System Volume

---

# Keyboard

Space

↓

Play/Pause

Enter

↓

Open Full Player

---

# Encoder

Rotate

↓

Volume

Click

↓

Play/Pause

Double Click

↓

Open Player

---

# Animation

Appear

Slide Up

Disappear

Slide Down

Track Change

Crossfade

Album Art

Crossfade

Progress

Continuous

Theme

Crossfade

Animations interruptible.

---

# Rendering

Background

↓

Album Art

↓

Metadata

↓

Playback Button

↓

Progress

↓

Focus

---

# Performance Budget

Draw

<0.20ms

Animation

<0.10ms

Memory

<10MB

Album Decode

Background Thread

---

# Accessibility

48dp targets

Large Text

Supported

Reduced Motion

Supported

Contrast

Automatic

---

# Error Handling

Missing Artwork

↓

Placeholder

Unknown Artist

↓

Unknown Artist

Playback Failure

↓

Retry Button

Never crash.

---

# Acceptance Criteria

✓ Always responsive

✓ Album colors

✓ Cached artwork

✓ Zero allocations while idle

✓ Pi Zero W optimized

✓ Instant open

---

# Future

Lyrics Shortcut

Favorite Button

Playback Speed

Cast

USB DAC Status

Visualizer

Sleep Timer

Bookmark

---

# Final Principle

The Mini Player should provide everything needed for quick playback control while encouraging deeper interaction through the Full Music Player.