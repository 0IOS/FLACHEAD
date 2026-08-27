# FLACHEAD AUDIO LIBRARY AND SCAN ARCHITECTURE

Document

19_AUDIO_LIBRARY_AND_SCAN_ARCHITECTURE.md

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

This document defines how FLACHEAD discovers music files, builds the offline music library, updates it incrementally, handles removed or changed files, groups tracks into albums/artists/genres, and exposes library data to the rest of the application.

The library system covers:

- music-root configuration
- directory traversal
- FLAC discovery
- file identity
- metadata extraction
- incremental scanning
- full scanning
- change detection
- database/cache storage
- album grouping
- artist grouping
- genre grouping
- sorting
- duplicate handling
- unavailable files
- removable/swappable SD-card content
- scan performance
- background operation
- recovery

The primary objective is:

PROVIDE A FAST, DETERMINISTIC, OFFLINE MUSIC LIBRARY WITHOUT INTERFERING WITH AUDIO PLAYBACK.

---

# 2. CORE PRINCIPLE

The music library is:

DERIVED DATA.

The actual music files remain authoritative.

The database/cache must always be rebuildable from the filesystem.

---

# 3. SOURCE OF TRUTH

The hierarchy is:

MUSIC FILE
    ↓
FLAC METADATA
    ↓
LIBRARY DATABASE
    ↓
UI / SEARCH / BROWSE

The database must never become the only copy of important metadata.

---

# 4. MUSIC ROOTS

FLACHEAD should support one or more configured music roots.

Example:

/music

Additional roots may be supported later.

---

# 5. ROOT VALIDATION

Before scanning a root:

verify that:

- the path exists
- it is accessible
- it is a directory
- the filesystem can be read.

If validation fails:

the scanner reports an error and does not crash.

---

# 6. OFFLINE-FIRST

The library must function entirely without:

- internet access
- cloud services
- remote metadata APIs
- authentication.

---

# 7. SUPPORTED AUDIO FILES

The initial library target is:

FLAC.

The scanner should not attempt to parse arbitrary files as FLAC.

---

# 8. EXTENSION CHECK

A fast initial filter may use:

.flac

matching should be case-insensitive.

Examples:

song.flac

SONG.FLAC

Song.FlAc

---

# 9. CONTENT VALIDATION

Extension matching is not enough.

The file should ultimately be validated as a readable FLAC.

A file named:

song.flac

that is not actually FLAC must not enter the playable library as a valid track.

---

# 10. DIRECTORY TRAVERSAL

The scanner should recursively traverse configured music roots.

Example:

/music
    /Artist
        /Album
            01.flac
            02.flac

---

# 11. DIRECTORY ORDER

Filesystem enumeration order must not determine library ordering.

All user-visible ordering must be explicitly sorted.

---

# 12. SYMLINK POLICY

The scanner must define deterministic symlink behavior.

Recommended default:

do not recursively follow directory symlinks.

This prevents accidental:

- cycles
- duplicate trees
- unexpected filesystem traversal.

---

# 13. FILE SYMLINKS

File symlinks may be supported if explicitly configured.

If supported:

the resolved target must be validated.

---

# 14. CYCLE PROTECTION

If symlink traversal is enabled:

the scanner must maintain sufficient visited-directory identity information to prevent cycles.

---

# 15. PERMISSION ERRORS

If a directory cannot be read:

record the error.

Continue scanning other accessible directories.

Do not abort the entire library scan.

---

# 16. BROKEN LINKS

Broken links should be ignored or reported as scan warnings.

They must not crash the scanner.

---

# 17. SPECIAL FILES

The scanner should ignore:

- sockets
- devices
- FIFOs
- unrelated special filesystem objects.

Only regular audio files should enter the music library.

---

# 18. FILE IDENTITY

Each discovered track requires a stable internal identity.

Recommended conceptual model:

FileIdentity

    canonical_path
    size
    modification_time
    optional_device
    optional_inode

---

# 19. PATH NORMALIZATION

