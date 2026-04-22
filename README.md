# FlippCoin

[![Build](https://github.com/TravisYule/FlippCoin/actions/workflows/build.yml/badge.svg)](https://github.com/TravisYule/FlippCoin/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/TravisYule/FlippCoin)](https://github.com/TravisYule/FlippCoin/releases/latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-FAP-FF6200)](https://flipperzero.one/)

A polished coin flip app for the [Flipper Zero](https://flipperzero.one/),
featuring physics-based animation, hand-drawn pixel art, sound effects,
persistent statistics, and a full menu system. Uses the device's hardware
RNG for true randomness on every flip.

---

## Screens

```
 ┌─────── FlippCoin ───────┐      ┌────── FlippCoin ───────┐
 │                         │      │                        │
 │    > Flip               │      │          ○             │      ┌── ~ STATISTICS ~ ───┐
 │      Statistics         │      │         / \            │      │                      │
 │      Settings           │      │        ▓▓▓▓▓           │      │  Total Flips:  42   │
 │      About              │      │         \_/            │      │  Heads: 23 (55%)    │
 │                         │      │                        │      │  Tails: 19 (45%)    │
 │         v3.0            │      │      > HEADS! <        │      │  H├████▓░░░░─░░░┤T  │
 │                         │      │ H:23 T:19       Up=Menu│      │  Streak:3H  Best:7T │
 └─────────────────────────┘      └────────────────────────┘      └──────────────────────┘
       Main Menu                       Flip Screen                    Statistics
```

## Features

### Visuals
- **Physics-based flip animation** — 32-frame parabolic arc with three full
  rotations and landing bounce, ~1 second at 30 FPS
- **Hand-drawn coin art** — Liberty-style profile (heads) and heraldic eagle
  with shield, wings, olive branch, and arrows (tails)
- **3D rotation** — edge-on frames show apparent coin thickness with paired
  rim lines and caps
- **Ground shadow** during flight that scales with coin height
- **Impact effects** — 8-particle landing spray + radiating sparkle lines
- **Idle gleam** — diagonal highlight sweeps across the resting coin

### Stats
- Persistent across app restarts (saved to SD card)
- Total flips, heads/tails with percentages
- Current and best streak tracking with side indicator
- Visual heads/tails ratio bar with 50% midpoint reference

### Audio & Haptics
- Distinct sounds for launch, landing, heads reveal (rising), tails reveal (falling)
- Menu navigation clicks and selection chirps
- Single gentle haptic tick on landing (motor-longevity friendly)
- Both toggleable in Settings

### UX
- Main menu launcher with four options
- Settings screen with toggle switches
- About screen with version and credits
- Confirmation dialog before clearing stats
- Seven-state machine with full Back-navigation support

## Controls

| Button    | Main Screen             | Menus              |
|:---------:|:------------------------|:-------------------|
| **OK**    | Flip coin               | Select / toggle    |
| **Right** | Auto-flip (10 in a row) | —                  |
| **Up**    | Open menu               | Navigate up        |
| **Down**  | —                       | Navigate down      |
| **Back**  | Open menu               | Back / cancel      |

## Install (prebuilt)

1. Download `flipp_coin.fap` from the [latest release](https://github.com/TravisYule/FlippCoin/releases/latest)
2. Copy it to your Flipper Zero SD card at `apps/Games/`
3. On the device: **Apps → Games → FlippCoin**

Stats are stored at `/ext/apps_data/flipp_coin/state.bin` and survive
app restarts and firmware updates.

## Build from source

Requires [`ufbt`](https://github.com/flipperdevices/flipperzero-ufbt)
(micro Flipper Build Tool), which handles SDK download automatically.

```bash
pip install ufbt
git clone https://github.com/TravisYule/FlippCoin
cd FlippCoin
ufbt
```

The compiled `.fap` lands in `dist/flipp_coin.fap`.

## Project structure

```
FlippCoin/
├── app.h              # Shared types, constants, App state struct
├── animation.c/h      # Keyframe tables + particles + shadow + sparkles
├── faces.c/h          # Coin face pixel art + full/narrow coin body
├── render.c/h         # All screens (menu/main/stats/settings/about)
├── persistence.c/h    # Save/load to SD card
├── sound.c/h          # Speaker tone effects
├── coin_flip.c        # Main entry, event loop, state machine dispatcher
├── application.fam    # App manifest
├── icon.png           # 10×10 1-bit menu icon
├── .github/
│   └── workflows/build.yml   # CI builds FAP on every push
├── LICENSE
├── README.md
└── CHANGELOG.md
```

## Architecture

Each module owns a single concern and exposes a narrow header:

- **animation** — precomputed 32-frame keyframe tables (Y offset, half-width,
  face-visible flag, haptic triggers) plus the particle physics + visual
  effects (shadow, sparkles)
- **faces** — pixel-art `draw_heads` / `draw_tails` plus `draw_coin_full` /
  `draw_coin_narrow` for body rendering
- **render** — one function per screen (menu/main/stats/settings/about/
  reset-confirm). Read-only on app state
- **persistence** — fixed-size binary blob with magic + version header for
  forward compatibility; silent degradation on corrupt/missing files
- **sound** — blocking short tones (<30ms each), no-ops when disabled or
  speaker is busy
- **coin_flip** — event loop, input dispatch per state, animation ticking

`app.h` defines the `App` struct that every module reads via pointer. A
`FuriMutex` protects it between the render/input callbacks and the main
loop.

## Contributing

Issues and pull requests welcome. For substantial changes, open an issue
first so we can discuss. Keep additions focused — this is a coin flip app,
not a casino.

## License

MIT — see [LICENSE](LICENSE).
