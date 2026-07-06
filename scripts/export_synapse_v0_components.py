from __future__ import annotations

import argparse
import json
import math
import os
import re
from pathlib import Path
from typing import Any

import numpy as np
import pandas as pd


PROJECT_ROOT = Path(__file__).resolve().parents[1]


def _default_source_root() -> Path:
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


SOURCE_ROOT = _default_source_root()
COOK_DIR = SOURCE_ROOT / "cook 2019 SI"
FENYVES_DIR = SOURCE_ROOT / "fenyves 2020 S Data"

COOK_SI2 = COOK_DIR / "41586_2019_1352_MOESM6_ESM SI2.xlsx"
COOK_SI5 = COOK_DIR / "41586_2019_1352_MOESM9_ESM SI5.xlsx"
FENYVES_S1 = FENYVES_DIR / "S1_Data.xlsx"


def default_baai_table4_text(source_root: Path) -> Path:
    return source_root.parent / "multi-compartment neuron 0416" / "baaiworm_pdf_text" / "pdf_1.txt"

REFERENCE_CELL_FILES = {
    "AWC": "AWCL.json",
    "AIY": "AIYL.json",
    "AVA": "AVAL.json",
    "RIM": "RIML.json",
    "VD5": "VD05.json",
}

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


def norm(name: Any) -> str:
    text = str(name).strip()
    match = re.match(r"^(AS|DA|DB|DD|VA|VB|VC|VD)(\d+)$", text)
    if match:
        return f"{match.group(1)}{int(match.group(2)):02d}"
    return text


def is_non_neuron(name: str) -> bool:
    return any(re.match(pattern, name) for pattern in NON_NEURON_PATTERNS)


def normalize_polarity(value: Any) -> str:
    text = str(value).strip().lower()
    if text in {"+", "plus", "positive", "exc", "excitatory"}:
        return "+"
    if text in {"-", "minus", "negative", "inh", "inhibitory"}:
        return "-"
    if text == "complex":
        return "complex"
    if text in {"no pred", "nopred", "no prediction", "0", "nan", ""}:
        return "no pred"
    raise ValueError(f"Unknown Fenyves polarity label: {value!r}")


def normalize_weight(weight: float, max_weight: float, strategy: str) -> float:
    if max_weight <= 0.0:
        return 0.0
    if strategy == "log1p_max":
        return math.log1p(weight) / math.log1p(max_weight)
    if strategy == "linear_max":
        return weight / max_weight
    if strategy == "sqrt_max":
        return math.sqrt(weight) / math.sqrt(max_weight)
    if strategy == "none":
        return weight
    raise ValueError(f"Unknown weight normalization strategy: {strategy}")


def read_adj_matrix(excel_path: Path, sheet_name: str) -> pd.DataFrame:
    raw = pd.read_excel(excel_path, sheet_name=sheet_name, header=None)
    col_names = [norm(x) for x in raw.iloc[2, 3:].dropna().astype(str).to_list()]
    row_names = [norm(x) for x in raw.iloc[3:, 2].dropna().astype(str).to_list()]
    data = raw.iloc[3:, 3:]
    data = data.apply(pd.to_numeric, errors="coerce").fillna(0.0)
    data = data.iloc[: len(row_names), : len(col_names)]
    return pd.DataFrame(data.to_numpy(dtype=float), index=row_names, columns=col_names)


def adj_to_edges(adj: pd.DataFrame, directed: bool, drop_self: bool = False) -> pd.DataFrame:
    matrix = adj.to_numpy(dtype=float, copy=True)
    if directed:
        src_idx, tgt_idx = np.where(matrix > 0)
        weights = matrix[src_idx, tgt_idx]
    else:
        matrix = np.triu(matrix, k=1)
        src_idx, tgt_idx = np.where(matrix > 0)
        weights = matrix[src_idx, tgt_idx]
    edges = pd.DataFrame(
        {
            "pre": np.array(adj.index)[src_idx],
            "post": np.array(adj.columns)[tgt_idx],
            "anatomical_weight": weights,
        }
    )
    if drop_self:
        edges = edges[edges["pre"] != edges["post"]].reset_index(drop=True)
    return edges