Paths should be canonicalized consistently.

Avoid storing multiple representations of the same file accidentally.

---

# 20. FILE SIZE

File size is useful for detecting changes.

A file whose size changes should be considered potentially modified.

---

# 21. MODIFICATION TIME

Modification time is another primary change-detection signal.

---

# 22. INODE

On filesystems where reliable:

inode/device information can improve identity detection.

Do not make the entire architecture dependent on inode stability.

---

# 23. CONTENT HASH

Full content hashing should not be required for every scan.

It is too expensive for large FLAC libraries on the Pi Zero W.

---

# 24. OPTIONAL HASH

A hash may be calculated when:

- duplicate detection is requested
- file identity is ambiguous
- verification is explicitly requested.

---

# 25. SCAN TYPES

The library must support:

1. Initial scan
2. Full rescan
3. Incremental scan
4. Targeted rescan

---

# 26. INITIAL SCAN

The first scan builds the library from scratch.

Steps:

discover files

→ parse metadata

→ validate tracks

→ insert records

→ build indexes.

---

# 27. FULL RESCAN

A full rescan evaluates the entire configured music tree.

Use cases:

- recovery
- changed storage
- database rebuild
- user-requested rescan.

---

# 28. INCREMENTAL SCAN

Incremental scanning should avoid reparsing unchanged files.

Use file identity information to determine whether metadata must be refreshed.

---

# 29. TARGETED SCAN

A targeted scan can refresh:

- one directory
- one album
- one file.

This is useful after external edits.

---

# 30. SCAN STATE

Recommended scanner states:

IDLE

SCANNING

PAUSED

COMPLETED

CANCELLED

ERROR.

---

# 31. SCAN PROGRESS

The scanner may expose:

files discovered

files processed

files added

files changed

files removed

files failed.

---

# 32. PROGRESS ESTIMATION

The total file count may not be known at scan start.

Do not require a perfectly accurate percentage.

A determinate progress bar should only be shown when the denominator is reliable.

---

# 33. BACKGROUND SCANNING

Scanning should execute outside:

- audio output thread
- main UI rendering thread.

---

# 34. SCAN PRIORITY

Recommended priority:

audio playback

then:

UI responsiveness

then:

library scanning.

---

# 35. CPU LIMIT

Scanning must not monopolize the Pi Zero W CPU.

---

# 36. I/O LIMIT

Scanning should avoid unnecessary repeated reads from the SD card.

---

# 37. METADATA PARSING

Only parse metadata when necessary.

Unchanged files should use cached metadata.

---

# 38. CACHE VALIDATION

For each cached track:

compare its stored file identity against the current filesystem state.

---

# 39. UNCHANGED FILE

If:

path

+

size

+

modification time

remain unchanged:

reuse cached metadata.

---

# 40. CHANGED FILE

If any identity field indicates change:

re-read metadata.

---

# 41. NEW FILE

A file not present in the database should be:

parsed

validated

and inserted.

---

# 42. REMOVED FILE

A database entry whose file no longer exists should be marked:

unavailable

or removed according to library policy.

---

# 43. RECOMMENDED REMOVAL POLICY

Do not immediately destroy all metadata for a missing file.

Mark it unavailable first.

This allows:

- temporary SD removal
- removable storage handling
- recovery
- debugging.

---

# 44. SWAPPABLE SD CARD

Because music storage may be swapped:

the library must tolerate an entire music root temporarily disappearing.

---

# 45. ROOT DISAPPEARANCE

If the configured music root disappears:

do not interpret this immediately as:

"every song was intentionally deleted."

Instead mark the root:

UNAVAILABLE.

---

# 46. ROOT RETURN

When the root becomes available again:

run an incremental/full reconciliation according to file identity.

---

# 47. ACTIVE TRACK DURING STORAGE REMOVAL

If the currently playing file remains open:

the playback system may continue according to operating-system file semantics.

The library must not forcibly terminate playback simply because its path disappeared from directory enumeration.

