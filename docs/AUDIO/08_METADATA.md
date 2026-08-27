# FLACHEAD METADATA ARCHITECTURE

Document

08_METADATA.md

Category

Audio System

Status

Production Specification

Target Hardware

- Raspberry Pi Zero W
- ARMv6
- Raspberry Pi OS
- 2.8" portrait TFT touchscreen
- TANCHJIM BUNNY DSP
- Offline FLAC playback

---

# 1. PURPOSE

This document defines how FLACHEAD discovers, parses, normalizes, stores, caches, validates, and exposes audio metadata.

The metadata system covers:

- FLAC metadata blocks (STREAMINFO, VORBIS_COMMENT, PICTURE)
- track titles
- artist information
- album information
- album artist
- genre
- date/year
- track number
- disc number
- total tracks
- total discs
- composer
- grouping
- comments
- embedded artwork references
- replay-related metadata where applicable
- file identity
- metadata caching
- malformed metadata
- missing metadata
- metadata changes
- library scanning
- metadata editing

The primary objective is:

PROVIDE FAST, CONSISTENT, OFFLINE METADATA ACCESS WITHOUT MAKING THE AUDIO PLAYBACK PATH DEPEND ON REPEATED FILE PARSING.

---

# 2. CORE PRINCIPLE

Metadata is:

DESCRIPTIVE DATA.

It is not:

AUDIO PLAYBACK STATE.

The audio engine must be able to play a valid FLAC even when metadata is:

- incomplete
- malformed
- missing
- unusual
- inconsistent.

---

# 3. ARCHITECTURAL RELATIONSHIP

The intended flow is:

```
                      AUDIO FILE
                          │
                          ▼
                    METADATA READER
                          │
                          ▼
                    METADATA PARSER
                          │
                          ▼
                   NORMALIZATION
                          │
                          ▼
                  NORMALIZED TRACK MODEL
                          │
              ┌───────────┴───────────┐
              ▼                       ▼
        LIBRARY DATABASE          ARTWORK SYSTEM
              │                       │
              └───────────┬───────────┘
                          ▼
                     UI / SEARCH
                          │
                          ▼
                         QUEUE
                          │
                          ▼
                    PLAYBACK ENGINE
```

The decoder should not be responsible for maintaining the application-wide metadata database.

---

# 4. METADATA OWNERSHIP

The metadata parser owns:

- reading metadata blocks
- interpreting format-specific fields
- extracting embedded artwork information
- detecting malformed metadata.

The library service owns:

- persistent metadata records
- track identity
- metadata updates
- metadata availability.

The UI owns:

- presentation.

The audio engine owns:

- playback-specific runtime information.

The artwork subsystem owns:

- artwork extraction
- artwork decoding
- artwork resizing
- artwork caching
- artwork rendering assets.

The lyrics subsystem owns:

- lyrics parsing
- lyrics synchronization
- lyrics display.

---

# 5. SOURCE OF TRUTH

For a local FLAC file:

the file's embedded metadata is the primary source.

The library database is the cached application representation.

The cache must never become the only source of truth.

---

# 6. FLAC METADATA SOURCE

FLAC files commonly contain metadata through FLAC metadata blocks.

Relevant blocks include:

- STREAMINFO
- VORBIS_COMMENT
- PICTURE
- SEEKTABLE

FLACHEAD should use the appropriate block for each responsibility.

---

# 7. STREAMINFO

STREAMINFO provides technical audio information such as:

- sample rate
- channel count
- bits per sample
- total samples
- MD5 signature where available.

This information belongs primarily to the audio/technical metadata model.

---

# 8. VORBIS COMMENTS

Vorbis comments are the primary textual metadata source for FLAC.

Typical fields include:

TITLE

ARTIST

ALBUM

ALBUMARTIST

TRACKNUMBER

TRACKTOTAL

DISCNUMBER

DISCTOTAL

GENRE

DATE

COMPOSER

COMMENT

---

# 9. FIELD CASE

Metadata field names should be treated case-insensitively.

For example:

TITLE

and:

title

should resolve to the same logical field.

---

# 10. TAG ALIASES

The parser should support common tag spelling variations.

Examples:

ALBUMARTIST
ALBUM ARTIST

TRACKNUMBER
TRACK NUMBER

DISCNUMBER
DISC NUMBER

