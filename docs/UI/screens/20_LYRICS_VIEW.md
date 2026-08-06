# Lyrics View

Component ID

LYRICS_VIEW

Category

Music Screen

Status

Stable

Depends On

• MEDIA_SESSION_MANAGER
• LYRICS_ENGINE
• PLAYBACK_ENGINE
• THEME_ENGINE
• ANIMATION_ENGINE
• GESTURE_ENGINE
• TEXT
• BUTTON

---

# Purpose

Lyrics View presents synchronized lyrics in a clean, immersive environment.

It should help users connect with the music without distracting from it.

Lyrics are a first-class feature of FLACHEAD.

---

# Philosophy

The current lyric is the focus.

Everything else fades into the background.

The music should lead the eyes.

---

# Goals

Beautiful

Readable

Smooth

Offline First

Perfect Synchronization

One-Handed

---

# Entry

Full Music Player

↓

Lyrics Button

↓

Shared Transition

↓

Lyrics View

Swipe Up

(Optional)

↓

Lyrics View

---

# Exit

Back

↓

Full Music Player

Swipe Down

↓

Player

Home

↓

Home

Playback continues.

---

# Layout

┌────────────────────────────┐

Status Bar

────────────────────────────

Current Lyrics

────────────────────────────

Previous Lines

Next Lines

────────────────────────────

Progress Indicator

Navigation Bar

└────────────────────────────┘

---

# Background

Current Album Artwork

↓

Heavy Blur

↓

Dark Gradient

↓

Lyrics

Background generated once.

Cached.

---

# Current Line

Largest text.

Centered.

Accent color.

Maximum

2 lines.

---

# Previous Lines

Smaller.

Above.

Lower opacity.

---

# Upcoming Lines

Smaller.

Below.

Lower opacity.

---

# Scrolling

Automatic.

Smooth.

Never jumps.

Lyrics always remain centered.

---

# Synchronization

Supported Formats

LRC

Enhanced LRC

Embedded Tags

Future

TTML

Word-Level Timing

---

# Playback Sync

Lyrics update from Media Session.

Maximum update rate

30 FPS

Interpolation supported.

---

# Manual Scroll

Allowed.

Automatic sync pauses.

Resume button appears.

Tap Resume

↓

Jump to current lyric.

---

# Lyrics Availability

Embedded

Preferred

External

Second

Downloaded

Future

Missing

Display Empty State

---

# Empty State

Album Artwork

↓

"No lyrics available."

↓

Optional Search

Playback unaffected.

---

# Search

Search within current lyrics.

Highlights all matches.

Jump directly to line.

---

# Dynamic Theme

Album Palette

↓

Lyrics View

Contrast verified.

Accessibility maintained.

---

# Text Rendering

GPU Cached

Subpixel Positioning

No re-layout during playback.

---

# Animations

Current Line

Scale

Previous

Fade

Upcoming

Fade

Track Change

Crossfade

Theme

Crossfade

---

# Gestures

Swipe Down

↓

Player

Tap Line

↓

Seek Playback

Long Press

↓

Copy Lyrics (if permitted)

Double Tap

↓

Favorite Line (Future)

---

# Performance Budget

Frame Time

<16ms

Memory

<12MB

Text Layout

Cached

No allocations during scrolling.

---

# Accessibility

Large Text

Supported

Reduced Motion

Supported

High Contrast

Supported

Dynamic Scaling

Supported

---

# Error Handling

Lyrics Missing

↓

Empty State

Sync Failure

↓

Manual Mode

Malformed LRC

↓

Ignore Invalid Entries

Never crash.

---

# Acceptance Criteria

✓ Perfect sync

✓ Smooth scrolling

✓ Pi Zero W optimized

✓ Offline first

✓ Cached rendering

✓ Album theme

---

# Future

Translations

Dual Lyrics

Romanization

Word Highlighting

Karaoke Mode

Notes

Bookmarks

Annotations

Plugin Providers

---

# Final Principle

Lyrics should feel like part of the music rather than subtitles on a screen.