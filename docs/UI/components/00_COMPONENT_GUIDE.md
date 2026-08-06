# FLUID Component Guide

Version: 1.0

Status: Stable

Depends On

• FLUID Design System
• Layout System
• Theme System
• Animation System
• Input System

---

# Purpose

This document defines how every UI component inside FLACHEAD is designed, implemented, tested, and maintained.

Every widget in the system follows these rules.

No exceptions.

This document is intentionally implementation-focused.

---

# Component Philosophy

A component should solve exactly one problem.

Examples

✓ Button

✓ Slider

✓ Progress Bar

✓ Album Art

Examples of things that are NOT components

✗ Home Screen

✗ Music Player

✗ Settings

Those are compositions.

---

# Design Goals

Every component should be

Reusable

Composable

Theme-aware

Animation-aware

Input-aware

Accessible

Cheap to render

Easy to test

Easy to maintain

---

# Component Lifecycle

Every component follows exactly the same lifecycle.

Construct

↓

Initialize

↓

Measure

↓

Layout

↓

Theme

↓

Input Ready

↓

Paint

↓

Animate

↓

Destroy

---

# Component Responsibilities

A component owns

Rendering

Animation

Input

Internal Padding

Accessibility

Local State

Local Cache

A component does NOT own

Navigation

Global State

External Margins

Theme Generation

Wallpaper

Audio

Filesystem

---

# Rendering Pipeline

Every frame

Measure

↓

Layout

↓

Cull

↓

Paint

↓

Composite

↓

Present

Components never skip stages.

---

# Measure()

Purpose

Determine ideal size.

Returns

Minimum Size

Preferred Size

Maximum Size

Aspect Ratio

Baseline

Measure must never allocate memory.

---

# Layout()

Purpose

Receive final geometry.

Inputs

Position

Bounds

Constraints

Output

Cached geometry.

Layout must not render.

---

# Paint()

Purpose

Draw using cached geometry.

Paint must never

Measure

Allocate

Change layout

Query filesystem

Decode images

Load fonts

---

# Input()

Purpose

Consume Actions.

Not hardware.

Touch

GPIO

Keyboard

Encoder

all become

Action objects.

---

# Theme()

Receive

Current Theme

↓

Resolve Tokens

↓

Cache Colors

↓

Request repaint

Never generate colors.

---

# Animation()

Each component owns

Appear

Disappear

Focus

Pressed

Disabled

Loading

Error

Never invent custom timelines.

---

# Visibility

Visible

Hidden

Collapsed

Hidden

Occupies layout.

Collapsed

Removed from layout.

---

# Enabled State

Enabled

Disabled

Disabled components

Ignore input.

Remain visible.

Remain measurable.

---

# Focus

Only one component owns focus.

Components expose

CanFocus()

SetFocus()

LoseFocus()

HasFocus()

---

# Selection

Selection is separate from Focus.

Focused ≠ Selected

Examples

List Item

Focused

Selected

Possible.

---

# State Machine

Idle

↓

Focused

↓

Pressed

↓

Released

↓

Focused

↓

Idle

Additional branches

Loading

Disabled

Error

Hidden

---

# Geometry

Each component stores

Bounds

Content Bounds

Padding

Clip Rect

Hit Rect

Dirty Rect

Never recalculate every frame.

---

# Hit Testing

Touch

↓

Hit Rect

↓

Visible?

↓

Enabled?

↓

Accept Input

Invisible widgets never consume events.

---

# Dirty Regions

Every component tracks

Needs Layout

Needs Paint

Needs Animation

Needs Theme

Only dirty components update.

---

# Caching

Cache whenever possible.

Examples

Text Layout

Glyph Runs

Textures

Gradients

Blur

Rounded Rect Paths

Never cache

Input

Animation Progress

Focus

---

# Memory Ownership

Each component owns

Its local cache.

Nothing else.

Shared resources use

Reference Counting.

---

# Thread Safety

Components exist on UI thread.

Heavy work

Image Decode

Palette Extraction

Filesystem

runs elsewhere.

---

# Draw Order

Background

↓

Shadow

↓

Surface

↓

Border

↓

Content

↓

Overlay

↓

Focus Ring

↓

Debug

Always identical.

---

# Theme Contract

Components request

Theme.Surface

Theme.Text

Theme.Primary

Theme.Focus

Theme.Error

Never RGB values.

---

# Layout Contract

Components never choose their own position.

Parents decide placement.

Components decide content layout only.

---

# Animation Contract

Components expose

StartAnimation()

StopAnimation()

Interrupt()

Reverse()

Reset()

Animation ownership belongs to AnimationManager.

---

# Accessibility Contract

Every component provides

Accessible Name

Description

Role

State

Focus Order

Touch Bounds

Future Screen Reader Text

---

# Performance Budget

Maximum Measure

0.05 ms

Maximum Layout

0.05 ms

Maximum Paint

0.15 ms

Target Total

0.25 ms

Large widgets receive individual budgets.

---

# Memory Budget

Simple Widgets

<2 KB

Composite Widgets

<8 KB

Large Widgets

Case-by-case

---

# Texture Budget

Widgets never own duplicate textures.

Everything shared.

---

# Draw Calls

Goal

Batch whenever possible.

Avoid state changes.

Avoid texture swaps.

---

# Component IDs

Every component receives

Stable ID

Persistent across frames.

Never use pointer addresses.

---

# Naming

Use nouns.

Good

Button

Slider

List

AlbumArt

Bad

ButtonThing

SliderNew

Widget123

---

# Composition

Large widgets should compose smaller widgets.

Example

Mini Player

Album Art

↓

Text

↓

Progress

↓

Buttons

↓

Container

Never duplicate functionality.

---

# Error Handling

Components fail gracefully.

Missing icon

↓

Placeholder

Missing image

↓

Fallback artwork

Missing font

↓

Default font

Never crash.

---

# Testing

Each component must pass

Construction

Layout

Rendering

Input

Animation

Theme

Accessibility

Performance

Memory

before shipping.

---

# Debug Overlay

Developer mode should display

Component ID

Bounds

Dirty State

FPS Cost

Memory

Paint Time

Layout Time

Focus

Theme Tokens

Animation State

---

# Anti-Patterns

❌ Widgets owning global state

❌ Paint allocating memory

❌ Layout inside Paint

❌ Hardcoded colors

❌ Hardcoded spacing

❌ Global singletons inside widgets

❌ Widget-to-widget communication

❌ Circular ownership

---

# Component Checklist

Every new widget must answer

Why does it exist?

Can another widget do this?

Can it be composed?

Can it be themed?

Can it animate?

Can it be focused?

Can it be reused?

Can it run on Pi Zero W?

If any answer is "No",

the widget should be redesigned.

---

# Acceptance Criteria

✓ Stateless rendering where possible

✓ Shared theme tokens

✓ Shared animation tokens

✓ Shared input actions

✓ Cached geometry

✓ Cached text

✓ No allocations during paint

✓ Pi Zero W budget respected

---

# Future Expansion

Virtualized lists

GPU compositing

Partial repaint

Display lists

Hardware cursors

Vector icon renderer

SVG support

Advanced typography

RTL languages

Localization

---

# Final Principle

A component should be so well-defined that two engineers implementing it independently produce nearly identical behaviour.