The canonical internal model should remain consistent.

---

# 11. MULTIPLE VALUES

Some tags can legitimately contain multiple values.

Examples:

- ARTIST
- GENRE
- COMPOSER

The parser must not blindly assume one value.

For fields where multiple values are meaningful:

store all values internally where practical.

For simple UI display:

use the defined primary value.

---

# 12. ARTIST REPRESENTATION

The system should distinguish between:

primary artist

and:

additional artists.

For example:

ARTIST:
Artist A

ARTIST:
Artist B

should not necessarily become an accidental string such as:

Artist A; Artist B

unless formatting for display specifically requires it.

---

# 13. DISPLAY FORMATTING

Raw metadata should remain structured.

The UI may create:

display strings

from structured values.

Do not destroy structure merely to simplify rendering.

---

# 14. TITLE

TITLE should represent the track title.

---

# 15. TITLE FALLBACK

If missing:

use a deterministic fallback.

Recommended fallback:

filename without extension.

Example:

01 - Example Song.flac

becomes:

01 - Example Song

if TITLE is missing.

---

# 16. ARTIST

ARTIST represents the track artist.

---

# 17. ARTIST FALLBACK

If ARTIST is missing:

use:

Unknown Artist

or the configured localized equivalent.

---

# 18. ALBUM

ALBUM represents the album title.

---

# 19. ALBUM FALLBACK

If ALBUM is missing:

use:

Unknown Album.

---

# 20. ALBUM ARTIST

ALBUMARTIST should be preferred for album-level grouping.

---

# 21. ALBUM ARTIST FALLBACK

If ALBUMARTIST is missing:

use ARTIST where appropriate.

Otherwise:

Unknown Album Artist.

---

# 22. GENRE

GENRE is descriptive metadata.

It should not automatically determine application behavior.

Genre may be multi-valued.

Do not assume one genre is always sufficient.

---

# 23. GENRE FALLBACK

Missing genre should not produce:

Unknown Genre

inside every UI component unless the UI explicitly needs a visible placeholder.

Internally:

genre may simply be absent.

---

# 24. DATE

DATE should be treated primarily as a display/sorting field.

DATE should be represented as metadata rather than assuming it is always a four-digit year.

Possible values include:

2026

2026-08-09

1987-04

depending on the source.

---

# 25. DATE NORMALIZATION

The metadata system should preserve:

original date value

and optionally provide:

normalized year

when extraction is unambiguous.

---

# 26. TRACK NUMBER

TRACKNUMBER may appear as:

1

01

1/12

01/12

The parser should separate:

track number

from:

track total

when possible.

Recommended model:

track_number

track_total

Both are optional.

---

# 27. DISC NUMBER

DISNUMBER/DISCNUMBER variants should be normalized to:

disc_number.

---

# 28. DISC TOTAL

DISCTOTAL should map to:

disc_total.

---

# 29. CANONICAL FIELD NAMES

Recommended internal fields:

title

artists

album

album_artist

genres

date

year

track_number

track_total

disc_number

disc_total

composer

comment

---

# 30. OPTIONAL FIELDS

Not every file has every field.

Optional metadata should be represented explicitly as:

missing/null/empty

rather than fabricated data.

---

# 31. UNKNOWN VS MISSING

The system should distinguish:

metadata is missing

from:

metadata exists and explicitly says "Unknown".

These are semantically different.

---

# 32. COMPOSER

COMPOSER should be preserved independently from ARTIST.

---

# 33. COMMENT

COMMENT is informational metadata.

It should not affect:

playback behavior.

---

# 34. CUSTOM TAGS

Unknown FLAC/Vorbis Comment fields should not cause parsing failure.

The parser may retain them in an extensible metadata map if useful.

Unknown tags should be treated as:

unrecognized metadata.

They must not be discarded from the source file during read-only scanning.

---

# 35. FILE IDENTITY

Every metadata record must be associated with a stable file identity.

A simple path alone may be insufficient because:

files can be renamed or moved.

---

# 36. PATH

The absolute or canonical path should still be stored.

Paths should use a consistent canonical representation.

---

# 37. FILE STATISTICS

Useful file identity information includes:

- file size
- modification time
- path
- optional inode/device information
- optional content hash.

---

# 38. METADATA CACHE KEY

