from __future__ import annotations

import argparse
import json
import os
import re
from pathlib import Path
from typing import Any

import numpy as np
import pandas as pd


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SHEET_NAME = "hermaphrodite chemical"
SOURCE = "Cook2019_SI5"
BODY_WALL_PATTERN = re.compile(r"^([dv])BWM([LR])(\d+)$")
MOTOR_NEURON_PATTERN = re.compile(r"^(AS|DA|DB|DD|VA|VB|VC|VD)(\d+)$")

# motor-to-muscle projection v0 sign policy:
# - DD/VD body motor neurons are treated as inhibitory GABAergic outputs.
# - DA/DB/VA/VB/AS/VC/PDA/PDB/SAB are treated as excitatory/cholinergic outputs.
# - Other direct presynaptic neurons in Cook SI5 are kept and assigned +1 in v0.
# This is a coarse projection sign for MuJoCo coupling, not a final NMJ model.
INHIBITORY_CLASSES = {"DD", "VD"}
EXCITATORY_CLASSES = {"AS", "DA", "DB", "VA", "VB", "VC", "PDA", "PDB", "SABD", "SABVL", "SABVR"}

NON_NEURON_PATTERNS = [
    r"^pm\d",
    r"^mc\d",
    r"^e\d",
    r"^g\d",
    r"^g1p$",
    r"^bm$",
    r"^hyp$",
    r"^int$",
    r"^exc_",
    r"^hmc$",
    r"^GLR",
    r"^CEPsh",
    r"^(dBW|vBW|BW)",
    r"^(MDL|MDR|MVL|MVR)\d",
    r"^vm\d",
    r"^um\d",
    r"^mu_",
]


def default_source_root() -> Path:
    candidates: list[Path] = []
    env_value = os.environ.get("CELEGANS_NETWORK_SOURCE_ROOT")
    if env_value:
        candidates.append(Path(env_value))
    candidates.extend(
        [
            Path(r"E:\1 PhD work\C.elegans simulation\code\C.elegans.network"),
            Path(r"E:\1 PhD work\C.elegans simulation\code\draft\C.elegans.network"),
        ]
    )
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


def default_cook_si5() -> Path:
    return default_source_root() / "cook 2019 SI" / "41586_2019_1352_MOESM9_ESM SI5.xlsx"


def normalize_cell_name(name: Any) -> str:
    text = str(name).strip()
    match = MOTOR_NEURON_PATTERN.match(text)
    if match:
        return f"{match.group(1)}{int(match.group(2)):02d}"
    return text


def is_non_neuron(name: str) -> bool:
    return any(re.match(pattern, name) for pattern in NON_NEURON_PATTERNS)


def read_adj_matrix(excel_path: Path, sheet_name: str) -> pd.DataFrame:
    raw = pd.read_excel(excel_path, sheet_name=sheet_name, header=None)
    col_names = [normalize_cell_name(x) for x in raw.iloc[2, 3:].dropna().astype(str).to_list()]
    row_names = [normalize_cell_name(x) for x in raw.iloc[3:, 2].dropna().astype(str).to_list()]
    data = raw.iloc[3:, 3:]
    data = data.apply(pd.to_numeric, errors="coerce").fillna(0.0)
    data = data.iloc[: len(row_names), : len(col_names)]
    return pd.DataFrame(data.to_numpy(dtype=float), index=row_names, columns=col_names)


def muscle_channel(muscle_name: str) -> tuple[int, str, int]:
    match = BODY_WALL_PATTERN.match(muscle_name)
    if not match:
        raise ValueError(f"Not a body wall muscle name: {muscle_name}")
    dorsal_or_ventral, left_or_right, segment_text = match.groups()
    segment = int(segment_text)
    if segment < 1 or segment > 24:
        raise ValueError(f"Body wall muscle segment out of 1..24: {muscle_name}")

    if dorsal_or_ventral == "d" and left_or_right == "L":
        quadrant = "DL"
        muscle_index = segment - 1
    elif dorsal_or_ventral == "d" and left_or_right == "R":
        quadrant = "DR"
        muscle_index = 24 + segment - 1
    elif dorsal_or_ventral == "v" and left_or_right == "L":
        quadrant = "VL"
        muscle_index = 48 + segment - 1
    elif dorsal_or_ventral == "v" and left_or_right == "R":
        quadrant = "VR"
        muscle_index = 72 + segment - 1
    else:
        raise ValueError(f"Unexpected body wall muscle quadrant: {muscle_name}")
    return muscle_index, quadrant, segment


def neuron_class(name: str) -> str:
    match = re.match(r"^[A-Z]+", name)
    return match.group(0) if match else name


def sign_for_pre_neuron(name: str) -> tuple[int, str]:
    cls = neuron_class(name)
    if cls in INHIBITORY_CLASSES:
        return -1, "inhibitory_known_v0"
    if cls in EXCITATORY_CLASSES:
        return 1, "excitatory_known_v0"
    return 1, "unknown_default_excitatory_v0"


def expected_muscle_names() -> list[str]:
    names: list[str] = []
    for prefix in ["dBWML", "dBWMR", "vBWML", "vBWMR"]:
        for segment in range(1, 25):
            names.append(f"{prefix}{segment}")
    return names


