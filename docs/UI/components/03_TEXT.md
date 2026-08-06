# Text Component

Component ID

TEXT

Category

Primitive

Status

Stable

---

# Purpose

The Text component is responsible for rendering every piece of readable content inside FLACHEAD.

Text should be

Fast

Readable

Beautiful

Consistent

Theme-aware

Cached

International-ready

---

# Philosophy

Text exists to communicate.

Decoration is secondary.

Readability always wins.

---

# Responsibilities

Render text.

Wrap text.

Ellipsize text.

Measure text.

Align text.

Cache glyphs.

Support theme changes.

Support animations.

Support accessibility.

Nothing else.

---

# Widget Tree

Text

No children.

---

# Public API

SetText()

Append()

Clear()

SetFont()

SetSize()

SetWeight()

SetAlignment()

SetWrap()

SetOverflow()

SetColor()

Measure()

PreferredSize()

Baseline()

---

# Text Types

Display

Large Title

Title

Section

Body

Caption

Metadata

Lyrics

Developer

No custom categories.

---

# Font Family

Primary UI

Inter

Fallback

Noto Sans

Monospace

JetBrains Mono

Developer only.

---

# Font Weight

Regular

Medium

SemiBold

Bold

Avoid

Thin

ExtraBold

Black

---

# Font Style

Normal

Italic

Developer

Monospace

---

# Alignment

Left

Center

Right

Justify

Default

Left

Lyrics

Center

---

# Vertical Alignment

Top

Center

Bottom

Baseline

---

# Wrapping

No Wrap

Word Wrap

Character Wrap

Lyrics Wrap

Never clip unexpectedly.

---

# Overflow

Visible

Clip

Ellipsis

Fade

Marquee

Marquee only for

Song titles

Never paragraphs.

---

# Line Height

Default

1.2×

Lyrics

1.35×

Developer

1.1×

Never arbitrary.

---

# Letter Spacing

Default

0

Display

Slightly increased

Captions

Slightly reduced

---

# Paragraph Spacing

Automatic

Based on typography tokens.

Never hardcoded.

---

# Bidirectional Text

Support

LTR

RTL

Mixed

Future-ready.

---

# Unicode

UTF-8

Required.

Emoji support

Optional.

---

# Localization

Never concatenate strings.

Use placeholders.

Good

"%d songs"

Bad

"Songs: " + number

---

# Measuring

Text measurement must be cached.

Repeated layouts reuse measurements.

Never remeasure every frame.

---

# Glyph Cache

Shared globally.

Reference counted.

Eviction

LRU

Target hit rate

>95%

---

# Rendering

Subpixel positioning

Enabled

Hinting

Enabled

Kerning

Enabled

Ligatures

Optional

---

# Rendering Order

Shadow

↓

Glyph

↓

Selection

↓

Caret

↓

Debug

---

# Selection

Supports

Highlight

Caret

Future text editing.

---

# Cursor

Blink

500 ms

Developer configurable.

---

# Text Animation

Supported

Fade

Color

Slide

Scale (small)

Weight transition (future)

Never bounce.

---

# Theme Integration

Colors from

Theme.Text

Theme.Muted

Theme.Primary

Theme.Error

Never RGB.

---

# Lyrics Mode

Special rendering mode.

Current line

Largest

Brightest

Centered

Previous

Muted

Smaller

Next

Muted

Slightly brighter

Transition synchronized with playback.

---

# Marquee

Only when

Text exceeds bounds.

Delay

1 second.

Loop

Smooth.

Pause

500 ms

Never accelerate.

---

# Accessibility

Large Text

Supported

High Contrast

Supported

Reduced Motion

Supported

Future Screen Reader

Supported

---

# Performance Budget

Measure

<0.05 ms

Draw

<0.1 ms

Memory

<4 KB

Glyph Cache

Shared

---

# Memory

Never duplicate glyph atlases.

Reuse font instances.

Pool layout objects.

---

# Error Handling

Missing Font

↓

Fallback

Missing Glyph

↓

Replacement Character

Invalid UTF

↓

Ignore invalid sequence

Never crash.

---

# Developer Overlay

Display

Font

Size

Weight

Baseline

Bounds

Glyph Count

Cache Hit

Layout Time

---

# Testing

Long text

Short text

Unicode

RTL

Empty

Very large

Theme switch

Animation

Marquee

Wrapping

---

# Anti Patterns

Hardcoded colors

Manual truncation

Per-frame measuring

Multiple font instances

Bitmap fonts

Random spacing

Text inside images

---

# Acceptance Criteria

✓ Cached layout

✓ Cached glyphs

✓ Theme aware

✓ Animation aware

✓ Unicode safe

✓ Pi Zero W budget

✓ No allocations during draw

---

# Future

Variable fonts

Font fallback chains

Rich text

Markdown

Ruby text

SVG glyphs

---

# Final Principle

Text should disappear.

Users should remember what they read,

not how it was rendered.