A metadata cache key should detect when a file has changed.

A practical key can include:

canonical_path

+

file_size

+

modification_time.

---

# 39. CONTENT HASH

A full file hash may be more reliable but is potentially expensive.

Do not hash every FLAC file unnecessarily during normal library scans.

---

# 40. CACHE INVALIDATION

If the identity signature changes:

cached metadata should be considered stale.

---

# 41. FILE RENAMING

If a file moves:

path-based caches may become invalid.

The library scanner should be capable of rediscovering the file.

---

# 42. METADATA CHANGE

If a file's tags are edited:

the next scan should detect the modification.

The cache must then be refreshed.

---

# 43. TRACK ID

track_id must uniquely identify the library's logical track record.

It should not be derived solely from:

display title.

Track IDs must be:

- unique
- stable
- locally generated
- independent of UI ordering.

---

# 44. DUPLICATE TITLES

Two tracks may both be named:

Intro

They must remain separate records.

---

# 45. DUPLICATE FILES

Two identical files may exist at different paths.

The library must have deterministic behavior for duplicate entries.

---

# 46. ALBUM ID

An album grouping should not depend solely on:

album title.

A practical grouping key may include:

album_artist

+

album

+

disc context.

---

# 47. ARTIST ID

Artist grouping should use normalized metadata rather than display strings alone.

---

# 48. METADATA NORMALIZATION

Raw metadata should be normalized into an application-level representation.

Normalization may include:

- trimming accidental surrounding whitespace
- interpreting numeric fields
- normalizing field-name casing
- converting invalid numeric values to unknown
- preserving original textual content where important.

---

# 49. DO NOT OVER-NORMALIZE

Do not aggressively alter:

- artist capitalization
- punctuation
- Unicode characters
- intentional spacing.

---

# 50. UNICODE

FLACHEAD must support Unicode metadata.

Examples:

- Hindi
- Japanese
- Korean
- Arabic
- Cyrillic
- accented Latin characters.

Internal application strings should use UTF-8-compatible Unicode representation.

Malformed text must not crash the scanner.

Use safe replacement behavior where necessary.

---

# 51. SORTING

Metadata sorting should be deterministic.

Recommended album ordering:

album artist

album

disc number

track number.

Within an album:

disc_number

then:

track_number

should normally determine ordering.

---

# 52. TRACK TITLE SORTING

When sorting tracks alphabetically:

use normalized Unicode-aware comparison.

Case differences should not unexpectedly produce separate sort groups.

---

# 53. MISSING TRACK NUMBER

If track_number is missing:

place the track using a deterministic fallback such as:

filename

or:

title.

Do not randomly place it.

---

# 54. MULTI-DISC ALBUM

For multi-disc albums:

disc number must be considered before track number.

Example:

Disc 1 Track 1

comes before:

Disc 2 Track 1.

---

# 55. ALBUM ARTIST

Album grouping should generally prefer:

ALBUMARTIST

because an album can contain multiple track artists.

---

# 56. COMPILATIONS

Compilation albums commonly require:

album_artist

to represent:

Various Artists

or equivalent metadata.

The library should preserve the file's actual metadata where present.

---

# 57. NATURAL SORTING

Filename and title sorting may optionally use natural numeric ordering.

Example:

Track 2

before:

Track 10.

---

# 58. NUMERIC TAG PARSING

Fields such as:

TRACKNUMBER

TRACKTOTAL

DISCNUMBER

DISCTOTAL

should be parsed safely.

Invalid numeric strings should result in:

missing/invalid numeric state.

Values such as:

3/12

should produce:

track_number = 3

track_total = 12.

Values such as:

abc/xyz

must not crash the parser.

---

# 59. DATE PARSING

Date parsing should be conservative.

Do not reinterpret arbitrary strings into dates.

---

# 60. TECHNICAL AUDIO METADATA

The audio model should expose:

sample_rate

channels

bits_per_sample

total_frames

duration.

---

# 61. DURATION

Duration should preferably be calculated from:

total_frames / sample_rate.

This avoids dependence on textual duration metadata.

FLAC STREAMINFO provides authoritative total sample information where valid.

Use it for duration.

---

# 62. BIT DEPTH

Bits-per-sample should be represented independently from the storage format used internally.

Example:

24-bit source

may be processed using:

32-bit storage.

---

