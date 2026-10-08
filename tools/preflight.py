#!/usr/bin/env python3
"""Lay every design sheet over each other and list what is unfinished.

Reports, for rows up to the chosen milestone:
  - unfilled cells (null or missing, unless the column is optional)
  - unverified cells (named in the row's _unverified list)
  - cell values that don't match the column type or enum
  - references to other sheets that don't resolve (checked against all rows, any milestone)
Exit code 0 only when nothing is listed.
"""
import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SHEETS = ROOT / "design" / "sheets"
MILESTONES = ["M0", "M1", "M2", "M3"]


def load_sheets():
    sheets = {}
    for p in sorted(SHEETS.glob("*.json")):
        s = json.loads(p.read_text())
        sheets[s["sheet"]] = s
    return sheets


def is_vec3(v):
    return isinstance(v, list) and len(v) == 3 and all(isinstance(x, (int, float)) for x in v)


def type_ok(col, v):
    t = col["type"]
    if t in ("id", "string", "ref"):
        return isinstance(v, str) and v != ""
    if t == "enum":
        return v in col["values"]
    if t == "int":
        return isinstance(v, int) and not isinstance(v, bool)
    if t == "number":
        return isinstance(v, (int, float)) and not isinstance(v, bool)
    if t == "bool":
        return isinstance(v, bool)
    if t == "vec3":
        return is_vec3(v)
    if t == "vec3_list":
        return isinstance(v, list) and len(v) > 0 and all(is_vec3(x) for x in v)
    if t == "int_list":
        return isinstance(v, list) and len(v) > 0 and all(isinstance(x, int) for x in v)
    if t in ("string_list", "ref_list"):
        return isinstance(v, list) and all(isinstance(x, str) and x for x in v)
    raise ValueError(f"unknown column type {t}")


def preflight(sheets, milestone):
    limit = MILESTONES.index(milestone)
    problems = []
    index = {name: {r["id"]: r for r in s["rows"]} for name, s in sheets.items()}

    def resolve(ref, value, where):
        sheet, column = ref.split(".")
        if sheet not in index:
            problems.append(("ref", where, f"sheet '{sheet}' does not exist"))
        elif column != "id":
            problems.append(("ref", where, f"refs must point at an id column, got {ref}"))
        elif value not in index[sheet]:
            problems.append(("ref", where, f"'{value}' is not a row of {sheet}"))

    for name, s in sheets.items():
        cols = s["columns"]
        seen = set()
        for row in s["rows"]:
            rid = row.get("id", "?")
            if rid in seen:
                problems.append(("dup", f"{name}.{rid}", "duplicate id"))
            seen.add(rid)
            if MILESTONES.index(row.get("milestone", "M3")) > limit:
                continue
            for extra in set(row) - set(cols) - {"_unverified", "_source"}:
                problems.append(("column", f"{name}.{rid}.{extra}", "cell for a column the sheet doesn't define"))
            unverified = set(row.get("_unverified", []))
            for cname, col in cols.items():
                where = f"{name}.{rid}.{cname}"
                v = row.get(cname)
                if v is None:
                    if not col.get("optional"):
                        problems.append(("unfilled", where, ""))
                    continue
                if not type_ok(col, v):
                    problems.append(("type", where, f"{v!r} is not a valid {col['type']}"))
                    continue
                if cname in unverified:
                    problems.append(("unverified", where, row.get("_source", "")))
                if col["type"] == "ref":
                    resolve(col["ref"], v, where)
                elif col["type"] == "ref_list":
                    for x in v:
                        resolve(col["ref"], x, where)
    return problems


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--milestone", default="M3", choices=MILESTONES)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()
    sheets = load_sheets()
    problems = preflight(sheets, args.milestone)
    if args.json:
        print(json.dumps([dict(kind=k, cell=w, note=n) for k, w, n in problems], indent=1))
    else:
        rows = sum(1 for s in sheets.values() for r in s["rows"]
                   if MILESTONES.index(r["milestone"]) <= MILESTONES.index(args.milestone))
        cells = sum(len(s["columns"]) for s in sheets.values() for r in s["rows"]
                    if MILESTONES.index(r["milestone"]) <= MILESTONES.index(args.milestone))
        print(f"Preflight up to {args.milestone}: {len(sheets)} sheets, {rows} rows, {cells} cells")
        by_kind = {}
        for k, w, n in problems:
            by_kind.setdefault(k, []).append((w, n))
        for k, items in by_kind.items():
            print(f"\n{k} ({len(items)}):")
            for w, n in items:
                print(f"  {w}" + (f"  -- {n}" if n else ""))
        print("\nCLEAN" if not problems else f"\nNOT CLEAN: {len(problems)} open cells")
    sys.exit(0 if not problems else 1)


if __name__ == "__main__":
    main()