---

# 48. NEXT TRACK AFTER REMOVAL

Before advancing to a future queue item:

verify that its file is available.

---

# 49. QUEUE INTEGRITY

A library scan must not silently reorder or destroy the active playback queue.

The queue is a separate playback object.

---

# 50. LIBRARY VS QUEUE

Library:

"What music exists?"

Queue:

"What should play next?"

They must remain separate.

---

# 51. TRACK OBJECT

A library track should contain or reference:

- track_id
- file identity
- metadata
- technical audio information
- artwork reference
- availability state.

---

# 52. TRACK ID

track_id must remain stable enough for:

- queue references
- playback history
- UI selection
- metadata updates.

It must not be based solely on:

track title.

---

# 53. DATABASE MODEL

A practical database may contain:

tracks

albums

artists

genres

artwork

scan_roots

library_metadata.

---

# 54. TRACK TABLE

Conceptual fields:

track_id

path

file_size

modified_time

title

album_id

artist_id

album_artist_id

track_number

track_total

disc_number

disc_total

duration_frames

sample_rate

channels

bits_per_sample

availability

---

# 55. ALBUM TABLE

Conceptual fields:

album_id

album_name

album_artist_id

date

artwork_id

track_count.

---

# 56. ARTIST TABLE

Conceptual fields:

artist_id

display_name.

---

# 57. GENRE TABLE

Conceptual fields:

genre_id

display_name.

---

# 58. NORMALIZATION

Grouping should use normalized comparison values.

Example:

"Artist"

and:

"artist"

may be treated as the same grouping value if the configured normalization policy allows it.

---

# 59. DISPLAY VALUE

Normalization must not destroy the original preferred display form.

Store:

canonical comparison value

and:

display value

when necessary.

---

# 60. ALBUM GROUPING

Album identity should generally include:

album name

+

album artist

+

relevant disc context.

---

# 61. ALBUM ARTIST

Album grouping should prefer:

ALBUMARTIST

over individual:

ARTIST.

This correctly handles:

compilations

and:

multi-artist albums.

---

# 62. MULTI-DISC ALBUMS

A multi-disc album must preserve:

disc number

and:

disc total.

---

# 63. TRACK ORDER

Recommended order:

disc number ascending

then:

track number ascending

then:

deterministic fallback.

---

# 64. MISSING TRACK NUMBERS

Missing track numbers must not break album browsing.

Fallback:

filename

then:

title.

---

# 65. ARTIST GROUPING

Artist pages should be derived from normalized artist metadata.

---

# 66. MULTI-ARTIST TRACKS

A track with multiple artists may belong to multiple artist groupings.

The underlying Track remains one object.

---

# 67. GENRE GROUPING

Genre grouping may similarly support multiple genres.

---

# 68. UNKNOWN GROUPS

Missing artist/album information should not create an unlimited number of malformed groups.

Use deterministic fallback groups when the UI requires them.

---

# 69. SEARCH

The library should support searching across:

- title
- artist
- album
- album artist
- genre.

---

# 70. SEARCH INDEX

The first implementation may use database queries rather than a dedicated full-text engine.

Do not introduce a heavy search subsystem without demonstrated need.

---

# 71. SEARCH SAFETY

Search strings must use:

parameterized queries.

---

# 72. SORTING

Library results must have explicit sorting.

Examples:

TITLE ASC

ARTIST ASC

ALBUM ASC

DATE DESC.

---

# 73. NATURAL SORTING

For track numbers and filenames:

natural numeric sorting may be useful.

Example:

2

before:

10.

---

# 74. USER SORT

If the UI allows sorting:

the library layer provides the sorted result.

The UI should not load the entire database just to sort it.

---

# 75. PAGINATION

Large libraries should support bounded result retrieval.

Do not load thousands of tracks into UI memory unnecessarily.

---

# 76. LIST VIEWS

The UI should request:

the visible range

plus:

a small prefetch window.

---

# 77. MEMORY CONSTRAINT

