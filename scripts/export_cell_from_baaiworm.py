from __future__ import annotations

import argparse
import csv
import json
import os
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]


def _default_diagnostics_dir() -> Path:
    candidates: list[Path] = []
    env_value = os.environ.get("CPP_WORM_VALIDATED_NEURON_DIAGNOSTICS")
    if env_value:
        candidates.append(Path(env_value))
    candidates.extend(
        [
            Path(r"E:\1 PhD work\C.elegans simulation\code\0522_sysTest\validated_neuron_layer\build\diagnostics"),
            Path(r"E:\1 PhD work\C.elegans simulation\code\draft\0522_sysTest\validated_neuron_layer\build\diagnostics"),
        ]
    )
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


DEFAULT_DIAGNOSTICS = _default_diagnostics_dir()
DEFAULT_CELLS = ["AIYL", "AVAL", "AWCL", "RIML", "VD05"]

FIELDS = [
    "idx",
    "parent_idx",
    "label",
    "section",
    "seg",
    "nseg",
    "area_um2",
    "C_m_pF",
    "g_L_nS",
    "e_L_mV",
    "g_C_nS",
    "gbnca_nS",
    "gbnca_density",
    "gbirk_nS",
    "gbirk_density",
    "gbkqt3_nS",
    "gbkqt3_density",
    "gbegl2_nS",
    "gbegl2_density",
    "gbshk1_nS",
    "gbshk1_density",
    "gbkvs1_nS",
    "gbkvs1_density",
    "gbshl1_nS",
    "gbshl1_density",
    "gbegl36_nS",
    "gbegl36_density",
    "gbegl19_nS",
    "gbegl19_density",
    "gbcca1_nS",
    "gbcca1_density",
    "gbunc2_nS",
    "gbunc2_density",
    "gbkcnl_nS",
    "gbkcnl_density",
    "gbslo1_egl19_nS",
    "gbslo1_egl19_density",
    "gbslo1_unc2_nS",
    "gbslo1_unc2_density",
    "gbslo2_egl19_nS",
    "gbslo2_egl19_density",
    "gbslo2_unc2_nS",
    "gbslo2_unc2_density",
]

PARAMS = [
    "gbnca",
    "gbirk",
    "gbkqt3",
    "gbegl2",
    "gbshk1",
    "gbkvs1",
    "gbshl1",
    "gbegl36",
    "gbegl19",
    "gbcca1",
    "gbunc2",
    "gbkcnl",
    "gbslo1_egl19",
    "gbslo1_unc2",
    "gbslo2_egl19",
    "gbslo2_unc2",
]


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def export_cell(cell: str, diagnostics_dir: Path, output_dir: Path) -> Path:
    cell = cell.upper()
    cell_lower = cell.lower()
    comp_src = diagnostics_dir / f"{cell_lower}_compartment_table.csv"
    preview_src = diagnostics_dir / f"{cell_lower}_nest_params_preview.json"
    for required in [comp_src, preview_src]:
        if not required.exists():
            raise FileNotFoundError(required)

    preview = {int(row["idx"]): row["params"] for row in load_json(preview_src)}

    out_dir = output_dir / cell_lower
    out_dir.mkdir(parents=True, exist_ok=True)
    out = out_dir / f"{cell}_cell.csv"

    with comp_src.open(newline="", encoding="utf-8") as src, out.open("w", newline="", encoding="utf-8") as dst:
        reader = csv.DictReader(src)
        writer = csv.DictWriter(dst, fieldnames=FIELDS)
        writer.writeheader()
        for row in reader:
            params = preview[int(row["idx"])]
            out_row = {
                "idx": row["idx"],
                "parent_idx": row["parent_idx"],
                "label": row["label"],
                "section": row["section"],
                "seg": row["seg"],
                "nseg": row.get("nseg", ""),
                "area_um2": row["area_um2"],
                "C_m_pF": params["C_m"],
                "g_L_nS": params["g_L"],
                "e_L_mV": params["e_L"],
                "g_C_nS": params["g_C"],
            }
            for param in PARAMS:
                out_row[f"{param}_nS"] = params.get(param, 0.0)
                out_row[f"{param}_density"] = params.get(f"{param}_density_nS_per_um2", 0.0)
            writer.writerow(out_row)
    return out


def main() -> None:
    parser = argparse.ArgumentParser(description="Export validated representative cell CSVs from BAAIWorm diagnostics.")
    parser.add_argument("--cell", required=True, help="One of AIYL, AVAL, AWCL, RIML, VD05, or all.")
    parser.add_argument("--diagnostics-dir", type=Path, default=DEFAULT_DIAGNOSTICS)
    parser.add_argument("--output-dir", type=Path, default=PROJECT_ROOT / "data")
    args = parser.parse_args()

    cells = DEFAULT_CELLS if args.cell.lower() == "all" else [args.cell.upper()]
    for cell in cells:
        print(export_cell(cell, args.diagnostics_dir, args.output_dir))


if __name__ == "__main__":
    main()



