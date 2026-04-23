# Changelog

All notable changes to FlippCoin are documented in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.5.0] — 2026-04-22

Thorough layout refinement pass. Every screen audited pixel-by-pixel
against the Flipper Zero's 128×64 canvas and font metrics.

### Fixed
- **Main screen** — title and separator now hide during animation.
  Previously the coin at apex (cy=13 with artwork extending to y=2)
  would overlap the "FlippCoin" title text at y=1..10.
- **Menu screen** — dropped the version footer. With 5 items at 8px
  spacing, item 5 at y=59 was overlapping the footer at y=54..62 by
  up to 6 pixels. Version is visible on the About screen.
- **Stats screen** — tightened vertical spacing; sparkline reduced
  to 2px bars. The old 3px bars at baseline y=47 collided with the
  "Tails" line bottom. New layout: Total y=27, Heads y=35, Tails y=43,
  sparkline y=48, Streak y=57. Empty state now shows helpful hint.
- **Achievements screen** — unlock count moved into the title
  ("Achievements N/M") so the footer could be removed. The footer
  was overlapping item 6 by 5 pixels. Detail text switched to column
  layout with short goals (1, 10, 100, 1000, 5 row, 10 row) so even
  "Grand Master 1000" fits within the 120px inner frame.
- **Settings screen** — repositioned items to y=28/38/48 with footer
  at y=58. Previous y=30/40/50 had the Reset item overlapping the
  footer.
- **About screen** — moved the github URL up to y=52 AlignTop. It
  was at y=54, putting its bottom at y=62 — outside the inner frame
  at y=59.
- **draw_menu_item** alignment bug — label (baseline y) and value
  (AlignBottom y) were vertically offset by 2px. Value now uses
  `canvas_string_width` to right-align at the same baseline as the
  label.

---

## [3.4.0] — 2026-04-22

### Changed
- **Heads artwork redesigned**: taller crown, stronger nose protrusion
  (max at `cx-7`), defined jaw with chin-separator shading, and two
  subtle hair-texture carves suggesting waves. Laurel wreath reorganized
  into paired V-leaf marks along the back-of-head rim instead of loose
  dots — now reads as actual foliage.
- **Tails artwork redesigned**: added eye dot on the eagle head, sharper
  distinction between olive-branch cluster (3 leaves with proper cluster
  shape) and arrow talon (shaft + arrowhead + fletching), and a cleaner
  4-stripe heraldic shield.

### Added
- **Milled rim texture** — 12 tiny radial dots between the outer border
  (r=13) and inner decorative ring (r=11) simulating the "reeding"
  texture on real coin edges. Subtle but gives the coin tactile
  authenticity.
- **Landing gleam** — the diagonal highlight sweep now fires immediately
  when the coin lands (by resetting the gleam-phase tick), so every
  result gets a visual shine as it's revealed instead of waiting for
  the next periodic sweep.

---

## [3.3.0] — 2026-04-22

### Added
- **Flip history** (last 32 flips) stored as a packed `uint32_t` bitmap
  alongside an 8-bit count. Newest flip in bit 0, oldest in the highest
  valid bit. Persists across restarts.
- **Sparkline chart** on the stats screen — bars above the baseline are
  heads, bars below are tails, newest on the right. Shows ratio AND
  temporal streaks at a glance.

### Changed
- Replaced the aggregate ratio bar with the sparkline (the sparkline
  shows distribution visually AND reveals patterns the bar hid).
- Save format bumped to v3. v1 and v2 files load cleanly — the new
  `history_count` and `history` fields live where `_pad` and
  `_reserved[4]` used to, both of which were always zeroed, so old
  saves naturally map to empty-history under v3's layout.

---

## [3.2.0] — 2026-04-22

### Added
- **Achievements system** with 6 milestones:
  First Flip · Ten Count (10) · Centurion (100) · Grand Master (1000) ·
  Hot Streak (5 in a row) · Impossible (10 in a row)
- **Achievements screen** accessible from the main menu, showing unlock
  status (filled/empty checkbox) and a `N / 6 unlocked` progress footer