def load_fenyves_connections() -> pd.DataFrame:
    raw = pd.read_excel(FENYVES_S1, sheet_name="5. Sign prediction", header=None)
    data = raw.iloc[2:, [0, 3, 4, 5, 16]].copy()
    data.columns = ["pre", "post", "fenyves_weight", "edge_type", "fenyves_polarity_raw"]
    data["pre"] = data["pre"].map(norm)
    data["post"] = data["post"].map(norm)
    data["edge_type"] = data["edge_type"].astype(str).str.strip().str.lower()
    data["fenyves_weight"] = pd.to_numeric(data["fenyves_weight"], errors="coerce").fillna(0.0)
    data["fenyves_polarity_raw"] = data["fenyves_polarity_raw"].map(normalize_polarity)
    data = data[
        (data["edge_type"] == "chemical")
        & (data["fenyves_weight"] > 0.0)
    ].reset_index(drop=True)
    data["connection_id"] = data["pre"] + "__" + data["post"]
    return data[
        [
            "connection_id",
            "pre",
            "post",
            "fenyves_weight",
            "fenyves_polarity_raw",
        ]
    ]


def load_cook_chemical_connections() -> pd.DataFrame:
    adj = read_adj_matrix(COOK_SI5, "hermaphrodite chemical")
    edges = adj_to_edges(adj, directed=True)
    edges = edges.rename(columns={"anatomical_weight": "cook_weight"})
    edges["connection_id"] = edges["pre"] + "__" + edges["post"]
    return edges[["connection_id", "pre", "post", "cook_weight"]]


def load_scored_contact_counts() -> pd.DataFrame:
    adj = read_adj_matrix(COOK_SI2, "herm chem synapse adjacency")
    edges = adj_to_edges(adj, directed=True)
    edges = edges.rename(columns={"anatomical_weight": "scored_contact_count"})
    edges["connection_id"] = edges["pre"] + "__" + edges["post"]
    return edges[["connection_id", "scored_contact_count"]]


def effective_polarity(row: pd.Series) -> tuple[str, str]:
    source = str(row["connection_source"])
    raw = row.get("fenyves_polarity_raw")
    if pd.isna(raw):
        return "-", "missing_fenyves_default_inhibitory"
    raw_text = str(raw)
    if raw_text == "no pred":
        return "-", "fenyves_no_pred_default_inhibitory"
    if source == "fenyves_only":
        return raw_text, "fenyves_only_connection_with_sign"
    return raw_text, "fenyves_sign_prediction"


