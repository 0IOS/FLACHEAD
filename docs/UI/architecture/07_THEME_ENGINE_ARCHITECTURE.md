# 07_THEME_ENGINE_ARCHITECTURE.md

# FLACHEAD UI — Theme Engine Architecture Specification

## 1. Purpose

This document defines the architecture of the FLACHEAD theme system.

FLACHEAD does not use a single static theme. The interface adapts depending on context:

* system screens use wallpaper-derived colors
* music screens use album-art-derived colors
* third-party applications use their own themes

The theme engine provides a unified system for generating, applying, and managing visual styles.

It provides a unified way for all UI elements to request colors, typography, and visual tokens without implementing their own styling logic.

Primary principle:

> Themes should enhance the experience without consuming resources needed for playback.

> The interface should adapt visually while remaining consistent and readable.

---

# 2. Design goals

The theme system must be:

* lightweight enough for Raspberry Pi Zero W
* provide dynamic colors
* maintain readability
* avoid expensive real-time processing
* separate theme generation from rendering
* allow future custom themes
* support caching

The theme system should not:

* recalculate colors every frame
* load large image-processing libraries
* force every application into the same style

---

# 3. Theme architecture

Data flow:

```text
Theme Source

↓

Theme Engine

↓

Color Processing

↓

Theme Cache

↓

UI Components
```

---

# 4. Theme sources

FLACHEAD supports:

```text
1. System Theme

2. Music Theme

3. Application Theme

4. User Theme
```

---

# 5. System theme

Used by:

* home screen
* launcher
* settings
* task overview
* boot
* shutdown
* system overlays

Source:

```text
Wallpaper

↓

Color Extraction

↓

System Palette
```

Generated values:

```text
Primary Color

Secondary Color

Accent Color

Background Tone

Text Contrast
```

---

# 6. Music theme

Used by:

* full music player
* mini player
* lyrics view
* queue

Source:

```text
Album Artwork

↓

Color Extraction

↓

Music Palette
```

Example:

Album:

Blue artwork

↓

UI:

Blue accent controls

---

# 7. Application themes

Third-party applications are isolated.

Example:

```text
Calculator App

↓

Own Theme

↓

FLACHEAD Hosts It
```

Applications may provide a `theme.json` file:

```json
{
  "primary":"#123456",
  "accent":"#abcdef"
}
```

The app theme should be sandboxed. It cannot modify:

* system colors
* global UI tokens

Applications should not modify system themes.

---

# 8. Theme data model

A theme should not directly store UI objects. It stores reusable tokens.

```text
Theme

primary
secondary
background
surface
accent
text_primary
text_secondary
text_disabled
success
warning
error
contrast_mode
```

Components consume semantic colors.

Example:

Good:

```text
Button Accent
```

Bad:

```text
Use RGB(40,90,120)
```

---

# 9. Theme engine components

Required:

```text
Theme Engine

├── Palette Generator

├── Color Extractor

├── Contrast Manager

├── Theme Cache

└── Theme Provider
```

---

# 10. Color extraction

Color generation should happen asynchronously.

Pipeline:

```text
Image Input

↓

Resize Image

↓

Extract Dominant Colors

↓

Generate Palette

↓

Calculate Contrast

↓

Store Cache
```

Because Raspberry Pi Zero W is limited, before processing:

```text
Original Image

↓

Resize to small thumbnail

↓

Analyze
```

Example:

```text
3000x3000 image

becomes

64x64 thumbnail
```

Avoid expensive algorithms.

---

# 11. Palette generation

Generated palette:

```text
Background

Surface

Primary

Accent

Text

Muted
```

Generated colors should include:

## Primary

Main accent. Examples: buttons, highlights, progress bars.

## Secondary

Supporting color. Examples: cards, secondary controls.

## Background

Main UI surface.

## Text colors

Must include: primary text, secondary text, disabled text.

## Status colors

Fixed semantic colors: Success, Warning, Error.

These should not become unreadable due to dynamic themes.

---

# 12. Contrast management

