# Music Library

Component ID

MUSIC_LIBRARY

Category

Primary Screen

Status

Stable

Depends On

• STATUS_BAR
• NAVIGATION_BAR
• SEARCH_BAR
• LIST
• LIST_ITEM
• FILTER_BAR
• MINI_PLAYER
• LIBRARY_ENGINE
• IMAGE_CACHE
• THEME_ENGINE

---

# Purpose

The Music Library provides fast, intuitive browsing of every locally available audio file.

It should feel like browsing a physical music collection rather than searching a filesystem.

---

# Philosophy

Music comes first.

Folders come second.

The user should browse by feeling, not by path.

---

# Goals

Instant

Organized

Beautiful

Offline

Fast

Scalable

---

# Entry

Launcher

↓

Music Library

Mini Player

↓

Library

Back

↓

Previous Screen

---

# Layout

┌────────────────────────────┐

Status Bar

────────────────────────────

Search

Filter Chips

────────────────────────────

Library Content

────────────────────────────

Mini Player

────────────────────────────

Navigation Bar

└────────────────────────────┘

---

# Library Tabs

Songs

Albums

Artists

Playlists

Genres

Folders

Recently Added

Favorites

Each tab remembers its own

Scroll Position

Selection

Search

Sort

---

# Default Tab

Songs

---

# Search

Always visible.

Instant filtering.

Never blocks UI.

Searches

Title

Artist

Album

Genre

Filename

Composer

Future

Lyrics

Comments

---

# Filter Chips

Downloaded

Hi-Res

FLAC

DSD

Recently Added

Favorites

Lossless

Explicit

Future Plugins

Multiple filters allowed.

---

# Sorting

Title

Artist

Album

Duration

Track Number

Date Added

File Size

Bitrate

Sample Rate

Last Played

Play Count

Ascending

Descending

---

# Song List Item

Album Art

↓

Title

↓

Artist

↓

Album

↓

Duration

↓

Hi-Res Badge

Running Track

↓

Playing Indicator

---

# Album View

Large Album Art

Album Name

Artist

Track Count

Year

Duration

---

# Artist View

Artist Image

(Optional)

Artist Name

Albums

Songs

Duration

---

# Playlist View

Cover

Playlist Name

Track Count

Duration

---

# Folder View

Folder Icon

Folder Name

Song Count

Filesystem Path

Only view exposing folders.

---

# Recently Added

Sorted automatically.

Newest first.

---

# Favorites

Pinned.

Offline.

Instant.

---

# Dynamic Theme

Wallpaper colors.

Never album colors.

Album colors belong exclusively to playback.

---

# Artwork

Loaded asynchronously.

Cached.

Never decoded on UI thread.

Placeholder supported.

---

# Input

Touch

GPIO

Keyboard

Encoder

Remote

Identical navigation.

---

# Touch

Tap

↓

Open

Long Press

↓

Context Menu

Swipe

↓

Future

Selection

---

# Context Menu

Play

Play Next

Add To Queue

Favorite

Playlist

Track Info

Share (Future)

Delete

Developer

---

# Multi Selection

Supported.

Actions

Queue

Playlist

Delete

Favorite

Export

---

# Playback

Tap Song

↓

Playback Starts

Mini Player Updates

Current Song Highlighted

Automatically

---

# Rendering

Status Bar

↓

Search

↓

Filters

↓

List

↓

Mini Player

↓

Navigation

---

# Animation

Search

Crossfade

Filters

Slide

Selection

Accent

Artwork

Fade

Scrolling

GPU Accelerated

Interruptible

Yes

---

# Performance Budget

100,000 Songs

Supported

Frame Time

<16ms

Memory

<40MB

Artwork Decode

Background Thread

Zero allocations while scrolling.

---

# Accessibility

Large Text

Supported

Reduced Motion

Supported

High Contrast

Supported

Touch Target

48dp

---

# Error Handling

Broken Metadata

↓

Unknown

Missing Artwork

↓

Placeholder

Missing File

↓

Unavailable Badge

Playback Failure

↓

Toast

Never crash.

---

# Acceptance Criteria

✓ Instant search

✓ Virtualized

✓ Cached artwork

✓ Offline

✓ Pi Zero W optimized

✓ Smooth scrolling

✓ Shared playback state

---

# Future

Smart Playlists

Mood

Tags

Composer

Conductor

Disc View

Box Sets

Ratings

Streaming

Plugins

---

# Final Principle

The Music Library should make finding music feel effortless regardless of collection size.