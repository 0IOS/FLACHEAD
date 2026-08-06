# Queue

Component ID

QUEUE

Category

Music Screen

Status

Stable

Depends On

• PLAYBACK_ENGINE
• MEDIA_SESSION_MANAGER
• LIST
• LIST_ITEM
• MINI_PLAYER
• SEARCH_BAR
• BUTTON
• ANIMATION_ENGINE

---

# Purpose

Queue displays the current playback order.

Users can inspect, reorder, remove, and modify upcoming playback without interrupting music.

Queue is transient.

It is independent of the Music Library.

---

# Philosophy

The queue represents intent.

Changing it should be effortless.

Playback should never stop because the queue is being edited.

---

# Entry

Mini Player

↓

Queue Button

↓

Bottom Sheet

Full Player

↓

Queue Button

↓

Bottom Sheet

Gesture

↓

Swipe Down

(Optional)

---

# Exit

Swipe Down

Back

Tap Outside

Done Button

Playback never pauses.

---

# Layout

┌────────────────────────────┐

Now Playing

────────────────────────────

Next Up

────────────────────────────

Later

────────────────────────────

Mini Player (optional)

────────────────────────────

Navigation Bar

└────────────────────────────┘

---

# Sections

Now Playing

Exactly one item.

Next Up

Immediate upcoming songs.

Later

Remaining queue.

History

Optional.

Previous songs.

---

# Queue Item

Album Art

Title

Artist

Duration

Quality Badge

Playing Indicator

Drag Handle

---

# Playing Item

Accent Border

Animated Equalizer

Progress Indicator

Cannot be reordered.

---

# Drag & Drop

Supported.

Long Press

↓

Drag

↓

Drop

Queue updates instantly.

Playback continues.

---

# Remove Song

Swipe Left

↓

Remove

Undo Toast

5 seconds

---

# Add Song

Library

↓

Add to Queue

↓

Appended

or

Play Next

↓

Inserted after current track

---

# Multi Selection

Supported.

Actions

Remove

Move

Favorite

Playlist

Collection

Share (Future)

---

# Search

Optional.

Search only within the queue.

Never searches library.

---

# Queue Modes

Normal

Repeat Queue

Repeat Track

Shuffle

Party Shuffle (Future)

Visible at top.

---

# Queue Information

Songs Remaining

Remaining Duration

Estimated Finish Time

Current Output Device

---

# Queue Persistence

Configurable.

Resume after reboot

Supported.

Clear on exit

Optional.

---

# Theme

Album colors.

Inherited from current playing track.

---

# Rendering

Background

↓

Queue

↓

Playing Item

↓

Mini Player

↓

Navigation

---

# Animation

Insert

Slide

Remove

Collapse

Drag

Lift

Drop

Spring

Selection

Fade

Interruptible

Yes

---

# Performance Budget

10,000 Queue Items

Supported

Frame Time

<16ms

Memory

<15MB

Zero allocations while scrolling.

---

# Accessibility

Large Text

Supported

Reduced Motion

Supported

Touch Targets

48dp

Keyboard

Supported

Encoder

Supported

---

# Error Handling

Missing Song

↓

Unavailable

Missing Metadata

↓

Unknown

Queue Corruption

↓

Rebuild

Playback Error

↓

Skip

Never crash.

---

# Acceptance Criteria

✓ Drag reorder

✓ Instant update

✓ Undo removal

✓ Resume playback

✓ Pi Zero optimized

✓ Zero allocations while idle

---

# Future

Queue Folders

Smart Queue

AI Queue

Cross-device Queue

Temporary Tags

Collaborative Queue

---

# Final Principle

The Queue should feel editable at all times without making playback feel fragile.