The Pi Zero W has limited memory.

Avoid maintaining:

multiple full copies

of the library in RAM.

---

# 78. DATABASE CACHE

SQLite is suitable for a lightweight local library database.

The exact implementation should remain consistent with the project's database architecture.

---

# 79. DATABASE AS CACHE

Even if SQLite is used:

the filesystem remains authoritative.

---

# 80. TRANSACTIONS

Large scan updates should use transactions.

Do not commit every individual track if it can be avoided.

---

# 81. BATCH INSERTION

New tracks should be inserted in batches.

This improves:

- performance
- SD-card behavior
- database consistency.

---

# 82. ATOMIC SCAN UPDATE

A scan should avoid leaving the database in an obviously inconsistent state if the application exits unexpectedly.

---

# 83. CRASH DURING SCAN

If FLACHEAD crashes during a scan:

the next scan should be able to recover automatically.

---

# 84. POWER LOSS

Unexpected power loss must not make the music library permanently unrecoverable.

---

# 85. DATABASE CORRUPTION

If the database is corrupted:

rebuild it from the music roots.

---

# 86. SCAN JOURNAL

A lightweight scan journal/state marker may be used if necessary.

Do not add complexity unless it solves a real recovery problem.

---

# 87. SCAN CANCELLATION

The user may cancel a long scan.

Cancellation should be cooperative.

---

# 88. CANCELLATION SAFETY

A cancelled scan must not:

- kill playback
- corrupt the database
- leave locks permanently held.

---

# 89. SCAN LOCKING

The library database must protect against simultaneous conflicting scan operations.

---

# 90. MULTIPLE SCANNERS

Only one authoritative scanner should modify a given library dataset at a time.

---

# 91. PLAYBACK CONCURRENCY

Playback may continue while scanning.

The scanner must not take locks that block:

audio playback.

---

# 92. DATABASE READS DURING PLAYBACK

UI/library reads may continue during playback.

They should be lightweight.

---

# 93. DATABASE WRITE PRIORITY

Large database writes should preferably occur:

- outside active user interactions
- without blocking playback
- in controlled batches.

---

# 94. SCAN NOTIFICATIONS

The UI may receive:

SCAN_STARTED

SCAN_PROGRESS

SCAN_COMPLETED

SCAN_CANCELLED

SCAN_ERROR.

---

# 95. UI RESPONSE

The UI can display:

Scanning library...

without exposing internal scanner implementation details.

---

# 96. BACKGROUND SCAN

The scanner should be able to run after boot without requiring the user to remain on a library screen.

---

# 97. BOOT SCAN

At startup:

load existing library cache quickly.

Then:

perform incremental reconciliation in the background.

---

# 98. FAST STARTUP

Do not require a complete filesystem scan before showing the home screen.

---

# 99. STALE CACHE

A stale cache may be displayed temporarily if clearly treated as cached data.

The scanner then reconciles it.

---

# 100. PLAYBACK DURING SCAN

The user must be able to start playback while the library is still being scanned if enough metadata is available.

---

# 101. SCAN AND AUDIO

The scanner must never:

- decode FLAC audio unnecessarily
- read entire FLAC files merely to build metadata
- compete aggressively with the decoder.

---

# 102. METADATA-ONLY ACCESS

The library scanner should use metadata-level reads whenever possible.

---

# 103. FULL FILE READ

Reading the entire FLAC merely to obtain tags is prohibited unless required by the parser/library implementation.

---

# 104. ARTWORK DURING SCAN

Artwork extraction should be deferred or bounded.

Do not decode every high-resolution cover immediately during the initial scan.

---

# 105. ARTWORK LAZY LOADING

Recommended:

discover artwork during scanning

but:

decode artwork only when required.

---

# 106. LIBRARY THUMBNAILS

If album thumbnails are generated:

use a bounded thumbnail cache.

---

# 107. DUPLICATES

Duplicate tracks must be handled deterministically.

---

# 108. PATH DUPLICATE