# 63. CHANNEL COUNT

Channel count is technical metadata.

Do not assume:

all music is stereo.

---

# 64. SAMPLE RATE

Sample rate must be treated as authoritative technical metadata.

Examples:

44.1 kHz

48 kHz

88.2 kHz

96 kHz

etc.

---

# 65. AUDIO FORMAT MODEL

Recommended conceptual model:

AudioFormat

    sample_rate
    channels
    bits_per_sample
    sample_format

---

# 66. TRACK MODEL

Recommended conceptual model:

Track

    track_id
    file_path
    title
    artists
    album
    album_artist
    genres
    date
    year
    track_number
    track_total
    disc_number
    disc_total
    composer
    comment
    audio_format
    duration_frames
    artwork_reference
    file_identity

---

# 67. METADATA IS DERIVED DATA

The cache must never become the only source of truth.

The FLAC file remains authoritative for embedded metadata.

---

# 68. OFFLINE OPERATION

Metadata functionality must work entirely offline.

No internet metadata service should be required for:

basic library operation.

If online metadata enrichment is added later:

it must remain optional.

The core library must continue functioning without network access.

---

# 69. METADATA PRIORITY

Recommended priority:

embedded metadata

then:

supported local metadata sources

then:

optional external enrichment.

---

# 70. NO NETWORK DEPENDENCY

Playback must never block waiting for:

online metadata.

---

# 71. METADATA LOAD TIMING

For a track selected for playback:

critical playback initialization should be able to proceed without waiting for expensive artwork processing.

---

# 72. AUDIO VS METADATA PRIORITY

Priority order:

1. Maintain audio playback.
2. Maintain audio timing.
3. Maintain user input responsiveness.
4. Load metadata.
5. Decode artwork.
6. Perform nonessential indexing.

---

# 73. METADATA EVENTS

The library may publish events such as:

METADATA_DISCOVERED

METADATA_UPDATED

METADATA_INVALID

ARTWORK_DISCOVERED

ARTWORK_UPDATED

TRACK_REMOVED

TRACK_ADDED.

---

# 74. EVENT THREADING

Metadata events must not execute expensive UI work directly on the scanner thread.

Publish events through the application's event system.

---

# 75. PLAYBACK SNAPSHOT

The audio system may consume a lightweight immutable Track object/snapshot.

It should not depend on a mutable scanner object.

---

# 76. METADATA IMMUTABILITY

Once a Track snapshot is supplied to active playback:

changes to the library database should not unexpectedly mutate active playback state.

---

# 77. ACTIVE TRACK

The playback engine should hold its own authoritative:

active Track snapshot.

---

# 78. LIBRARY RESCAN

If the library rescans the currently playing file:

the active playback snapshot remains stable.

The new metadata can be applied for future UI refreshes.

---

# 79. METADATA UPDATE DURING PLAYBACK

If the file is externally edited while playing:

do not reload metadata in a way that disrupts audio.

---

# 80. FILE CHANGE DURING PLAYBACK

If the underlying FLAC file changes while playing:

the playback engine should detect the situation according to file identity policy.

Do not silently continue with assumptions that the file is unchanged.

A reasonable policy is:

continue the current open file handle

and:

apply the updated metadata only after the current playback session ends.

The exact behavior must remain deterministic.

---

# 81. DELETED FILE DURING PLAYBACK

If the file is deleted after being opened:

Linux may keep the open file usable until the handle is closed.

The library should mark the track as removed without immediately breaking active playback.

---

# 82. NEXT AFTER DELETION

When playback transitions:

the queue/library must verify the next track is still available.

---

# 83. MISSING FILE

A library entry pointing to a missing file should be marked:

UNAVAILABLE.

It should not crash the library.

---

# 84. METADATA SAFETY

Metadata values must never be blindly interpreted as filesystem commands or executable paths.

Metadata must not be capable of:

- executing shell commands
- changing application configuration
- altering SQL syntax
- injecting UI markup.

All storage and rendering paths must use safe APIs.

---

# 85. SQLITE SAFETY

If metadata is stored in SQLite:

use parameterized queries.

Never construct SQL from raw tag strings.

---

# 86. UI SAFETY

If the UI supports markup/rich text:

metadata must be escaped before rendering.

---

# 87. METADATA LENGTH

Metadata can be unexpectedly large.

