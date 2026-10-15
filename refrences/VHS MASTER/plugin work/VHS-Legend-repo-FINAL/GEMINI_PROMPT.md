# Reconstruction + Build Instructions — VHS_Legend

You are reconstructing a native Adobe After Effects plugin from the surviving reference binary. The goal is the closest practical functional reproduction, not a mock effect.

## Inputs
- `reference/VHS LEGEND.aex` = surviving original plugin binary. Treat it as the authoritative behavior/reference artifact.
- `reference/runtime/forgeVkLoader.dll` = recovered runtime dependency observed with the original plugin. Do not replace it with a fake DLL.
- `sdk/` or `AE_SDK_ROOT` = licensed Adobe After Effects SDK supplied by the owner.

## Mandatory workflow
1. Inspect the reference AEX PE headers, exports, imports, resources/PiPL, strings, embedded assets, dynamic library loading, and version/compatibility information.
2. Inspect the recovered runtime DLL for exported/imported APIs and dependency paths relevant to the plugin.
3. Build an evidence map before implementing behavior. Preserve every discoverable effect name, match name, parameter, UI grouping, resource, shader asset, and processing mode.
4. Reconstruct native C++ source under `src/` using the Adobe AE SDK. Do NOT merely wrap, shell out to, load, or invoke the original AEX. Do NOT fake the effect with AE built-in effects.
5. Build on Windows x64 with Visual Studio 2022/MSVC and the supplied AE SDK.
6. If compilation fails: read the complete compiler/linker/resource/PiPL error, fix the source/project, and rebuild. Repeat automatically until it builds or a genuinely unavailable external dependency/tool is proven to be the blocker.
7. Verify the final artifact is a real `.aex`, x64, loadable by After Effects 23.2.1, with the expected plugin identity and effect entry points.
8. Never claim success without the generated `.aex` artifact and a recorded build log.

## Fidelity priority
1. Same plugin identity and effect registration.
2. Same controls/parameters and defaults.
3. Same UI grouping/order where discoverable.
4. Same rendering/processing behavior as far as can be recovered from the binary/assets.
5. Same animation/randomization behavior where discoverable.
6. Preserve recovered shader/resources instead of inventing replacements when technically possible.

## Dependency rule
Do not silently remove a dependency just to make compilation pass. If the original uses LaForge/ProductionCrate/Vulkan/CUDA or another runtime, document exactly which part is required. Reproduce the owner's own functionality without bypassing third-party licensing or DRM.

## Deliverables
- complete source under `src/`
- correct CMake/PiPL/resource configuration
- successful Release x64 `.aex`
- `BUILD_REPORT.md` documenting evidence, dependencies, build fixes, and remaining fidelity limitations
- build logs/artifacts