Dynamic colors must remain usable.

The Theme Engine must calculate:

* text contrast
* brightness level
* background suitability

Small TFT displays require careful contrast.

Example:

Bad:

```text
Light yellow background + White text
```

Good:

```text
Light yellow background + Dark text
```

Low contrast: adjust automatically.

---

# 13. Theme application model

Components request tokens. The component does not know:

* where colors came from
* wallpaper details
* album artwork

Example:

```text
Button

asks:

theme.accent

theme.text_primary
```

---

# 14. Theme states

The engine supports:

```text
Theme State

current_theme

previous_theme

source

generation_status

cache_status
```

---

# 15. Theme switching

Theme changes should be gradual but simple.

Flow:

```text
New Theme Requested

↓

Generate/Load Palette

↓

Validate Contrast

↓

Apply Tokens

↓

Refresh UI
```

When theme changes, allowed transitions:

* fade
* gradual color interpolation

Duration:

100-300ms

Avoid:

* heavy transitions
* rebuilding every component
* complex transitions

---

# 16. Theme caching

Theme generation should not happen repeatedly.

Cache:

* extracted palette
* image identifier
* theme tokens

```text
Theme Cache

album_hash

palette

timestamp
```

Cache locations:

```text
~/.flachead/cache/themes/
```

or:

```text
/data/themes/cache/
```

depending on final filesystem design.

Reuse when returning to tracks.

---

# 17. Music player theme behavior

When a new track starts:

Flow:

```text
Track Changed Event

↓

Check Album Cache

↓

Load Existing Palette

OR

Generate New Palette

↓

Update Player Theme
```

Generated once per track.

---

# 18. System wallpaper behavior

When wallpaper changes:

Flow:

```text
Wallpaper Changed

↓

Generate System Palette

↓

Update System Theme
```

---

# 19. User customization

Possible options:

```text
Theme Mode

Accent Preference

Brightness

Wallpaper Choice
```

Customization should override generated themes safely.

---

# 20. Theme priority

Priority order:

```text
Temporary Music Theme

↓

Application Theme

↓

User Theme

↓

System Theme
```

Context decides priority.

---

# 21. Performance requirements

Theme generation must be:

* asynchronous
* cached
* low CPU

Never generate colors during frame rendering.

Specific targets:

Color extraction:

<1 second for normal artwork

Theme switching:

<300ms

Runtime cost:

Near zero after generation

---

# 22. Error handling

## Missing artwork

Fallback:

```text
Default Music Theme
```

## Invalid theme data

Fallback:

```text
System Default Theme
```

## Color extraction failure

Use:

* cached palette
* default palette

Never block playback.

---

# 23. Acceptance criteria

Implementation is complete when:

* system UI can use wallpaper colors
* music UI can use album colors
* applications can define themes
* themes remain readable
* palettes are cached
* theme generation happens off the UI thread
* playback is never interrupted
* Pi Zero W remains responsive
* applications remain isolated

---

# 24. Future improvements

Possible additions:

* user-created themes
* theme marketplace
* animated ambient themes
* per-album theme memory
* advanced color controls
* OLED optimized palettes

---

# 25. Architectural recommendations

## Theme Manager

```text
Theme Manager

├── loadTheme()
├── generatePalette()
├── applyTheme()
├── cacheTheme()
├── validateContrast()
└── resetTheme()
```

## Event integration

Theme events:

```text
WallpaperChanged

AlbumChanged

ThemeChanged
```

Consumed by:

* UI Renderer
* Components
* Screens

---

# 26. Implementation notes for AI coding agent

When implementing:

* Keep theme generation separate from rendering.
* Never calculate colors during frame rendering.
* Cache everything possible.
* Preserve readability over aesthetics.
* Use simple color extraction algorithms.
* Keep dynamic themes optional.
* Use semantic colors, never raw RGB values.
* Test readability on the actual 2.8" TFT.

The FLACHEAD theme system should make the device feel premium while remaining efficient enough for Raspberry Pi Zero W.