def build_chemical_connections() -> pd.DataFrame:
    cook = load_cook_chemical_connections()
    fenyves = load_fenyves_connections()
    merged = cook.merge(
        fenyves,
        on="connection_id",
        how="outer",
        suffixes=("_cook", "_fenyves"),
        indicator=True,
    )

    merged["pre"] = merged["pre_cook"].combine_first(merged["pre_fenyves"])
    merged["post"] = merged["post_cook"].combine_first(merged["post_fenyves"])
    merged["connection_source"] = merged["_merge"].map(
        {
            "left_only": "cook_si5_only",
            "right_only": "fenyves_only",
            "both": "cook_si5_and_fenyves",
        }
    )
    merged["cook_weight"] = pd.to_numeric(merged["cook_weight"], errors="coerce")
    merged["fenyves_weight"] = pd.to_numeric(merged["fenyves_weight"], errors="coerce")
    merged["anatomical_weight"] = merged["cook_weight"].combine_first(merged["fenyves_weight"])
    merged["weight_source"] = np.where(merged["cook_weight"].notna(), "Cook2019_SI5", "Fenyves2020_only")

    polarity = merged.apply(effective_polarity, axis=1, result_type="expand")
    merged["effective_polarity"] = polarity[0]
    merged["polarity_policy"] = polarity[1]
    merged["active_in_v0"] = True
    merged["pre_is_neuron"] = ~merged["pre"].map(is_non_neuron)
    merged["post_is_neuron"] = ~merged["post"].map(is_non_neuron)
    merged["connection_class"] = np.where(
        merged["pre_is_neuron"] & merged["post_is_neuron"],
        "neuron_to_neuron",
        np.where(merged["pre_is_neuron"], "neuron_to_non_neuron_or_muscle", "non_neuron_source"),
    )
    merged = merged.drop(columns=["pre_cook", "post_cook", "pre_fenyves", "post_fenyves", "_merge"])

    scored = load_scored_contact_counts()
    out = merged.merge(scored, on="connection_id", how="left")
    out["scored_contact_count"] = out["scored_contact_count"].fillna(0.0)
    out["has_scored_contact"] = out["scored_contact_count"] > 0.0
    out["has_cook_si5_connection"] = out["cook_weight"].notna()
    out["has_fenyves_sign_row"] = out["fenyves_weight"].notna()
    return out[
        [
            "connection_id",
            "pre",
            "post",
            "connection_class",
            "connection_source",
            "anatomical_weight",
            "cook_weight",
            "fenyves_weight",
            "weight_source",
            "fenyves_polarity_raw",
            "effective_polarity",
            "polarity_policy",
            "active_in_v0",
            "pre_is_neuron",
            "post_is_neuron",
            "has_cook_si5_connection",
            "has_fenyves_sign_row",
            "scored_contact_count",
            "has_scored_contact",
        ]
    ].sort_values(["pre", "post"]).reset_index(drop=True)


def add_optional_cook_metadata(connections: pd.DataFrame) -> pd.DataFrame:
    scored = load_scored_contact_counts()
    out = connections.merge(scored, on="connection_id", how="left")
    out["scored_contact_count"] = out["scored_contact_count"].fillna(0.0)
    out["has_scored_contact"] = out["scored_contact_count"] > 0.0
    out["has_final_connection"] = True
    return out


def build_components(
    connections: pd.DataFrame,
    norm_strategy: str,
    g0_exc_uS: float,
    g0_inh_uS: float,
    e_exc_mV: float,
    e_inh_mV: float,
    tau_ms: float,
    v_half_mV: float,
    k_s_mV: float,
) -> pd.DataFrame:
    max_weight = float(connections["anatomical_weight"].max()) if len(connections) else 0.0
    rows: list[dict[str, Any]] = []
    for _, row in connections.iterrows():
        polarity = str(row["effective_polarity"])

        component_specs: list[tuple[str, float, float, float]] = []
        if polarity == "+":
            component_specs.append(("exc", 1.0, g0_exc_uS, e_exc_mV))
        elif polarity == "-":
            component_specs.append(("inh", 1.0, g0_inh_uS, e_inh_mV))
        elif polarity == "complex":
            component_specs.append(("exc", 0.5, g0_exc_uS, e_exc_mV))
            component_specs.append(("inh", 0.5, g0_inh_uS, e_inh_mV))
        else:
            raise ValueError(f"Unhandled polarity: {polarity}")

        normalized_weight = normalize_weight(float(row["anatomical_weight"]), max_weight, norm_strategy)
        for component_type, rho, g0_uS, e_rev_mV in component_specs:
            g_uS = g0_uS * normalized_weight * rho
            rows.append(
                {
                    "component_id": f"{row['connection_id']}__{component_type}",
                    "connection_id": row["connection_id"],
                    "pre": row["pre"],
                    "post": row["post"],
                    "pre_compartment": 0,
                    "post_compartment": 0,
                    "placement_policy": "soma_compartment_0_v0",
                    "component_type": component_type,
                    "fenyves_polarity_raw": row["fenyves_polarity_raw"],
                    "effective_polarity": polarity,
                    "polarity_policy": row["polarity_policy"],
                    "connection_class": row["connection_class"],
                    "connection_source": row["connection_source"],
                    "anatomical_weight": row["anatomical_weight"],
                    "cook_weight": row["cook_weight"],
                    "fenyves_weight": row["fenyves_weight"],
                    "weight_source": row["weight_source"],
                    "normalized_weight": normalized_weight,
                    "rho": rho,
                    "g0_uS": g0_uS,
                    "g_uS": g_uS,
                    "e_rev_mV": e_rev_mV,
                    "tau_ms": tau_ms,
                    "v_half_mV": v_half_mV,
                    "k_s_mV": k_s_mV,
                    "active_in_v0": True,
                    "evidence": "Cook2019_SI5_structure__Fenyves2020_polarity",
                }
            )
    return pd.DataFrame(rows)


