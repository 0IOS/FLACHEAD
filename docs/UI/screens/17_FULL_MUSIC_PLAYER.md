# Full Music Player

Component ID

FULL_MUSIC_PLAYER

Category

Primary Screen

Status

Stable

Depends On

• MINI_PLAYER
• MEDIA_SESSION_MANAGER
• PLAYBACK_ENGINE
• AUDIO_MANAGER
• LYRICS_ENGINE
• IMAGE
• TEXT
• BUTTON
• PROGRESS_BAR
• GESTURE_ENGINE
• THEME_ENGINE
• ANIMATION_ENGINE

---

# Purpose

The Full Music Player is the primary interaction surface for music playback.

It presents rich metadata, artwork, playback controls, queue access, lyrics, and audio information.

This is the flagship experience of FLACHEAD.

---

# Philosophy

Everything unnecessary has already been removed.

The music should be the focus.

The interface exists only to support the listening experience.

---

# Goals

Elegant

Fast

Readable

One-handed

Distraction-free

Responsive

Music-first

---

# Entry

Tap Mini Player

↓

Shared Element Transition

↓

Full Music Player

Back

↓

Return to Previous Screen

---

# Layout

┌────────────────────────────┐

Status Bar

Album Artwork

Song Information

Playback Progress

Playback Controls

Playback Information

Bottom Actions

Navigation Bar

└────────────────────────────┘

---

# Background

Album artwork

↓

Gaussian Blur

↓

Dark Gradient

↓

Overlay

↓

Foreground

Blur is generated once.

Cached.

Never recomputed every frame.

---

# Album Artwork

Center aligned.

Maximum Width

80% of screen.

Aspect Ratio

1:1

Rounded Corners

24dp

Shadow

Theme controlled.

Artwork fades smoothly between tracks.

---

# Song Information

Primary

Song Title

Secondary

Artist

Album

Year

Metadata automatically truncates.

Maximum

2 lines.

---

# Playback Progress

Current Position

Progress Slider

Remaining Time

Seek supported.

Updates

10Hz

Maximum.

---

# Playback Controls

Previous

Play/Pause

Next

Shuffle

Repeat

Play/Pause

Largest button.

---

# Bottom Actions

Queue

Lyrics

Audio Info

Favorite

More

Configurable.

---

# Dynamic Theme

Album Art

↓

Palette Extraction

↓

Player Theme

Wallpaper colors are never used.

---

# Audio Information

Displays

Codec

Sample Rate

Bit Depth

Bitrate

ReplayGain

Output Device

Gapless Status

Real-time updates.

---

# Playback Modes

Normal

Repeat Track

Repeat Queue

Shuffle

Gapless

Crossfade (Future)

---

# Gestures

Swipe Left

↓

Next Track

Swipe Right

↓

Previous Track

Swipe Up

↓

Lyrics

Swipe Down

↓

Queue

Double Tap Artwork

↓

Favorite

Long Press Artwork

↓

Track Information

All gestures optional.

---

# Queue

Bottom Sheet

Instant

No screen transition.

Supports

Reordering

Removing

Jump To Track

---

# Lyrics

Opens dedicated Lyrics View.

Shared transition.

Synchronized.

Offline preferred.

---

# Artwork Loading

Asynchronous

Cached

Crossfade

Placeholder

Fallback artwork supported.

---

# Theme

Album palette

↓

Entire player

Icons

Buttons

Progress

Background

Text

Generated once per track.

---

# Rendering Order

Blur Background

↓

Gradient

↓

Artwork

↓

Metadata

↓

Progress

↓

Controls

↓

Actions

↓

Navigation

---

# Animation

Open

Shared Element

Close

Reverse Shared Element

Track Change

Crossfade

Artwork

Crossfade

Controls

Morph

Theme

Crossfade

Interruptible

Yes

---

# Performance Budget

Artwork Decode

Background Thread

Frame Time

<16ms

Memory

<25MB

Blur Generation

Once Per Track

No allocations during playback.

---

# Accessibility

Large Text

Supported

High Contrast

Supported

Reduced Motion

Supported

48dp Touch Targets

Required

---

# Error Handling

Missing Artwork

↓

Placeholder

Missing Metadata

↓

Unknown

Playback Error

↓

Retry

Lyrics Missing

↓

Lyrics Unavailable

Never crash.

---

# Acceptance Criteria

✓ Pi Zero W optimized

✓ Shared transition

✓ Cached artwork

✓ Dynamic album theme

✓ Responsive controls

✓ Gapless compatible

✓ One-handed

---

# Future

Visualizer

USB DAC Controls

Equalizer

ReplayGain Editor

Comments

Bookmarks

Sleep Timer

Streaming

Plugins

---

# Final Principle

The Full Music Player should disappear, leaving only the music.