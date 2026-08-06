# Settings

Component ID

SETTINGS

Category

System Screen

Status

Stable

Depends On

• STATUS_BAR
• NAVIGATION_BAR
• LIST
• LIST_ITEM
• SEARCH_BAR
• SWITCH
• SLIDER
• BUTTON
• CARD
• THEME_ENGINE

---

# Purpose

Settings provides centralized configuration for the FLACHEAD operating system.

It exposes every configurable system feature while remaining approachable and uncluttered.

---

# Philosophy

Every setting should have one obvious home.

Users should never wonder where a setting belongs.

---

# Goals

Simple

Organized

Fast

Searchable

Offline

Consistent

---

# Entry

Launcher

↓

Settings

---

# Layout

┌────────────────────────────┐

Status Bar

────────────────────────────

Search

────────────────────────────

Setting Categories

────────────────────────────

Navigation Bar

└────────────────────────────┘

---

# Categories

Playback

Audio

Library

Display

Appearance

Device

Connectivity

Power

Storage

Notifications

Accessibility

Developer

Plugins

About

---

# Search

Always available.

Instant.

Searches

Category

Setting Name

Description

Aliases

Plugin Settings

---

# Playback

Gapless

ReplayGain

Crossfade

Resume Playback

Shuffle

Repeat

Playback Speed

Sleep Timer

Resume Queue

Last Position

Auto Resume

---

# Audio

Output Device

Sample Rate

Exclusive Mode

USB DAC

Bluetooth Codec

Channel Balance

ReplayGain Mode

Preamp

Limiter

Bit-perfect Mode

Future

EQ

DSP

---

# Library

Scan Now

Auto Scan

Folder Management

Collections

Playlists

Ignored Folders

Artwork Cache

Metadata Refresh

Duplicate Detection

---

# Display

Brightness

Screen Timeout

Always On

Refresh Rate

Animations

Blur Strength

Status Bar

Navigation Bar

OLED Protection

Future

---

# Appearance

Wallpaper

Accent Style

Dark Theme

Font Size

Icon Shape

Animation Speed

Reduced Motion

Dynamic Theme

Wallpaper Colors

---

# Device

Date & Time

Language

Buttons

Encoder

Volume Knob

Touch Calibration

Startup Screen

Shutdown Screen

Reset Settings

---

# Connectivity

Wi-Fi

Bluetooth

USB Mode

ADB

Developer Network

---

# Power

Battery Saver

CPU Profile

Auto Shutdown

Thermal Policy

Charging

Wake Behaviour

Suspend Timeout

---

# Storage

Library Size

Free Space

Artwork Cache

Logs

Database

Clear Cache

Storage Health

---

# Notifications

Playback Notifications

Library Notifications

System Notifications

Plugin Notifications

Do Not Disturb

LED (Future)

---

# Accessibility

Large Text

High Contrast

Reduced Motion

Touch Sensitivity

Button Repeat

Screen Reader

Future

---

# Developer

FPS

Memory

CPU

GPU

Frame Time

Audio Stats

Theme Debug

Layout Inspector

Log Viewer

Benchmark

---

# Plugins

Installed Plugins

Updates

Permissions

Disable

Developer Plugins

---

# About

Version

Build

Kernel

SDL Version

Libraries

Licenses

Credits

Storage

Hardware

System Report

---

# Setting Types

Switch

Slider

Dropdown

Navigation

Dialog

Color Picker

Folder Picker

Button

---

# Theme

Wallpaper Palette

↓

Settings

Album colors never used.

---

# Animation

Category

Slide

Search

Fade

Switch

Spring

Dialog

Scale

Interruptible

Yes

---

# Performance Budget

Open

<40ms

Memory

<20MB

Search

<20ms

No allocations while idle.

---

# Accessibility

Keyboard

GPIO

Encoder

Touch

Supported equally.

---

# Error Handling

Invalid Setting

↓

Reset Default

Missing Plugin

↓

Hide Entry

Corrupted Config

↓

Restore Backup

Never crash.

---

# Acceptance Criteria

✓ Instant search

✓ Pi Zero optimized

✓ Fully searchable

✓ Consistent categories

✓ Plugin ready

✓ No duplicate settings

---

# Future

Cloud Backup

Profiles

Import/Export

Themes

Remote Control

Automation

---

# Final Principle

Settings should feel like a carefully organized toolbox rather than a list of unrelated switches.