# VHS_Legend

Native After Effects plugin reconstruction project targeting After Effects 23.2.1 on Windows x64 / Visual Studio 2022.

## Reference artifacts
The original surviving AEX is in `reference/`. The recovered `forgeVkLoader.dll` is in `reference/runtime/`.

## SDK
Place the licensed Adobe After Effects SDK in a private location and set `AE_SDK_ROOT` to it. Do not publish the SDK in a public Git repository.

## Build goal
Use `GEMINI_PROMPT.md` as the agent instructions. The agent must analyze first, reconstruct native source, build, read failures, fix them, and repeat until the AEX is produced or an external dependency is proven missing.

## Important
A binary reference cannot guarantee 100% source-level recovery. The repository is structured to maximize fidelity from the surviving binary and recovered runtime, not to promise identical output where the original proprietary source/servers/assets are unavailable.
