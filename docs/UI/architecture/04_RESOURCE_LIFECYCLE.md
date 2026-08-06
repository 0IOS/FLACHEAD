# 04_RESOURCE_LIFECYCLE.md

# FLACHEAD UI — Resource Lifecycle Specification

## 1. Purpose

This document defines how FLACHEAD manages the lifecycle of UI resources.

The Raspberry Pi Zero W has limited resources:

* low RAM
* limited CPU power
* slower storage
* limited graphics capability

Therefore, every resource must have a clear lifecycle:

* creation
* loading
* usage
* caching
* release

Primary principle:

> Resources should exist only when needed and remain cached only when they provide real performance benefits.

---

# 2. Design goals

Resource management must be:

* memory efficient
* predictable
* low overhead
* leak-free
* playback-safe

---

# 3. Resource categories

FLACHEAD manages:

```text
1. Images

2. Fonts

3. UI Assets

4. Audio Metadata

5. Database Objects

6. Temporary Buffers
```

---

# 4. Resource architecture

Data flow:

```text
Resource Request

↓

Resource Manager

↓

Cache Lookup

↓

Load From Storage

↓

Return Resource

↓

Release When Unused
```

---

The UI must not manually manage resources.

---

# 5. Resource Manager

Required module:

```text
Resource Manager

├── Asset Loader

├── Cache Manager

├── Memory Tracker

├── Resource Resolver

└── Cleanup System
```

---

Responsibilities:

* loading assets
* tracking ownership
* freeing unused resources
* preventing duplicate loads

---

# 6. Image lifecycle

Images include:

* album artwork
* icons
* backgrounds
* UI graphics

Lifecycle:

```text
Request Image

↓

Check Cache

↓

Load Image

↓

Resize

↓

Store

↓

Render

↓

Release
```

---

# 7. Album artwork management

Artwork is one of the largest UI resources.

Rules:

* never decode during rendering
* cache resized versions
* release unused artwork

---

Recommended cache:

```text
Current Track Artwork

+

Recently Used Artwork
```

---

Avoid:

Keeping entire music library artwork in RAM.

---

# 8. Image resolution handling

Images should be resized before rendering.

Example:

```text
Original:

3000x3000


↓

Cache:

320x320
```

---

Reason:

The display does not benefit from oversized images.

---

# 9. Font lifecycle

Fonts are loaded:

```text
Startup

↓

Font Cache

↓

Reusable Rendering
```

---

Do not reload fonts per screen.

---

Cached:

* main font
* small font
* icon font

---

# 10. UI asset lifecycle

Static assets:

Examples:

* icons
* buttons
* system graphics

Loaded:

During startup.

---

Dynamic assets:

Examples:

* artwork
* generated themes

Loaded:

On demand.

---

# 11. Temporary resource handling

Temporary resources:

* animation buffers
* transition images
* calculation buffers

---

Rules:

Create:

When required.

Destroy:

Immediately after use.

---

# 12. Database resource handling

Database objects must:

* use lazy loading
* avoid unnecessary queries
* release connections

---

Example:

Bad:

```text
Load every song object
```

---

Good:

```text
Load visible tracks only
```

---

# 13. Cache strategy

FLACHEAD uses:

```text
Memory Cache

↓

Disk Cache

↓

Original Source
```

---

Priority:

Fast access > storage usage.

---

# 14. Cache limits

Every cache requires:

* maximum size
* cleanup policy
* expiration rules

---

Example:

Artwork cache:

```text
Maximum:

Defined by available RAM
```

---

# 15. Memory pressure handling

When memory is low:

System should:

1. Remove unused resources.
2. Reduce cache size.
3. Release temporary buffers.

---

Never:

Interrupt playback.

---

# 16. Resource ownership

Every resource must have:

```text
Owner

Lifetime

Cleanup Method
```

---

Example:

```text
Player Screen

owns:

Current Artwork
```

---

# 17. Background resource loading

Heavy resources may load asynchronously.

Examples:

* artwork
* large metadata sets

---

Flow:

```text
Request

↓

Background Loader

↓

Resource Ready Event

↓

UI Update
```

---

# 18. Performance requirements

Resource operations must:

* avoid frame drops
* avoid audio interruption
* minimize allocations

---

Targets:

Memory allocations:

```text
Avoid during frame rendering
```

---

# 19. Error handling

Missing resource:

Use fallback.

Example:

```text
Missing Artwork

↓

Generated Placeholder
```

---

Corrupt asset:

Skip safely.

---

# 20. Acceptance criteria

Implementation is complete when:

* resources have clear ownership
* memory remains stable
* caches are controlled
* artwork loading is smooth
* no leaks occur

---

# 21. Implementation notes for AI coding agent

When implementing:

* Create a centralized Resource Manager.
* Never load large assets directly from screens.
* Cache intelligently.
* Profile memory on real Raspberry Pi Zero W.
* Prefer predictable memory usage over maximum caching.

A well-designed resource lifecycle allows FLACHEAD to provide a premium interface despite extremely limited hardware.