The parser should impose reasonable safety limits on:

- individual field lengths
- number of repeated tags
- artwork size
- total metadata memory.

---

# 88. HUGE TAG

A pathological tag should not be allowed to consume the entire Pi Zero W memory budget.

---

# 89. HUGE ARTWORK

A massive embedded image should be rejected, deferred, or handled through bounded streaming/resource logic.

Do not decode enormous images directly into unrestricted RAM.

---

# 90. ARTWORK REFERENCE

The metadata model should store enough information to identify artwork without necessarily retaining decoded pixels.

Example:

artwork_source

artwork_offset

artwork_length

or an extracted-cache path.

---

# 91. ARTWORK DECODE

Artwork decoding belongs to the artwork/resource subsystem.

It should not block:

FLAC decoding.

---

# 92. PICTURE BLOCK

FLAC PICTURE metadata may contain:

- MIME type
- picture type
- description
- width
- height
- color depth
- indexed color count
- binary image data.

---

# 93. PRIMARY ARTWORK

The library should identify a preferred artwork candidate.

Typical preference:

front cover

over:

other picture types.

---

# 94. MULTIPLE ARTWORKS

If multiple PICTURE blocks exist:

select deterministically.

Recommended priority:

1. Front Cover
2. Other relevant cover image
3. First usable embedded image

---

# 95. EXTERNAL ARTWORK

FLACHEAD may optionally support external artwork files.

Examples:

cover.jpg

folder.jpg

cover.png

The exact supported filenames belong to the library specification.

---

# 96. EMBEDDED VS EXTERNAL

Recommended preference:

embedded front cover

then:

supported external artwork.

This can be overridden by configuration if necessary.

---

# 97. ARTWORK CACHE

Decoded artwork should use a bounded cache.

Do not keep every album's full-resolution image in memory.

---

# 98. METADATA MEMORY MODEL

Library metadata should be relatively lightweight.

Avoid storing:

- duplicate strings
- unnecessary raw tag copies
- full FLAC binary data.

---

# 99. STRING OWNERSHIP

Metadata strings must have clear ownership.

Avoid accidental references to temporary parser buffers.

---

# 100. STRING NORMALIZATION

The parser may normalize:

- surrounding whitespace
- invalid control characters
- known tag-key formatting.

Do not aggressively rewrite user metadata.

---

# 101. WHITESPACE

Leading/trailing whitespace may be removed for display-oriented normalized fields.

The original raw value may be preserved if required for diagnostics.

---

# 102. EMPTY TAGS

An empty tag should normally be treated as:

missing.

Example:

TITLE=

should not produce an empty title throughout the UI.

---

# 103. INVALID UTF-8

Metadata encoding should be handled defensively.

If malformed text is encountered:

the file should still remain playable.

Invalid metadata should be converted into a safe display representation where possible.

Do not allow malformed text to crash:

- scanner
- UI
- database/cache
- playback.

---

# 104. SPECIAL CHARACTERS

Metadata may contain:

- Unicode
- emoji
- punctuation
- quotes
- slashes
- symbols.

The application must treat metadata as untrusted external data.

---

# 105. METADATA DIAGNOSTICS

Debug information should be able to show:

- source file
- parsed tags
- normalized fields
- warnings
- artwork source
- technical audio data.

---

# 106. RAW TAG DEBUGGING

Debug mode may expose raw tags.

Normal UI should not require them.

---

# 107. PARSER WARNINGS

Warnings may include:

- malformed numeric tag
- invalid UTF-8
- duplicate metadata
- oversized tag
- invalid artwork
- unsupported picture type.

Warnings should not automatically mean:

playback failure.

---

# 108. PARSER ERRORS

A fatal parser error should only occur when the file cannot be safely interpreted.

Even then:

the playback engine should distinguish:

metadata failure

from:

audio decoding failure.

---

# 109. METADATA FAILURE

Metadata failure should not automatically mean:

cannot play.

---

# 110. AUDIO FAILURE

A FLAC decoding failure is different from:

bad title metadata.

Audio failure affects playback.

Metadata failure affects descriptive information.

---

# 111. SEPARATION

The architecture must preserve:

METADATA PATH

and:

AUDIO PATH

as separate logical systems.

---

# 112. METADATA PATH

The metadata path is:

FILE
 ↓
PARSER
 ↓
NORMALIZER
 ↓
CACHE
 ↓
LIBRARY
 ↓
UI.

---

# 113. AUDIO PATH

The audio path is:

FILE
 ↓
DECODER
 ↓
PCM BUFFER
 ↓
OUTPUT
 ↓
DSP.

---

# 114. SHARED FILE

Both paths may inspect the same FLAC file.

They must not interfere with each other's timing requirements.

---

# 115. FILE I/O COORDINATION

Concurrent metadata scanning and audio decoding may access the SD card.

The implementation should avoid unnecessary duplicate reads.

However:

audio playback takes priority.

---

# 116. ACTIVE PLAYBACK FILE

The active playback file should not be repeatedly reopened by metadata code unnecessarily.

---

# 117. LIBRARY SCANNER THROTTLING

During playback:

library scanning may be throttled.

This reduces:

SD-card contention

and:

CPU contention.

---

# 118. ARTWORK THROTTLING

Artwork extraction should also be throttled during playback if necessary.

---

# 119. METADATA CACHE LOCATION

The metadata cache should live outside the music files themselves.

It should use an application-managed location.

---

# 120. CACHE REBUILD

If the cache is deleted:

the library can rebuild it.

No music files should need modification.

---

# 121. CACHE WRITE SAFETY

Cache updates should be atomic enough to avoid leaving a corrupted database after power loss.

---

# 122. POWER LOSS

Because FLACHEAD is an embedded/offline device:

cache writes should avoid unnecessary high-frequency disk writes.

---

# 123. WRITE BATCHING

Metadata changes discovered during scanning may be committed in batches.

Do not perform a disk sync for every individual tag.

---

# 124. SD CARD WEAR

Avoid unnecessary repeated writes to the SD card.

The library cache should not constantly rewrite unchanged metadata.

---

# 125. DETERMINISTIC SCAN

Given the same music directory and file contents:

the library scanner should produce the same logical metadata results.

---

# 126. SCAN MODES

The library system should support at least:

FULL SCAN

and:

INCREMENTAL SCAN.

---

# 127. FULL SCAN

A full scan evaluates all configured music locations.

Use for:

- first setup
- rebuilding the library
- recovery
- major storage changes.

---

# 128. INCREMENTAL SCAN

An incremental scan checks files that are:

- new
- modified
- removed
- moved when detectable.

---

# 129. AUDIO THREAD ISOLATION

Metadata scanning MUST NOT run synchronously on the critical audio output thread.

---

# 130. SCANNING PRIORITY

Library scanning should have lower priority than:

audio playback.

---

# 131. METADATA PARSING COST

Parsing metadata should not cause:

- audio underruns
- UI freezes
- excessive memory allocation.

---

# 132. LARGE ARTWORK

Embedded artwork may be large.

The metadata scanner should avoid keeping full-resolution artwork for every track in RAM.

---

# 133. QUEUE INTERACTION

Queue entries reference track IDs.

Therefore metadata changes should not invalidate queue membership.

---

# 134. PLAYLIST INTERACTION

Playlist entries reference track IDs.

Therefore metadata changes should not invalidate playlist membership.

---

# 135. HISTORY INTERACTION

Playback history should reference track IDs rather than storing only titles.

---

# 136. SEARCH INDEX

Search should operate against normalized metadata fields.

Potential searchable fields:

- title
- artist
- album
- album artist
- genre
- composer.

---

# 137. SEARCH PERFORMANCE

The database should support indexed searches for common fields.

A lightweight full-text index may be added later if normal indexed queries become insufficient.

---

# 138. SEARCH NORMALIZATION

Search comparison should handle:

- case differences
- Unicode normalization where appropriate
- accidental surrounding whitespace.

---

# 139. ORIGINAL VALUES

Where possible, retain source metadata exactly enough to avoid destructive rewriting.

The normalized representation is for application behavior.

---

# 140. WRITE SUPPORT

If FLACHEAD later supports editing metadata:

metadata writing must be a separate subsystem.

Do not mix read-only scanning and destructive metadata editing.

---

# 141. METADATA EDITING

Editing should:

1. read current metadata
2. modify requested fields
3. write a valid FLAC file
4. verify the result
5. update the library database.

---

# 142. ATOMIC WRITE

