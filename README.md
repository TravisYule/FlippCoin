# FlippCoin

A polished coin flip app for the Flipper Zero, using the hardware RNG for true randomness.

## Features

- **Physics-based animation** — 32-frame parabolic arc with 3 full rotations, landing bounce, and impact sparkles
- **Original pixel art** — filled silhouette profile portrait (heads) and spread-wing eagle (tails), hand-tuned for the 128×64 display
- **Particle system** — 8 particles spawn on landing with gravity and randomized velocity
- **Gentle haptic feedback** — single short tick on landing, motor-friendly
- **Idle gleam** — subtle diagonal highlight sweeps across the coin face every few seconds
- **Statistics tracking** — total flips, heads/tails percentages, current streak, best streak
- **Flip history** — last 20 results shown as filled/outline squares on the stats screen

## Controls

| Button | Action |
|--------|--------|
| **OK** | Flip the coin |
| **Up** | Open statistics screen |
| **Left** | Reset all stats and history |
| **Back** | Exit app (or return from stats / cancel flip) |

## Install (prebuilt)

1. Grab `flipp_coin.fap` from the [Releases](../../releases) page
2. Copy it to your Flipper Zero SD card at `apps/Games/`
3. On the Flipper: **Apps → Games → FlippCoin**

## Build from source

Requires [`ufbt`](https://github.com/flipperdevices/flipperzero-ufbt) (micro Flipper Build Tool):

```bash
pip install ufbt
cd FlippCoin
ufbt
```

The compiled `.fap` lands in `dist/flipp_coin.fap`.

## Project layout

```
FlippCoin/
├── application.fam    # App manifest
├── coin_flip.c        # Everything: state machine, animation, rendering
├── icon.png           # 10×10 1-bit app icon
├── LICENSE
└── README.md
```

## License

MIT — see [LICENSE](LICENSE).
