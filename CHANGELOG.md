# Changelog

All notable changes to FlippCoin are documented in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