def build_gap_junctions(norm_strategy: str, g0_gap_uS: float) -> pd.DataFrame:
    adj = read_adj_matrix(COOK_SI5, "herm gap jn symmetric")
    edges = adj_to_edges(adj, directed=False)
    edges = edges[(~edges["pre"].map(is_non_neuron)) & (~edges["post"].map(is_non_neuron))].reset_index(drop=True)
    max_weight = float(edges["anatomical_weight"].max()) if len(edges) else 0.0
    rows = []
    for _, row in edges.iterrows():
        normalized_weight = normalize_weight(float(row["anatomical_weight"]), max_weight, norm_strategy)
        rows.append(
            {
                "gap_id": f"{row['pre']}__{row['post']}",
                "cell_a": row["pre"],
                "cell_b": row["post"],
                "compartment_a": 0,
                "compartment_b": 0,
                "placement_policy": "soma_compartment_0_v0",
                "anatomical_weight": row["anatomical_weight"],
                "normalized_weight": normalized_weight,
                "g0_uS": g0_gap_uS,
                "g_uS": g0_gap_uS * normalized_weight,
                "active_in_v0": True,
                "evidence": "Cook2019_SI5_gap_junction",
            }
        )
    return pd.DataFrame(rows)


def parse_baaiworm_table4(table_text_path: Path) -> pd.DataFrame:
    text = table_text_path.read_text(encoding="utf-8", errors="replace")
    marker = "Supplementary Table 4. 302 Neurons and Functional Groups"
    start = text.find(marker)
    if start < 0:
        raise ValueError(f"Could not find BAAIWorm Supplementary Table 4 marker in {table_text_path}")
    end_marker = "Supplementary Table 5."
    end = text.find(end_marker, start)
    if end < 0:
        end_marker = "Supplementary Table 6."
        end = text.find(end_marker, start)
    table = text[start : end if end > start else len(text)]
    pattern = re.compile(
        r"(\d+)\s+([A-Z0-9]+)\s+"
        r"(sensory neuron|interneuron|command neuron|head motor neuron|body motor neuron)\s+"
        r"(AWC|AIY|AVA|RIM|VD5)"
    )
    rows = []
    for match in pattern.finditer(table):
        rows.append(
            {
                "baai_index": int(match.group(1)),
                "neuron": norm(match.group(2)),
                "functional_group": match.group(3),
                "parameter_reference": match.group(4),
                "reference_cell": REFERENCE_CELL_FILES[match.group(4)].replace(".json", ""),
                "evidence": "BAAIWorm_Supplementary_Table_4",
            }
        )
    df = pd.DataFrame(rows)
    if len(df) != 302:
        raise ValueError(f"Expected 302 BAAIWorm Table 4 rows, parsed {len(df)} from {table_text_path}")
    return df.sort_values("baai_index").reset_index(drop=True)


