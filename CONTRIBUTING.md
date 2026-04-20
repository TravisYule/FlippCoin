# Contributing to FlippCoin

Thanks for your interest. This doc covers the basics.

## Ground rules

- **Scope**: this is a focused coin flip app. Feature PRs for things that
  aren't related to flipping a coin (e.g., dice, multi-coin, games) will be
  politely declined — feel free to fork.
- **Be kind**: normal OSS etiquette. No harassment, no spam.
- **License**: by contributing, you agree your code is released under the
  project's [MIT license](LICENSE).

## Before you open a PR

1. **Open an issue first** for anything larger than a small fix, so we can
   align on the approach before you invest in code.
2. **Build it locally**: `ufbt` should compile cleanly with no warnings
   (we treat warnings as errors in CI).
3. **Test on a real Flipper Zero**: the 1-bit display has quirks that
   don't show up in static analysis.
4. **Keep diffs focused**: one concern per PR. Refactors separate from
   features separate from bug fixes.

## Style

- C99, follow the existing formatting (4-space indent, K&R braces)
- `static` everything that doesn't need external linkage
- Prefer fixed-size integers (`uint8_t`, `int16_t`) over `int`/`unsigned`
- Header comments on each module explaining *what* it owns and *why*
- Favor readability over cleverness — this is a tiny app on a tiny screen

## Architecture notes

The app is split by concern:

| Module         | Owns                                                 |
|----------------|------------------------------------------------------|
| `app.h`        | Shared types, App state struct                       |
| `animation.*`  | Keyframe tables, particles, shadow, sparkles         |
| `faces.*`      | Coin face pixel art + coin body rendering            |
| `render.*`     | All screen rendering (read-only on state)            |
| `persistence.*`| Load/save to SD card                                 |
| `sound.*`      | Speaker-driven audio effects                         |
| `coin_flip.c`  | Main entry, event loop, state machine dispatch       |

If a change crosses more than 2-3 module boundaries, it probably belongs
broken up into smaller PRs.

## Testing

- Run the app and flip a few hundred times
- Check all menu transitions work with both OK and Back
- Verify stats persist across app restarts
- Toggle haptic and sound in settings, confirm they take effect
- Reset stats and confirm the reset confirmation dialog works both ways

## Reporting security issues

If you find a way for FlippCoin to misbehave with hardware (e.g., crash
the firmware, exhaust resources, corrupt SD card data), please do **not**
open a public issue. Email the maintainer first.