Metadata edits should use a safe file-replacement strategy where practical.

---

# 143. WRITE FAILURE

If metadata writing fails:

the original file must remain recoverable.

---

# 144. DATABASE CONSISTENCY

The database should be updated only after the file write succeeds.

---

# 145. READ AFTER WRITE

After metadata modification:

re-read the resulting metadata.

Do not assume the write succeeded merely because the file operation returned successfully.

---

# 146. PLAYBACK DURING WRITE

Metadata editing should not modify a file that is actively being decoded unless the architecture explicitly supports this safely.

Recommended:

prevent metadata writes on the currently playing file.

---

# 147. FILE LOCKING

The implementation should use appropriate locking/coordination when metadata writes and playback could conflict.

---

# 148. BACKGROUND SCANNING

Metadata scanning may run in a background worker.

---

# 149. UI THREAD

The UI thread must not synchronously scan the entire music collection.

---

# 150. DATABASE CONNECTIONS

Database access should follow the project's database/threading architecture.

Do not share unsafe database connection objects across threads.

---

# 151. ERROR LOGGING

Metadata parsing failures should include enough information to diagnose the file.

Recommended:

- track path
- parser stage
- metadata block
- error category.

Avoid excessive repeated logging during every UI refresh.

---

# 152. LOG SEVERITY

Suggested categories:

INFO

WARNING

ERROR.

Malformed optional metadata:

WARNING.

Invalid FLAC stream:

ERROR.

---

# 153. USER-FACING ERRORS

Low-level parser errors should generally not be exposed directly to the user.

Instead show:

Metadata unavailable

or:

Track unavailable.

---

# 154. DEBUG DETAILS

Detailed parser errors belong in logs/debug tools.

---

# 155. ACCEPTANCE CRITERIA

The metadata architecture is production-ready when:

- valid FLAC metadata is parsed correctly
- missing tags do not break playback
- malformed tags do not crash the application
- technical audio metadata is separated from descriptive metadata
- multi-value tags are supported where appropriate
- track/disc numbers are parsed safely
- album grouping is deterministic
- metadata cache invalidation works
- changed files are detected
- missing files are handled
- artwork references are bounded
- oversized metadata cannot exhaust RAM
- metadata scanning cannot block audio playback
- cache corruption can be recovered
- offline operation requires no network metadata service
- active playback is isolated from library rescans
- metadata is safe for UI/database rendering
- all metadata remains associated with the correct track identity.

---

# 156. ACCEPTANCE TEST: BASIC METADATA

Create a FLAC containing:

TITLE

ARTIST

ALBUM.

Scan it.

Verify all fields appear correctly.

---

# 157. ACCEPTANCE TEST: MISSING TITLE

Remove TITLE.

Verify filename fallback is used.

---

# 158. ACCEPTANCE TEST: MISSING ARTIST

Remove ARTIST.

Verify:

Unknown Artist

or configured fallback.

---

# 159. ACCEPTANCE TEST: MULTI-DISC

Create:

Disc 1 Track 1

Disc 2 Track 1.

Verify correct album ordering.

---

# 160. ACCEPTANCE TEST: COMPILATION

Create tracks with:

same ALBUM

same ALBUMARTIST

different ARTIST.

Verify they remain one album.

---

# 161. ACCEPTANCE TEST: UNICODE

Use metadata containing:

Hindi

Japanese

Arabic

and accented Latin characters.

Verify:

correct storage

correct rendering

correct searching.

---

# 162. ACCEPTANCE TEST: CORRUPTED COMMENT

Corrupt optional text metadata.

Verify:

track scanning remains recoverable.

---

# 163. ACCEPTANCE TEST: CORRUPTED ARTWORK

Use invalid artwork.

Verify:

text metadata remains available.

---

# 164. ACCEPTANCE TEST: METADATA CHANGE

Modify TITLE externally.

Run a scan.

Verify:

database reflects the new title.

---

# 165. ACCEPTANCE TEST: PATH MOVE

Move a track.

Rescan.

Verify:

track identity remains stable when identity detection succeeds.

---

# 166. ACCEPTANCE TEST: QUEUE

Start a track.

Modify its title.

Verify:

playback continues.

---

# 167. ACCEPTANCE TEST: PLAYLIST

Modify metadata for a playlist track.

Verify:

playlist membership remains unchanged.