The same path should never create two active library records.

---

# 109. CONTENT DUPLICATE

Two different paths containing identical audio may remain separate tracks.

Do not silently merge them without an explicit duplicate policy.

---

# 110. DUPLICATE DISPLAY

The UI may indicate duplicate tracks if desired.

This is not required for basic playback.

---

# 111. INVALID FLAC

A file that cannot be validated as FLAC should not enter the playable library.

Record a scan warning.

---

# 112. CORRUPTED FLAC

A partially corrupted FLAC may pass initial metadata parsing but fail during decoding.

The library should distinguish:

metadata-valid

from:

playback-valid.

---

# 113. DECODER VALIDATION

Do not perform expensive full decoding of every track during every scan.

---

# 114. OPTIONAL VERIFY

A separate:

VERIFY LIBRARY

operation may later perform deeper validation.

It should not be part of normal scanning.

---

# 115. ERROR CATEGORIES

Scanner errors should distinguish:

PATH_ERROR

PERMISSION_ERROR

METADATA_ERROR

INVALID_FORMAT

CACHE_ERROR

DATABASE_ERROR

UNKNOWN_ERROR.

---

# 116. ERROR HANDLING

An error affecting one file should not automatically abort the entire scan.

---

# 117. ERROR LOGGING

Log enough information to identify:

- path
- operation
- error category
- relevant parser error.

Do not flood normal logs with repeated identical failures.

---

# 118. RETRY POLICY

Transient filesystem errors may be retried a limited number of times.

Permanent metadata errors should not be retried endlessly.

---

# 119. RETRY LIMIT

Every retry path must have a finite limit.

---

# 120. REMOVABLE MEDIA

If music storage is removable:

mount availability should be represented explicitly.

---

# 121. STORAGE STATE

Recommended conceptual state:

AVAILABLE

UNAVAILABLE

SCANNING

CHANGED

ERROR.

---

# 122. MULTIPLE ROOTS

If multiple music roots are configured:

each root should have an independent scan state.

---

# 123. ROOT IDENTITY

Each root should have a stable internal identifier.

---

# 124. ROOT REMOVAL

Removing a configured root should not accidentally delete unrelated roots.

---

# 125. ROOT RECONFIGURATION

Changing music-root configuration should trigger reconciliation.

---

# 126. DATABASE CLEANUP

Stale records from permanently removed roots may eventually be deleted.

However:

active playback objects must remain safe.

---

# 127. HISTORY

Playback history should not necessarily be destroyed when a track becomes unavailable.

History can retain:

track metadata snapshot

even if the source file no longer exists.

---

# 128. FAVORITES

Favorites should use:

stable track identity

rather than only path text where practical.

---

# 129. QUEUE REFERENCES

Queue entries should reference:

track_id

plus sufficient fallback information to handle a missing file.

---

# 130. TRACK DELETION

If a track disappears:

remove it from future library results.

Do not silently alter an already-running playback session.

---

# 131. RESCAN AFTER EXTERNAL EDIT

If a user edits tags externally:

a subsequent scan updates:

metadata

album grouping

artist grouping

sorting indexes.

---

# 132. METADATA CHANGE

A metadata-only change should not require re-reading the entire audio stream.

---

# 133. PATH CHANGE

A renamed/moved file should be reconciled according to available file identity information.

---

# 134. MOVE DETECTION

If reliable identity data is available:

a moved file may be recognized as the same logical track.

Otherwise:

remove old record

and:

add new record.

---

# 135. TRACK ID STABILITY

When a move is confidently detected:

preserve track_id.

---

# 136. UNCERTAIN IDENTITY

If identity is uncertain:

do not merge records aggressively.

False merges are worse than temporary duplicates.

---

# 137. DATABASE INDEXES

Useful indexes include:

- path
- track_id
- album_id
- artist_id
- album_artist_id
- genre
- availability
- modification time.

Only add indexes that provide measurable value.

---

# 138. STARTUP QUERY

The home screen should be able to retrieve:

recent tracks

favorites

currently relevant library information

without scanning the filesystem.

---

# 139. LIBRARY UI

The UI should query the library service.

It should not directly walk:

/music

to populate screens.

---

# 140. ARCHITECTURAL SEPARATION

Filesystem scanner:

discovers files.

Metadata parser:

interprets metadata.

Library database:

stores indexed information.

Library service:

provides application queries.

UI:

renders results.

---

# 141. NO UI FILESYSTEM SCANNING

A screen must never directly execute a recursive filesystem scan as part of rendering.

---

# 142. NO BLOCKING SCAN

A user opening:

Music Library

must not freeze the entire UI while the filesystem is scanned.

---

# 143. CACHE-FIRST LIBRARY

Library screen behavior should be:

load cached results

→ display

→ receive scan updates.

---

# 144. EVENTUAL CONSISTENCY

During background scanning:

the displayed library may temporarily represent the previous scan state.

It must converge to the current filesystem state.

---

# 145. CONSISTENCY BOUNDARY

Once a scan completes:

the library database should represent the scanner's discovered filesystem state.

---

# 146. SCAN GENERATION

Each scan should have a:

scan_generation.

This helps identify stale scan events.

---

# 147. STALE SCAN EVENTS

If a newer scan begins:

events from an older scan should not overwrite newer state.

---

# 148. ACCEPTANCE TEST: INITIAL SCAN

Insert a new music directory.

Run the initial scan.

Verify:

all valid FLAC files appear.

---

# 149. ACCEPTANCE TEST: INCREMENTAL

Run another scan without changing anything.

Verify:

unchanged files are not unnecessarily reparsed.

---

# 150. ACCEPTANCE TEST: NEW FILE

Add a FLAC.

Run incremental scan.

Verify:

the new track appears.

---

# 151. ACCEPTANCE TEST: REMOVED FILE

Remove a FLAC.

Run incremental scan.

Verify:

it becomes unavailable or is removed according to policy.

---

# 152. ACCEPTANCE TEST: TAG CHANGE

Change a track's title.

Run incremental scan.

Verify:

the new title appears.

---

# 153. ACCEPTANCE TEST: RENAMED FILE

Rename a track.

Run reconciliation.

Verify:

the library does not contain two active copies unless identity cannot be determined.

---

# 154. ACCEPTANCE TEST: UNAVAILABLE ROOT

Remove/unmount the music storage.

Verify:

the library does not interpret the situation as immediate permanent deletion.

---

# 155. ACCEPTANCE TEST: ROOT RETURN

Restore the music storage.

Verify:

the library reconciles successfully.

---

# 156. ACCEPTANCE TEST: PLAYBACK DURING SCAN

Start playback.

Start a library scan.

Verify:

audio remains stable.

---

# 157. ACCEPTANCE TEST: LARGE LIBRARY

Test with a large collection.

Verify:

memory usage remains bounded.

---

# 158. ACCEPTANCE TEST: MALFORMED FILE

Add a file named:

bad.flac

that is not valid FLAC.

Verify:

scanner reports an error and continues.

---

# 159. ACCEPTANCE TEST: DATABASE FAILURE

Simulate database corruption.

Verify:

the library can be rebuilt.

---

# 160. ACCEPTANCE TEST: CANCEL

Start a long scan.

Cancel it.

Verify:

database remains usable.

---

# 161. ACCEPTANCE CRITERIA

The library architecture is production-ready when:

- valid FLAC files are discovered reliably
- invalid files do not crash scanning
- directory traversal is deterministic
- symlink behavior is explicit
- permission errors are isolated
- incremental scanning avoids unnecessary parsing
- full scanning can rebuild the library
- removed files are handled safely
- unavailable storage is handled safely
- metadata changes are detected
- album/artist grouping is deterministic
- multi-disc albums sort correctly
- library results are queryable without filesystem traversal
- UI never performs blocking scans
- scanning never blocks audio output
- database corruption is recoverable
- scan cancellation is safe
- large libraries remain memory-bounded
- startup does not require a complete scan
- offline operation requires no external service.

