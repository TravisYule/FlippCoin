# Changelog

All notable changes to FlippCoin are documented in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