---

# 168. ACCEPTANCE TEST: LARGE LIBRARY

Scan a large FLAC collection.

Verify:

UI remains responsive.

---

# 169. ACCEPTANCE TEST: RESCAN PERFORMANCE

Run a second scan without file changes.

Verify:

unnecessary metadata parsing is minimized.

---

# 170. ACCEPTANCE TEST: FORCED RESCAN

Force metadata refresh.

Verify:

external metadata modifications are detected.

---

# 171. ACCEPTANCE TEST: DATABASE FAILURE

Simulate metadata database failure.

Verify:

application fails gracefully and does not crash the audio engine.

---

# 172. ACCEPTANCE TEST: INVALID FILE

Place a non-FLAC or corrupt file in the music directory.

Verify:

scanner reports/skips it safely.

---

# 173. AI CODING AGENT RULES

The coding agent MUST:

1. Treat metadata as untrusted external data.
2. Keep metadata separate from the PCM playback path.
3. Never require metadata for basic FLAC playback.
4. Parse FLAC technical metadata from appropriate FLAC blocks.
5. Parse Vorbis comments case-insensitively.
6. Support common tag aliases.
7. Preserve multi-value fields where appropriate.
8. Keep raw metadata separate from normalized metadata when useful.
9. Use deterministic duplicate-tag behavior.
10. Parse track/disc numbers safely.
11. Derive duration from authoritative audio information.
12. Keep artwork references separate from decoded artwork.
13. Use bounded artwork memory.
14. Never allow oversized metadata to exhaust RAM.
15. Use parameterized database queries.
16. Escape metadata before rich UI rendering.
17. Treat cache data as derived data.
18. Detect stale metadata using file identity.
19. Support full and incremental scans.
20. Keep scanning away from the audio output thread.
21. Prioritize audio playback over indexing.
22. Avoid unnecessary SD-card writes.
23. Handle deleted and modified files safely.
24. Keep active playback snapshots stable during library rescans.
25. Never use online services as a requirement for offline playback.
26. Keep metadata failures separate from decoder failures.
27. Make metadata sorting deterministic.
28. Make album/disc/track grouping deterministic.
29. Make cache corruption recoverable.
30. Never let metadata processing crash the audio engine.

---

# 174. FINAL METADATA ARCHITECTURE

The intended architecture is:

```
                         FLAC FILE
                            │
             ┌──────────────┴──────────────┐
             │                             │
             ▼                             ▼
       METADATA READER                FLAC DECODER
             │                             │
             ▼                             ▼
       METADATA PARSER                  PCM BUFFER
             │                             │
             ▼                             ▼
        NORMALIZER                    AUDIO OUTPUT
             │                             │
             ▼                             ▼
      NORMALIZED TRACK                    ALSA
             │                             │
     ┌───────┴───────┐                     │
     ▼               ▼                     ▼
  DATABASE      ARTWORK CACHE       TANCHJIM BUNNY DSP
     │
     ├──────────┬──────────┐
     ▼          ▼          ▼
   SEARCH     QUEUE     PLAYLISTS
     │
     ▼
     UI
```

The central invariant is:

METADATA MUST NEVER BE REQUIRED FOR AUDIO PLAYBACK.

The second invariant is:

A METADATA ERROR MUST NOT BECOME AN AUDIO ERROR UNLESS THE UNDERLYING FILE ITSELF CANNOT BE SAFELY DECODED.

The third invariant is:

THE FLAC FILE IS THE AUTHORITATIVE SOURCE FOR EMBEDDED METADATA; THE CACHE IS ONLY DERIVED DATA.

The fourth invariant is:

NO METADATA SCAN, ARTWORK DECODE, OR LIBRARY DATABASE OPERATION MAY STARVE THE AUDIO OUTPUT PATH.

The fifth invariant is:

EVERY TRACK'S METADATA MUST REMAIN ASSOCIATED WITH ITS CORRECT FILE IDENTITY AND TRACK ID.

Final policy:

Parse metadata defensively.

Normalize it into a stable internal model.

Cache it efficiently.

Keep artwork bounded.

Keep technical audio information authoritative.

Keep library operations asynchronous.

Keep playback independent.

Keep everything offline-first.

And never allow a badly tagged FLAC to become a reason for FLACHEAD to crash.