def synapse_config(args: argparse.Namespace) -> dict[str, Any]:
    return {
        "chemical": {
            "structure_source": "Cook2019_SI5_hermaphrodite_chemical",
            "polarity_source": "Fenyves2020_S1_Data_sheet_5_sign_prediction",
            "fenyves_only_policy": "add_connection_with_fenyves_weight_and_sign",
            "weight_normalization": args.weight_norm,
            "g0_exc_uS": args.g0_exc_uS,
            "g0_inh_uS": args.g0_inh_uS,
            "tau_ms": args.tau_ms,
            "v_half_mV": args.v_half_mV,
            "k_s_mV": args.k_s_mV,
            "E_exc_mV": args.e_exc_mV,
            "E_inh_mV": args.e_inh_mV,
            "complex_rho_exc": 0.5,
            "complex_rho_inh": 0.5,
            "no_pred_policy": "default_inhibitory",
            "missing_fenyves_policy": "default_inhibitory",
        },
        "gap": {
            "weight_normalization": args.weight_norm,
            "g0_gap_uS": args.g0_gap_uS,
        },
        "neuron_parameter_reference": {
            "source": "BAAIWorm Supplementary Table 4",
            "representatives": REFERENCE_CELL_FILES,
        },
        "disabled_in_v0": {
            "harris_soft_prior": True,
            "wang_transmitter_metadata": True,
            "monoamine_modulation": True,
            "stdp": True,
            "individual_synapse_level_polarity": True,
        },
    }


def validate(connections: pd.DataFrame, components: pd.DataFrame, gaps: pd.DataFrame) -> None:
    if connections.empty:
        raise ValueError("No Cook/Fenyves chemical connections exported")
    if components.empty:
        raise ValueError("No active chemical components exported")
    if gaps.empty:
        raise ValueError("No Cook SI5 gap junctions exported")

    component_counts = components.groupby("connection_id")["component_type"].count()
    bad_counts = component_counts[component_counts > 2]
    if not bad_counts.empty:
        raise ValueError(f"Connections with too many components: {bad_counts.head().to_dict()}")

    rho_sum = components.groupby("connection_id")["rho"].sum()
    bad_rho = rho_sum[np.abs(rho_sum - 1.0) > 1e-9]
    if not bad_rho.empty:
        raise ValueError(f"Active component rho does not sum to 1: {bad_rho.head().to_dict()}")

    if (components["g_uS"] < 0.0).any() or (gaps["g_uS"] < 0.0).any():
        raise ValueError("Negative conductance generated")


