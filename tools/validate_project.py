from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CPP = ROOT / "src" / "VHS6RIFT.cpp"
PIPL = ROOT / "resources" / "VHS6RIFTPiPL.r"
CMAKE = ROOT / "CMakeLists.txt"


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    sys.exit(1)

for path in (CPP, PIPL, CMAKE, ROOT / ".github/workflows/build.yml", ROOT / "README.md"):
    if not path.exists():
        fail(f"required file missing: {path.relative_to(ROOT)}")

source = CPP.read_text(encoding="utf-8")
pipl = PIPL.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")

# Ensure parameter enum and ParamsSetup additions stay in exact registration order.
enum_match = re.search(r"// Stable parameter IDs.*?enum\s*\{(.*?)\n\};", source, re.S)
if not enum_match:
    fail("parameter enum not found")
enum_ids = [piece.strip().split("=")[0].strip() for piece in enum_match.group(1).split(",") if piece.strip()]
params_match = re.search(r"static PF_Err ParamsSetup\(.*?\n\}\n\nstatic inline float clamp01", source, re.S)
if not params_match:
    fail("ParamsSetup block not found")
params_block = params_match.group(0)
ops = []
for match in re.finditer(r"(PF_ADD_\w+|PF_END_TOPIC)\((.*?)\);", params_block, re.S):
    ids = re.findall(r"P_[A-Z0-9_]+", match.group(2))
    if ids:
        ops.append(ids[-1])
expected = enum_ids[1:-1]  # param 0 is the implicit input; P_COUNT is not a parameter
if ops != expected:
    for i, (a, b) in enumerate(zip(expected, ops)):
        if a != b:
            fail(f"parameter registration order mismatch at slot {i + 1}: enum={a}, setup={b}")
    fail(f"parameter count mismatch: enum={len(expected)}, setup={len(ops)}")

required_controls = [
    "VHS Camcorder / Reference Profile", "Fxc6 Character", "Gen 7 Character", "JVC GR-C7U Character",
    "VHS-R20 Character", "Fall-78 Character", "ERC-V2S Character", "JVC GR-SXM740U Character",
    "Continuous VHS Treatment", "Event Glitches", "Random Seed", "Colored Grain Amount",
    "Tape Type", "Overlay Layer (optional)", "Other VHS Engine", "VHS Craft Reference Controls",
    "NTSC-RS Reference Controls", "Signal Reference Controls", "Audio Glitch Enable",
    "Audio Event Glitches", "Audio Wet / Dry Mix", "PF_Cmd_AUDIO_RENDER",
    "Enable Grain / Noise", "Enable Tracking Damage", "Enable Tape Effects",
    "Enable Overlay Layer", "Enable Other Signal Engines", "Enable CRT TV Engine",
    "Procedural Transparent Tape", "Moving Tape Sweep", "CRT Scanline Strength"
]
for item in required_controls:
    if item not in source:
        fail(f"expected control/selector not found: {item}")

# All non-topic controls must be referenced outside enum/setup; topics are UI-only containers.
for param_id in enum_ids:
    if param_id == "P_INPUT" or param_id == "P_COUNT" or "TOPIC" in param_id or param_id.endswith("_END"):
        continue
    if len(re.findall(r"\b" + re.escape(param_id) + r"\b", source)) < 3:
        fail(f"parameter appears unused or unregistered: {param_id}")

if 'AE_Effect_Match_Name { "com.vhs6rift.effect" }' not in pipl:
    fail("PiPL match name changed")
if 'Name { "VHS6RIFT" }' not in pipl:
    fail("PiPL plugin name changed")
if "PF_OutFlag_AUDIO_EFFECT_TOO | PF_OutFlag_AUDIO_FLOAT_ONLY" not in source:
    fail("audio effect flags missing from GlobalSetup")
if "AE_Effect_Global_OutFlags { 0x48000006 }" not in pipl:
    fail("PiPL global flags do not match the audio-capable source flags")
if "PF_VERSION(1, 2, 0, PF_Stage_RELEASE, 1)" not in source:
    fail("compiled effect version is not the agreed 1.2 version")
if "AE_Effect_Version { 591361 }" not in pipl:
    fail("PiPL version is not the agreed 1.2 version")
if '#include "entry.h"' in source:
    fail("missing entry.h dependency remains; this SDK snapshot does not contain entry.h")
if 'PF_ADD_FLOAT_SLIDERX("Master VHS Strength", 0, 2000, 0, 2000, 350' not in source:
    fail("Master VHS Strength range/default is not configured as expected")
if 'PF_ADD_CHECKBOX("Enable CRT TV Engine", "Enable", FALSE' not in source:
    fail("CRT engine must be optional and disabled by default")
if 'SUFFIX ".aex"' not in cmake or 'OUTPUT_NAME "VHS6RIFT"' not in cmake:
    fail("CMake target is not configured to emit VHS6RIFT.aex")
if "PLACE_SOURCE_HERE" in source:
    fail("source placeholder remains")

print(f"PASS: {len(expected)} registered parameters are ordered consistently.")
print("PASS: camera, signal, tape, overlay, Other VHS and audio controls are present.")
print("PASS: PiPL identity and audio-capable outflags match the source.")
print("NOTE: this is a static source check only; it does not replace a Windows SDK build or AE runtime tests.")
