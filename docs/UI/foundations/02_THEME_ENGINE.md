# 02_THEME_ENGINE.md

# FLACHEAD UI — Theme Engine Foundation

## 1. Purpose

The Theme Engine defines how FLACHEAD generates, manages, and applies visual styles across the operating system, music player, and applications.

FLACHEAD does not use a single fixed theme.

The visual system adapts depending on context:

* System UI → wallpaper-derived colors
* Music UI → album-art-derived colors
* Third-party apps → application-defined themes

The Theme Engine provides a unified way for all UI elements to request colors, typography, and visual tokens without implementing their own styling logic.

Primary principle:

> The interface should adapt visually while remaining consistent and readable.

---

# 2. Design goals

The Theme Engine must:

* Be lightweight enough for Raspberry Pi Zero W.
* Provide dynamic colors.
* Maintain readability.
* Avoid expensive real-time processing.
* Separate theme generation from rendering.
* Allow future custom themes.
* Support caching.

The Theme Engine should not:

* recalculate colors every frame
* load large image-processing libraries
* force every application into the same style

---

# 3. Theme architecture

The Theme Engine sits between data sources and UI rendering.

Architecture:

```text
Theme Source

 |
 |
 +----------------+
                  |
                  v

          Theme Engine

                  |
                  v

          Theme Tokens

                  |
                  v

       UI Components / Screens
```

---

# 4. Theme sources

FLACHEAD has three primary theme sources.

---

# 4.1 System Theme

Used by:

* Home
* Launcher
* Settings
* Boot
* Shutdown
* System overlays

Source:

Wallpaper image.

Pipeline:

```text
Wallpaper

↓

Color Extraction

↓

Theme Palette

↓

System UI
```

---

# 4.2 Music Theme

Used by:

* Full Music Player
* Mini Player
* Lyrics View
* Queue

Source:

Album artwork.

Pipeline:

```text
Album Artwork

↓

Color Extraction

↓

Music Palette

↓

Player UI
```

---

# 4.3 Application Theme

Used by:

* third-party apps
* future plugins

Source:

Application metadata.

Example:

```text
App Theme

Primary Color

Background

Accent

Text Colors
```

---

# 5. Theme data model

A theme should not directly store UI objects.

It stores reusable tokens.

Example:

```text
Theme

primary

secondary

background

surface

accent

text_primary

text_secondary

success

warning

error

contrast_mode
```

---

# 6. Color extraction pipeline

Color generation should happen asynchronously.

Recommended:

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

---

# 7. Image processing limitations

Because Raspberry Pi Zero W is limited:

Avoid:

* large image analysis
* continuous processing
* complex AI color models

Recommended:

Before processing:

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

---

# 8. Palette generation

Generated colors should include:

## Primary

Main accent.

Examples:

* buttons
* highlights
* progress bars

---

## Secondary

Supporting color.

Examples:

* cards
* secondary controls

---

## Background

Main UI surface.

---

## Text colors

Must include:

* primary text
* secondary text
* disabled text

---

## Status colors

Fixed semantic colors:

```text
Success

Warning

Error
```

These should not become unreadable due to dynamic themes.

---

# 9. Contrast handling

Dynamic colors must remain usable.

The Theme Engine must calculate:

* text contrast
* brightness level
* background suitability

Example:

Bad:

```text
Light yellow background

+

White text
```

Good:

```text
Light yellow background

+

Dark text
```

---

# 10. Theme application model

Components request tokens.

Example:

```text
Button

asks:

theme.accent

theme.text_primary
```

The button does not know:

* where colors came from
* wallpaper details
* album artwork

---

# 11. Theme states

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

# 12. Theme switching

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

---

# 13. Animation rules

Theme transitions must be lightweight.

Allowed:

* short fade
* color interpolation

Duration:

200-300ms

Avoid:

* rebuilding every component
* complex transitions

---

# 14. Caching strategy

Caching is required for Pi Zero W.

Cache:

* extracted palette
* image identifier
* theme tokens

Example:

```text
Theme Cache

album_hash

palette

timestamp
```

---

# 15. Cache locations

Possible:

```text
~/.flachead/cache/themes/
```

or:

```text
/data/themes/cache/
```

depending on final filesystem design.

---

# 16. Music player theme behavior

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

---

# 17. System wallpaper behavior

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

# 18. Third-party application themes

Applications may provide:

```text
theme.json
```

Example:

```json
{
 "primary":"#123456",
 "accent":"#abcdef"
}
```

The app theme should be sandboxed.

It cannot modify:

* system colors
* global UI tokens

---

# 19. Error handling

## Missing artwork

Fallback:

```text
Default Music Theme
```

---

## Invalid theme data

Fallback:

```text
System Default Theme
```

---

## Color extraction failure

Use:

* cached palette
* default palette

Never block playback.

---

# 20. Performance budget

Theme operations should not affect playback.

Targets:

Color extraction:

<1 second for normal artwork

Theme switching:

<300ms

Runtime cost:

Near zero after generation

---

# 21. Acceptance criteria

Implementation is complete when:

* system UI can use wallpaper colors
* music UI can use album colors
* applications can define themes
* themes remain readable
* palettes are cached
* theme generation happens off the UI thread
* playback is never interrupted
* Pi Zero W remains responsive

---

# 22. Future improvements

Possible additions:

* user-created themes
* theme marketplace
* animated ambient themes
* per-album theme memory
* advanced color controls
* OLED optimized palettes

---

# 23. Architectural recommendations

## Theme Manager

Create:

```text
Theme Manager

├── loadTheme()
├── generatePalette()
├── applyTheme()
├── cacheTheme()
├── validateContrast()
└── resetTheme()
```

---

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

# 24. Implementation notes for AI coding agent

When implementing:

* Keep theme generation separate from rendering.
* Never calculate colors during frame rendering.
* Cache everything possible.
* Preserve readability over aesthetics.
* Use simple color extraction algorithms.
* Keep dynamic themes optional.

The final system should feel like a premium audio device with adaptive visuals while remaining lightweight enough for Raspberry Pi Zero W.