def main() -> int:
    parser = argparse.ArgumentParser(description="Export SynapseModel-v0 Cook/Fenyves CSV inputs.")
    parser.add_argument("--source-root", type=Path, default=SOURCE_ROOT)
    parser.add_argument("--cook-dir", type=Path)
    parser.add_argument("--fenyves-dir", type=Path)
    parser.add_argument("--cook-si2", type=Path)
    parser.add_argument("--cook-si5", type=Path)
    parser.add_argument("--fenyves-s1", type=Path)
    parser.add_argument("--output-dir", type=Path, default=PROJECT_ROOT / "data" / "synapse_v0")
    parser.add_argument("--weight-norm", choices=["log1p_max", "linear_max", "sqrt_max", "none"], default="log1p_max")
    parser.add_argument("--g0-exc-uS", type=float, default=4.9e-4)
    parser.add_argument("--g0-inh-uS", type=float, default=2.0e-4)
    parser.add_argument("--g0-gap-uS", type=float, default=1.0e-4)
    parser.add_argument("--tau-ms", type=float, default=10.0)
    parser.add_argument("--v-half-mV", type=float, default=-20.0)
    parser.add_argument("--k-s-mV", type=float, default=5.0)
    parser.add_argument("--e-exc-mV", type=float, default=30.0)
    parser.add_argument("--e-inh-mV", type=float, default=-70.0)
    parser.add_argument("--baai-table4-text", type=Path)
    args = parser.parse_args()

    global COOK_SI2, COOK_SI5, FENYVES_S1
    cook_dir = args.cook_dir or args.source_root / "cook 2019 SI"
    fenyves_dir = args.fenyves_dir or args.source_root / "fenyves 2020 S Data"
    COOK_SI2 = args.cook_si2 or cook_dir / "41586_2019_1352_MOESM6_ESM SI2.xlsx"
    COOK_SI5 = args.cook_si5 or cook_dir / "41586_2019_1352_MOESM9_ESM SI5.xlsx"
    FENYVES_S1 = args.fenyves_s1 or fenyves_dir / "S1_Data.xlsx"
    if args.baai_table4_text is None:
        args.baai_table4_text = default_baai_table4_text(args.source_root)

    for required in [COOK_SI2, COOK_SI5, FENYVES_S1, args.baai_table4_text]:
        if not required.exists():
            raise FileNotFoundError(required)

    args.output_dir.mkdir(parents=True, exist_ok=True)

    connections = build_chemical_connections()
    components = build_components(
        connections,
        args.weight_norm,
        args.g0_exc_uS,
        args.g0_inh_uS,
        args.e_exc_mV,
        args.e_inh_mV,
        args.tau_ms,
        args.v_half_mV,
        args.k_s_mV,
    )
    gaps = build_gap_junctions(args.weight_norm, args.g0_gap_uS)
    reference_df = parse_baaiworm_table4(args.baai_table4_text)
    validate(connections, components, gaps)
    neuron_names = set(reference_df["neuron"])
    neuron_neuron_components = components[
        components["pre"].isin(neuron_names) & components["post"].isin(neuron_names)
    ].reset_index(drop=True)

    connections.to_csv(args.output_dir / "chemical_connections_v0.csv", index=False)
    components.to_csv(args.output_dir / "chemical_components_v0.csv", index=False)
    neuron_neuron_components.to_csv(args.output_dir / "chemical_components_neuron_neuron_v0.csv", index=False)
    gaps.to_csv(args.output_dir / "gap_junctions_v0.csv", index=False)
    reference_df.to_csv(args.output_dir / "neuron_parameter_reference_v0.csv", index=False)
    (args.output_dir / "synapse_config_v0.json").write_text(
        json.dumps(synapse_config(args), indent=2), encoding="utf-8"
    )

    polarity_counts = connections["effective_polarity"].value_counts().to_dict()
    raw_polarity_counts = connections["fenyves_polarity_raw"].fillna("missing").value_counts().to_dict()
    component_counts = components["component_type"].value_counts().to_dict()
    summary = {
        "data_sources": {
            "source_root": str(args.source_root),
            "fenyves_s1": str(FENYVES_S1),
            "cook_si2": str(COOK_SI2),
            "cook_si5": str(COOK_SI5),
            "baai_table4_text": str(args.baai_table4_text),
        },
        "chemical_connections": int(len(connections)),
        "chemical_connections_by_source": connections["connection_source"].value_counts().to_dict(),
        "chemical_connections_by_class": connections["connection_class"].value_counts().to_dict(),
        "chemical_connections_active_in_v0": int(connections["active_in_v0"].sum()),
        "chemical_connections_by_raw_fenyves_polarity": raw_polarity_counts,
        "chemical_connections_by_effective_polarity": polarity_counts,
        "chemical_connections_by_polarity_policy": connections["polarity_policy"].value_counts().to_dict(),
        "chemical_components": int(len(components)),
        "chemical_components_neuron_neuron": int(len(neuron_neuron_components)),
        "chemical_components_by_type": component_counts,
        "gap_junctions": int(len(gaps)),
        "weight_normalization": args.weight_norm,
        "neuron_parameter_references": reference_df["parameter_reference"].value_counts().to_dict(),
        "neuron_functional_groups": reference_df["functional_group"].value_counts().to_dict(),
    }
    (args.output_dir / "summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))
    print(f"Wrote SynapseModel-v0 tables to {args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())



