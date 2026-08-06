# 07_THEME_ENGINE_ARCHITECTURE.md

# FLACHEAD UI — Theme Engine Architecture Specification

## 1. Purpose

This document defines the architecture of the FLACHEAD theme system.

FLACHEAD does not use a single static theme. The interface adapts depending on context:

* system screens use wallpaper-derived colors
* music screens use album-art-derived colors
* third-party applications use their own themes

The theme engine provides a unified system for generating, applying, and managing visual styles.

Primary principle:

> Themes should enhance the experience without consuming resources needed for playback.

---

# 2. Design goals

The theme system must be:

* lightweight
* consistent
* cached
* customizable
* hardware-friendly

---

# 3. Theme architecture

Data flow:

```text id="x7m4q9"
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

```text id="m5x8q3"
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

---

Source:

```text id="v8m3x5"
Wallpaper

↓

Color Extraction

↓

System Palette
```

---

Generated values:

```text id="c6m4x9"
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

---

Source:

```text id="r7m3x8"
Album Artwork

↓

Color Extraction

↓

Music Palette
```

---

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

```text id="n8m4x6"
Calculator App

↓

Own Theme

↓

FLACHEAD Hosts It
```

---

Applications should not modify system themes.

---

# 8. Theme engine components

Required:

```text id="w5m3x7"
Theme Engine

├── Palette Generator

├── Color Extractor

├── Contrast Manager

├── Theme Cache

└── Theme Provider
```

---

# 9. Color extraction

Artwork processing:

```text id="a7m4x8"
Image

↓

Sample Pixels

↓

Remove Extreme Colors

↓

Generate Palette
```

---

Avoid expensive algorithms.

---

# 10. Palette generation

Generated palette:

```text id="p6m8x2"
Background

Surface

Primary

Accent

Text

Muted
```

---

Components consume semantic colors.

---

Example:

Good:

```text id="u5m3x9"
Button Accent
```

---

Bad:

```text id="k8m4x5"
Use RGB(40,90,120)
```

---

# 11. Theme caching

Theme generation should not happen repeatedly.

Cache:

```text id="d7m3x8"
Album ID

↓

Generated Palette
```

---

Reuse when returning to tracks.

---

# 12. Theme transitions

When theme changes:

Allowed:

* fade
* gradual color interpolation

Duration:

```text id="h5m8x2"
100-300ms
```

---

Avoid:

Heavy transitions.

---

# 13. Contrast management

Small TFT displays require careful contrast.

Theme engine must ensure:

* readable text
* visible controls
* accessible buttons

---

Example:

Dark background:

↓

Light text

---

# 14. User customization

Possible options:

```text id="s6m4x9"
Theme Mode

Accent Preference

Brightness

Wallpaper Choice
```

---

Customization should override generated themes safely.

---

# 15. Theme priority

Priority order:

```text id="b8m3x5"
Temporary Music Theme

↓

Application Theme

↓

User Theme

↓

System Theme
```

---

Context decides priority.

---

# 16. Performance requirements

Theme generation:

Must be:

* asynchronous
* cached
* low CPU

---

Never:

Generate colors during frame rendering.

---

# 17. Error handling

Invalid artwork:

Fallback:

```text id="x7m4q2"
Default Music Theme
```

---

Low contrast:

Adjust automatically.

---

# 18. Acceptance criteria

Implementation is complete when:

* themes apply correctly
* colors are cached
* text remains readable
* theme changes do not affect playback
* applications remain isolated

---

# 19. Architectural recommendations

Required modules:

```text id="d8m4x6"
Theme Manager

├── Theme Provider

├── Palette Generator

├── Cache

├── Contrast Checker

└── Theme Applier
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep themes separate from components.
* Use semantic colors.
* Cache generated palettes.
* Never run heavy image processing in the render loop.
* Test readability on the actual 2.8" TFT.

The FLACHEAD theme system should make the device feel premium while remaining efficient enough for Raspberry Pi Zero W.
