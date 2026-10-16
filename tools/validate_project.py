from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CPP = ROOT / "src" / "VHS6RIFT.cpp"
HEADER = ROOT / "include" / "VHS6RIFT.h"
PIPL = ROOT / "resources" / "VHS6RIFTPiPL.r"
CMAKE = ROOT / "CMakeLists.txt"


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    sys.exit(1)

for path in (CPP, HEADER, PIPL, CMAKE, ROOT / ".github/workflows/build.yml", ROOT / "README.md"):
    if not path.exists():
        fail(f"required file missing: {path.relative_to(ROOT)}")

source = CPP.read_text(encoding="utf-8")
header = HEADER.read_text(encoding="utf-8")
pipl = PIPL.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")

# Clean comments from header before parsing enum
clean_header = re.sub(r"//.*?\n", "\n", header)
enum_match = re.search(r"enum\s*\{(.*?VHS_NUM_PARAMS\s*)\};", clean_header, re.S)
if not enum_match:
    fail("parameter enum not found in VHS6RIFT.h")

enum_ids = [piece.strip().split("=")[0].strip() for piece in enum_match.group(1).split(",") if piece.strip()]

params_match = re.search(r"static PF_Err ParamsSetup\(.*?\n\}\n\nstatic inline float clamp01", source, re.S)
if not params_match:
    fail("ParamsSetup block not found")
params_block = params_match.group(0)
ops = []
for match in re.finditer(r"(PF_ADD_\w+|PF_END_TOPIC)\((.*?)\);", params_block, re.S):
    ids = re.findall(r"VHS_[A-Z0-9_]+", match.group(2))
    if ids:
        ops.append(ids[-1])
expected = enum_ids[1:-1]  # param 0 is the implicit input; VHS_NUM_PARAMS is not a parameter
if ops != expected:
    for i, (a, b) in enumerate(zip(expected, ops)):
        if i < len(ops) and a != ops[i]:
            fail(f"parameter registration order mismatch at slot {i + 1}: enum={a}, setup={ops[i]}")
    fail(f"parameter count mismatch: enum={len(expected)}, setup={len(ops)}")

required_controls = [
    "1. Camera Engines", "2. VHS Degradation Engine", "3. Bad Signal / Glitch Engine",
    "4. CRT TV Engine", "5. Grain Engine", "6. Dates & Text OSD", "7. Advanced Overlays & Audio",
    "PF_Cmd_AUDIO_RENDER"
]
for item in required_controls:
    if item not in source:
        fail(f"expected control/selector not found: {item}")

# All non-topic controls must be referenced outside enum/setup; topics are UI-only containers.
for param_id in enum_ids:
    if param_id == "VHS_INPUT" or param_id == "VHS_NUM_PARAMS" or "TOPIC" in param_id or param_id.endswith("_END"):
        continue
    if len(re.findall(r"\b" + re.escape(param_id) + r"\b", source)) < 2:
        fail(f"parameter appears unused or unregistered: {param_id}")

if 'AE_Effect_Match_Name { "com.vhsmaster.effect" }' not in pipl:
    fail("PiPL match name does not match com.vhsmaster.effect")

if "PF_OutFlag_AUDIO_EFFECT_TOO | PF_OutFlag_AUDIO_FLOAT_ONLY" not in source:
    fail("audio effect flags missing from GlobalSetup")
if "PF_VERSION(1, 2, 0, PF_Stage_RELEASE, 1)" not in source:
    fail("compiled effect version is not the agreed 1.2 version")

print(f"PASS: {len(expected)} registered parameters are ordered consistently across VHS6RIFT.h and VHS6RIFT.cpp.")
print("PASS: Camera Engines, VHS Engine, Bad Signal/Glitch, CRT TV, Grain, Dates & Text, and Advanced groups are present.")
print("PASS: PiPL identity ('com.vhsmaster.effect') and audio-capable outflags match the source.")
