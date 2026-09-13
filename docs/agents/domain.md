# Domain Docs

How the engineering skills should consume this repo's domain documentation when exploring the codebase.

## Before exploring, read these

- **Context Documents**: the 7 planning docs (`00-project-overview.md` through `06-implementation-plan.md`) in the `Context Documents/` directory are the authoritative reference for project decisions, architecture, and current phase status. Read the relevant docs before working on any subsystem:
  - `00-project-overview.md` — project summary, what changed in the WiFi/Firebase pivot, guiding principles
  - `01-hardware-firmware.md` — ESP32 hardware, display, firmware architecture, WiFi/claiming flow
  - `02-hardware-ui.md` — design system, interaction model, screen tree, per-screen behavior
  - `03-web-app.md` — Firebase backend, auth model, API surface, Firestore schema
  - `04-workout-logging.md` — workout model, offline behavior, progress analysis
  - `05-music-control.md` — Spotify integration, Hype/Rest feature, album art handling
  - `06-implementation-plan.md` — current phase, task checklist, key decisions log
- **CLAUDE.md** at the repo root consolidates the most important rules and locked decisions from the Context Documents — treat it as the quick-reference summary, but go to the actual Context Documents when you need full detail on a subsystem.

If the Context Documents directory doesn't exist or files are missing, **proceed silently**. Don't flag their absence; don't suggest creating them upfront.

## File structure

Single-context repo (this repo):

```
/
├── CLAUDE.md                      ← Quick reference, points to Context Documents
├── Context Documents/             ← Authoritative planning docs (READ-ONLY)
│   ├── 00-project-overview.md
│   ├── 01-hardware-firmware.md
│   ├── 02-hardware-ui.md
│   ├── 03-web-app.md
│   ├── 04-workout-logging.md
│   ├── 05-music-control.md
│   └── 06-implementation-plan.md
├── Hardware/
│   ├── Firmware/                  ← ESP32/Arduino code goes here
│   ├── Lopaka Screens/            ← USER-AUTHORED TFT_eSPI code (READ-ONLY)
│   └── Unit Test/                 ← Hardware bring-up sketches (READ-ONLY reference)
└── Software/
    └── Web App/                   ← Firebase/web app code goes here
```

## Use the project's vocabulary

This project has a specific set of locked decisions and terminology documented in the Context Documents:

- **Hype/Rest** — the flagship feature (two dedicated buttons for music playlist switching with timers)
- **Guided Logging** — the encoder-driven workout set/rep entry UI
- **Home screen** — the default/idle screen, the only screen where Hype/Rest buttons trigger real behavior
- **Backend-proxy** — all Spotify calls go device → Firebase Cloud Functions → Spotify (never direct)
- **Offline-first** — device must work for entire workout session with zero network
- **Lopaka screens** — TFT_eSPI code exported from Lopaka tool, user-authored layouts (ground truth)
- **3-way switch** — Off / On / Lock hardware power switch

When your output names a concept, use the terms as defined in the Context Documents. Don't drift to synonyms or invent new terms for existing concepts.

## Locked decisions reference

CLAUDE.md section 4 ("Locked Decisions Reference") consolidates every decision marked "locked/confirmed/resolved" across all Context Documents. **Never re-propose or second-guess anything listed there.**

Key examples:
- Display is ST7789, 240×320, landscape, with 4 required TFT_eSPI settings
- Firmware uses Arduino-ESP32 core (not ESP-IDF native)
- All Spotify calls are backend-proxied (device never calls Spotify directly)
- Hype/Rest trigger only on Home screen; other screens treat those buttons as Back/Info
- Encoder weight/reps edit: ±2.5 lb or ±0.5 kg per detent, ±1 or ±0.5 rep
- WiFi provisioning: hybrid model (first-boot SoftAP, web-app-push for additional networks)
- Device claim via polling (`GET /device/claim/status`)

## Context Documents are read-only

The 7 planning docs in `Context Documents/` are reference material, not implementation destinations. Never edit them as part of implementation work. When you discover something that corrects or extends a locked decision:

1. Update `Context Documents/06-implementation-plan.md` in the "Key decisions & learnings log" section with a one-line summary + which doc it updates
2. Update the relevant source doc (e.g. `01-hardware-firmware.md`) with the full detail
3. Update CLAUDE.md section 4 if it's a newly-locked decision that should be in the quick reference

## Implementation destinations

- All ESP32/Arduino firmware code goes in `Hardware/Firmware/`
- All web app + Firebase backend code goes in `Software/Web App/`
- The other directories (`Context Documents/`, `Hardware/Lopaka Screens/`, `Hardware/Unit Test/`) are reference-only

## Flag conflicts with locked decisions

If your output contradicts a locked decision from the Context Documents or CLAUDE.md, surface it explicitly rather than silently overriding:

> _Contradicts the locked backend-proxy call shape in 01-hardware-firmware.md, but worth reopening because…_

When in doubt, re-read the actual source doc (named in CLAUDE.md section 4) rather than guessing.