def build_projection(cook_si5: Path) -> tuple[pd.DataFrame, dict[str, Any]]:
    adj = read_adj_matrix(cook_si5, SHEET_NAME)
    muscle_columns = [name for name in adj.columns if BODY_WALL_PATTERN.match(name)]
    expected_muscles = expected_muscle_names()
    present_muscles = set(muscle_columns)
    missing_muscles = [name for name in expected_muscles if name not in present_muscles]

    rows: list[dict[str, Any]] = []
    unknown_sign_classes: set[str] = set()
    muscles_with_edges: set[str] = set()
    for muscle_name in muscle_columns:
        muscle_index, quadrant, segment = muscle_channel(muscle_name)
        column = adj[muscle_name]
        for pre_neuron, weight in column[column > 0.0].items():
            pre_neuron = str(pre_neuron)
            if is_non_neuron(pre_neuron):
                continue
            sign, sign_policy = sign_for_pre_neuron(pre_neuron)
            if sign_policy == "unknown_default_excitatory_v0":
                unknown_sign_classes.add(neuron_class(pre_neuron))
            rows.append(
                {
                    "pre_neuron": pre_neuron,
                    "muscle_name": muscle_name,
                    "muscle_index": muscle_index,
                    "quadrant": quadrant,
                    "segment": segment,
                    "weight": float(weight),
                    "sign": sign,
                    "source": SOURCE,
                }
            )
            muscles_with_edges.add(muscle_name)

    projection = pd.DataFrame(
        rows,
        columns=["pre_neuron", "muscle_name", "muscle_index", "quadrant", "segment", "weight", "sign", "source"],
    )
    projection = projection.sort_values(["muscle_index", "pre_neuron", "muscle_name"]).reset_index(drop=True)

    no_edge_muscles = [name for name in expected_muscles if name not in muscles_with_edges]
    summary = {
        "projection": "motor-to-muscle projection v0",
        "source": SOURCE,
        "cook_si5": str(cook_si5),
        "sheet": SHEET_NAME,
        "rows": int(len(projection)),
        "unique_pre_neurons": int(projection["pre_neuron"].nunique()) if not projection.empty else 0,
        "present_body_wall_muscle_columns": int(len(muscle_columns)),
        "missing_body_wall_muscle_columns": missing_muscles,
        "channels_without_projection_edges": no_edge_muscles,
        "sign_policy": {
            "-1": "DD/VD inhibitory GABAergic motor neurons in v0",
            "+1_known": "DA/DB/VA/VB/AS/VC/PDA/PDB/SAB excitatory/cholinergic motor neurons in v0",
            "+1_unknown": "Other direct presynaptic neurons are retained and assigned +1 in v0; this is not a final NMJ model.",
        },
        "unknown_default_excitatory_classes": sorted(unknown_sign_classes),
    }
    return projection, summary


def validate_projection(projection: pd.DataFrame) -> None:
    if projection.empty:
        raise ValueError("Projection is empty")
    required = ["pre_neuron", "muscle_name", "muscle_index", "quadrant", "segment", "weight", "sign", "source"]
    if list(projection.columns) != required:
        raise ValueError(f"Unexpected columns: {list(projection.columns)}")
    if not projection["muscle_index"].between(0, 95).all():
        raise ValueError("muscle_index out of 0..95")
    if not projection["segment"].between(1, 24).all():
        raise ValueError("segment out of 1..24")
    if not projection["quadrant"].isin(["DL", "DR", "VL", "VR"]).all():
        raise ValueError("Unexpected quadrant")
    if not (projection["weight"] > 0.0).all():
        raise ValueError("All weights must be positive")
    if not projection["sign"].isin([-1, 1]).all():
        raise ValueError("sign must be -1 or +1")
    if not (projection["source"] == SOURCE).all():
        raise ValueError("Unexpected source value")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export motor-to-muscle projection v0 from Cook 2019 SI5.")
    parser.add_argument("--cook-si5", type=Path, default=default_cook_si5())
    parser.add_argument(
        "--output",
        type=Path,
        default=PROJECT_ROOT / "data" / "muscle" / "motor_to_muscle_projection_v0.csv",
    )
    parser.add_argument(
        "--summary-output",
        type=Path,
        default=PROJECT_ROOT / "data" / "muscle" / "motor_to_muscle_projection_v0_summary.json",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if not args.cook_si5.exists():
        raise FileNotFoundError(f"Cook SI5 file not found: {args.cook_si5}")

    projection, summary = build_projection(args.cook_si5)
    validate_projection(projection)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    projection.to_csv(args.output, index=False)
    args.summary_output.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    print(f"wrote {args.output}")
    print(f"rows={summary['rows']} unique_pre_neurons={summary['unique_pre_neurons']}")
    print(f"present_body_wall_muscle_columns={summary['present_body_wall_muscle_columns']}")
    print(f"missing_body_wall_muscle_columns={summary['missing_body_wall_muscle_columns']}")
    print(f"channels_without_projection_edges={summary['channels_without_projection_edges']}")
    print(f"unknown_default_excitatory_classes={summary['unknown_default_excitatory_classes']}")


if __name__ == "__main__":
    main()