- **Toast banner system** replaces the one-off NEW BEST! flash — any
  milestone (achievement unlock OR new best streak) surfaces a labelled
  banner for ~1.5s with a synchronized green-LED pulse
- On-disk save format bumped to v2 (adds `achievements` bitmask).
  v1 saves auto-migrate cleanly; achievements start at zero

### Changed
- Main menu now has 5 items (Achievements inserted between Statistics
  and Settings). Menu spacing tightened to 7px to fit

---

## [3.1.0] — 2026-04-22

### Added
- **Auto-flip mode** — press `Right` on the main screen to run 10 rapid
  flips in a row. Progress shown as `auto N/10` during the sequence.
  `Back` cancels the remaining flips. Great for statistical experiments.
- **New-best-streak celebration** — when a streak of 3+ beats your previous
  best, a "NEW BEST!" banner flashes for ~1.5 seconds and the green LED
  pulses via `sequence_success`.

### Changed
- Main-screen idle prompt now shows "OK=flip   Right=auto" to surface the
  new shortcut.

### Fixed
- **CI workflow** now pins `flipperdevices/flipperzero-ufbt-action@v0.1.4`
  (previously `@v0` which doesn't exist) and uses the action's `fap-dir`
  output for the artifact path. Builds are now green on every push.

---

## [3.0.0] — 2026-04-20

Major architectural rework. The app has been rebuilt from a single C file
into a modular project with persistence, sound, and a full menu system.

### Added
- **Main menu** at launch with Flip / Statistics / Settings / About
- **Persistent storage** — stats and settings survive restarts, saved to
  `/ext/apps_data/flipp_coin/state.bin`
- **Sound effects** via the Flipper Zero speaker — menu clicks, flip launch,
  landing thud, and distinct rising/falling chirps for heads/tails reveal
- **Settings screen** — toggle haptic on/off, toggle sound on/off, reset
  all stats (with confirmation dialog)
- **About screen** with version, author, and GitHub link
- **Reset confirmation dialog** prevents accidental stat wipes
- **Redesigned coin artwork** — bolder Liberty-style profile portrait with
  carved eye/mouth/nostril details and laurel leaf marks on the rim; more
  detailed heraldic eagle with horizontal-banded shield, radiating primary
  wing feathers with articulation tips, fanned tail, olive branch in left
  talon, and arrows in right talon

### Changed
- **Source split** across 7 files by concern:
  `app.h` · `animation.c/h` · `faces.c/h` · `render.c/h`
  · `persistence.c/h` · `sound.c/h` · `coin_flip.c`
- **State machine expanded** from 3 to 7 states with proper transition helpers
- **Navigation** — `Up` from the main screen now opens the menu (was: stats)

### Fixed
- Malloc return value now checked on startup (returns error instead of crashing)

---

## [2.1.0] — 2026-04-20

### Added
- Drop shadow appears under the coin during flight, scales with height
- 3D edge-on rendering during rotation — two parallel rim lines with caps
  and a subtle highlight, replacing the previous flat bar
- Partial face-ring hint on intermediate rotation frames
- Ratio bar in the stats screen showing heads/tails proportion with midpoint
  reference tick
- Animated "flipping..." text cycles during the animation

### Changed
- Replaced history dots on the stats screen with the ratio bar

---

## [2.0.0] — 2026-04-20

Premium visual rework.

### Added
- Physics-based 32-frame flip animation (parabolic arc + bounce)
- Hand-drawn pixel art coin faces: Liberty-style profile (heads) and
  spread-wing eagle (tails)
- 8-particle landing effect with gravity
- Impact sparkle lines on landing
- Idle gleam — diagonal highlight sweeps across the coin every ~3 seconds
- Statistics tracking: totals, percentages, current streak, best streak
- Flip history ring buffer (20 entries)
- Single-tick haptic feedback on landing (motor-friendly)

---

## [1.0.0] — 2026-04-12

Initial release. Basic coin flip with hardware RNG and running H/T counter.
