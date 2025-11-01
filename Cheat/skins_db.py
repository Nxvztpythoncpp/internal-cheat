import re
from pathlib import Path

# === Load game files ===
items = Path("items_game.txt").read_text(encoding="utf-8", errors="ignore")
lang_path = Path("csgo_english.txt")
lang = lang_path.read_text(encoding="utf-8", errors="ignore") if lang_path.exists() else ""

# === Parse localization (csgo_english.txt) ===
loc = {f"#{m.group(1)}": m.group(2) for m in re.finditer(r'"#([^"]+)"\s*"([^"]+)"', lang)}

# === Parse paint_kits from items_game.txt ===
paintkits = {}

# Match paint_kits block reliably (supports nested braces)
paintkits_section = re.search(r'"paint_kits"\s*\{([\s\S]*?)^\}', items, re.M)
if not paintkits_section:
    raise SystemExit("❌ Couldn't find 'paint_kits' section in items_game.txt")

for pkid, block in re.findall(r'"(\d+)"\s*\{([\s\S]*?)\}', paintkits_section.group(1)):
    name_m = re.search(r'"name"\s*"([^"]+)"', block)
    if not name_m:
        continue
    desc_m = re.search(r'"description_tag"\s*"([^"]+)"', block)

    tech = name_m.group(1)  # technical/internal name
    desc_tag = desc_m.group(1) if desc_m else ""
    display = loc.get(desc_tag, tech.replace("_", " ").title())

    # detect weapon type
    weapon_match = re.search(
        r'_(ak47|m4a4|m4a1_silencer|awp|deagle|glock|usp_silencer|fiveseven|p250|p90|mp9|mp7|mac10|nova|xm1014|ump45|bizon|mag7|negev|sawedoff|tec9|p2000|scar20|sg553|ssg08|cz75a|revolver|knife_[a-z0-9_]+|bayonet)',
        tech
    )
    if not weapon_match:
        continue
    weapon = weapon_match.group(1)

    # === Clean up name ===
    pretty = re.sub(
        r'^(Cu|Aq|Gs|Am|Aa|Hy|Sp|So|Sa|Ck|M|An|P|E|Wp|Dp)\s+', '', display, flags=re.I
    )
    pretty = re.sub(
        r'\b(Ak47|Awp|M4A4|M4A1|Glock|Fiveseven|Deagle|P250|P90|Mp9|Mp7|Mac10|Nova|Xm1014|Ump45|Bizon|Mag7|Negev|Sawedoff|Tec9|P2000|Scar20|Sg553|Ssg08|Cz75A|Revolver|Bayonet)\b',
        '', pretty, flags=re.I
    )
    pretty = pretty.strip(" -_")

    entry_name = pretty if pretty else display
    paintkits.setdefault(weapon, []).append((entry_name, int(pkid), tech))

# === Write skins_db.h ===
out = Path("skins_db.h")
with out.open("w", encoding="utf-8") as f:
    f.write("// Auto-generated from items_game.txt + csgo_english.txt\n")
    f.write("// Contains localized skin names, simplified for readability\n")
    f.write("#pragma once\n#include <unordered_map>\n#include <vector>\n#include <string>\n\n")
    f.write("static const std::unordered_map<std::string,std::vector<std::pair<std::string,int>>> skins_db = {\n")

    for weapon, entries in sorted(paintkits.items()):
        f.write(f'    {{ "{weapon}", {{\n')
        for name, pid, tech in sorted(entries, key=lambda x: x[1]):
            safe_name = name.replace('"', r'\"')
            f.write(f'        {{"{safe_name}", {pid}}}, // {tech}\n')
        f.write("    } }},\n")

    f.write("};\n")

print(f"[✅] Wrote {out} with {len(paintkits)} weapons and localized skin names.")
