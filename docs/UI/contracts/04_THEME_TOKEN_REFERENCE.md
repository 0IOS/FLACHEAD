# 04_THEME_TOKEN_REFERENCE.md

# FLACHEAD UI — Theme Token Reference

## 1. Purpose

This document defines the standardized visual tokens used throughout FLACHEAD.

Theme tokens provide a common visual language between:

* components
* screens
* overlays
* applications

Components must never hardcode visual values.

Instead of:

```cpp
color = "#FFFFFF";
```

Use:

```cpp
theme.text.primary;
```

Primary principle:

> Components understand design tokens, not individual themes.

---

# 2. Theme token architecture

Theme data flow:

```text id="n5q8m2"
Theme Source

(Wallpaper / Album Art / App Theme)

        |

        v

Theme Engine

        |

        v

Theme Tokens

        |

        v

Components + Screens
```

---

# 3. Token categories

FLACHEAD tokens are divided into:

```text id="m7x3p9"
Color Tokens

Typography Tokens

Spacing Tokens

Shape Tokens

Animation Tokens

Component Tokens
```

---

# 4. Color system

Colors are semantic.

Do not name colors by appearance.

Bad:

```text id="x4p8m6"
dark_blue
light_gray
```

Good:

```text id="q9m2x5"
background

surface

accent

text_primary
```

---

# 5. Background colors

## background

Main application background.

Used by:

* screens
* system pages

---

## surface

Raised areas.

Used by:

* cards
* panels
* dialogs

---

## surface_variant

Secondary surfaces.

Used by:

* lists
* menus

---

# 6. Accent colors

## primary

Main theme color.

Used for:

* selected items
* progress
* active controls

---

## secondary

Supporting highlight.

Used for:

* secondary actions
* decorative elements

---

## accent

Strong emphasis.

Used sparingly:

* important buttons
* active states

---

# 7. Text colors

Required:

```text id="p5m8x3"
text_primary

text_secondary

text_disabled
```

---

## text_primary

Used for:

* titles
* important information

---

## text_secondary

Used for:

* metadata
* descriptions

---

## text_disabled

Used for:

* unavailable controls

---

# 8. Status colors

These are semantic and should remain readable.

```text id="r3x7m9"
success

warning

error

info
```

Examples:

Success:

* completed scan

Warning:

* low storage

Error:

* playback failure

---

# 9. Contrast rules

Theme Engine must validate:

```text id="v8m2q6"
Text

against

Background
```

---

Minimum requirements:

* readable titles
* readable metadata
* visible controls

---

If contrast fails:

Use:

* adjusted color
* fallback token

---

# 10. Typography system

Typography must be limited.

Recommended sizes:

```text id="k7m4x2"
Large Title

32px


Title

24px


Subtitle

18px


Body

16px


Small

12px
```

---

Exact values depend on final TFT resolution.

---

# 11. Font rules

Requirements:

* one primary font family
* limited weights
* cached rendering

Avoid:

* many font files
* unnecessary styles

---

Recommended:

```text id="c5m9x1"
Regular

Medium

Bold
```

---

# 12. Text behavior

All text components support:

* truncation
* alignment
* wrapping where needed

---

Example:

Long song title:

```text id="h8x2m5"
Very Long Song Na...
```

---

Avoid:

* uncontrolled overflow

---

# 13. Spacing system

Use consistent spacing units.

Base:

```text id="q4m7x9"
4px
```

Scale:

```text id="s8x3m6"
4

8

12

16

24

32
```

---

Examples:

Small gap:

4px

Card padding:

12-16px

Screen margins:

16px

---

# 14. Layout tokens

Common values:

```text id="m2x7p5"
screen_padding

component_gap

card_padding

list_spacing
```

---

Screens should use tokens.

---

# 15. Shape tokens

Define:

## Corner radius

```text id="n8m4x2"
small

medium

large
```

---

Suggested:

Small:

4px

Medium:

8px

Large:

16px

---

# 16. Border tokens

Used for:

* cards
* separators
* focus states

Tokens:

```text id="v6p2m8"
border_color

border_width

divider_color
```

---

Avoid excessive borders.

---

# 17. Shadow and elevation

Because Pi Zero W is limited:

Use minimal shadows.

Preferred:

* flat design
* color contrast
* small elevation cues

Avoid:

* expensive blur shadows

---

Tokens:

```text id="x7m3q9"
elevation_none

elevation_small
```

---

# 18. Animation tokens

Animations use shared timing.

## Duration

```text id="p8x5m2"
instant

fast

normal

slow
```

---

Recommended:

Instant:

0ms

Fast:

100ms

Normal:

200ms

Slow:

300ms

---

# 19. Easing tokens

Supported:

```text id="m5q8x1"
linear

ease_in

ease_out

ease_in_out
```

---

Avoid complex physics.

---

# 20. Component styling tokens

Components access:

```text id="w9m4x7"
button.primary

button.secondary

card.default

slider.default
```

---

Example:

Button:

```text id="f3x8m6"
background

text

radius

padding
```

---

# 21. Music player theme rules

Music screens use:

```text id="r7m2x5"
Album Theme

+

Global readability rules
```

---

Album colors may modify:

* accents
* backgrounds
* highlights

Never override:

* error colors
* accessibility rules

---

# 22. System theme rules

System UI uses:

```text id="k6m9x3"
Wallpaper Theme
```

Applies to:

* Home
* Launcher
* Settings

---

# 23. Third-party app tokens

Apps receive:

```text id="a4m8x2"
App Theme Context
```

They cannot modify:

* global tokens
* system colors

---

# 24. Runtime theme changes

Flow:

```text id="z3m7x9"
Theme Changed

↓

Update Token Set

↓

Notify Components

↓

Redraw
```

---

# 25. Performance rules

Theme tokens must be:

* cached
* immutable during rendering
* lightweight

Avoid:

```text id="p6x2m8"
Calculating colors

inside render()
```

---

# 26. Error handling

Missing theme:

Fallback:

```text id="h9m4x7"
Default FLACHEAD Theme
```

---

Invalid token:

Use:

```text id="q2m8x5"
Safe Default Value
```

---

# 27. Acceptance criteria

Implementation is complete when:

* all components use tokens
* themes can change dynamically
* colors remain readable
* no hardcoded styling exists
* tokens are cached
* rendering remains fast

---

# 28. Architectural recommendations

## Theme Token Object

```cpp id="y5m8x2"
ThemeTokens

├── colors

├── typography

├── spacing

├── shapes

├── animations

└── components
```

---

## Token Manager

```text id="b8m3q6"
Token Manager

├── load()

├── update()

├── validate()

├── apply()

└── fallback()
```

---

# 29. Implementation notes for AI coding agent

When implementing:

* Never hardcode colors.
* Never hardcode spacing.
* Use semantic token names.
* Keep themes separate from components.
* Validate contrast.
* Cache generated values.
* Optimize for Pi Zero W.

The final theme system should allow FLACHEAD to feel personal and premium while maintaining a consistent embedded-device identity.