---

# 162. AI CODING AGENT RULES

The coding agent MUST:

1. Treat the filesystem as the authoritative music source.
2. Treat the library database as derived data.
3. Support full and incremental scans.
4. Never require a network connection for library construction.
5. Discover FLAC files recursively.
6. Use deterministic sorting.
7. Define explicit symlink behavior.
8. Protect against filesystem traversal cycles.
9. Ignore unsupported special files.
10. Handle permission errors per path.
11. Validate files rather than trusting extensions.
12. Avoid full-file reads when metadata-only access is sufficient.
13. Avoid full decoding during normal scanning.
14. Cache unchanged metadata.
15. Detect file changes using file identity.
16. Handle missing files without crashing.
17. Handle unavailable music roots explicitly.
18. Support removable/swappable storage.
19. Keep scanning outside the audio output thread.
20. Keep scanning outside the UI rendering thread.
21. Never let scanning interrupt active playback.
22. Use database transactions for batch updates.
23. Avoid excessive SD-card writes.
24. Make interrupted scans recoverable.
25. Make database corruption recoverable.
26. Keep library state separate from playback queue state.
27. Keep active playback snapshots stable during rescans.
28. Use generation IDs to reject stale scan events.
29. Keep UI access through a library service/API rather than direct filesystem traversal.
30. Keep memory usage bounded for large libraries.

---

# 163. FINAL LIBRARY ARCHITECTURE

The intended architecture is:

                         MUSIC ROOT
                             │
                             ▼
                    +------------------+
                    | FILESYSTEM       |
                    | SCANNER          |
                    +------------------+
                             │
                             ▼
                    DISCOVERED FLAC FILES
                             │
                             ▼
                    +------------------+
                    | METADATA PARSER  |
                    +------------------+
                             │
                             ▼
                    +------------------+
                    | NORMALIZER       |
                    +------------------+
                             │
                             ▼
                    +------------------+
                    | LIBRARY DATABASE |
                    +------------------+
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
          TRACKS          ALBUMS         ARTISTS
              │              │              │
              └──────────────┼──────────────┘
                             ▼
                     LIBRARY SERVICE
                             │
                             ▼
                             UI


Playback remains separate:

                    LIBRARY SERVICE
                           │
                           ▼
                       TRACK ID
                           │
                           ▼
                    PLAYBACK ENGINE
                           │
                           ▼
                       FLAC FILE
                           │
                           ▼
                        DECODER
                           │
                           ▼
                          PCM
                           │
                           ▼
                    TANCHJIM BUNNY DSP


The central invariant is:

THE FILESYSTEM IS AUTHORITATIVE; THE LIBRARY IS REBUILDABLE DERIVED DATA.

The second invariant is:

A LIBRARY SCAN MUST NEVER BLOCK OR STARVE THE AUDIO OUTPUT PATH.

The third invariant is:

THE UI MUST QUERY THE LIBRARY SERVICE, NOT PERFORM ITS OWN FILESYSTEM SCANS.

The fourth invariant is:

REMOVING OR TEMPORARILY UNMOUNTING MUSIC STORAGE MUST NOT IMMEDIATELY DESTROY THE LOGICAL LIBRARY STATE.

The fifth invariant is:

LIBRARY RESCANS MUST NEVER SILENTLY MODIFY THE ACTIVE PLAYBACK QUEUE OR INTERRUPT CURRENT AUDIO.

Final policy:

Scan asynchronously.

Cache aggressively but safely.

Treat files as authoritative.

Keep the database rebuildable.

Keep removable storage safe.

Keep metadata parsing separate.

Keep library queries lightweight.

Keep UI independent from filesystem traversal.

Keep audio completely independent from library scan timing.

And make the entire library capable of recovering from missing files, corrupted databases, interrupted scans, malformed metadata, and temporary storage disappearance without crashing FLACHEAD.