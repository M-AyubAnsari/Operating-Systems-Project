"""
Cross-Device Statistical Plotter & Shannon Entropy Analyzer
Role: Cross-Validation & Statistical Plotting Lead
Lead: Muhammad Rashad Wahedna
Dataset: output/master_compiled_experiment_data.csv (400 Physical Device Trials)
"""

import os
import csv
import math
from collections import defaultdict

def shannon_entropy(data_bytes):
    if not data_bytes:
        return 0.0
    entropy = 0.0
    length = len(data_bytes)
    freq = defaultdict(int)
    for b in data_bytes:
        freq[b] += 1
    for count in freq.values():
        p = count / length
        entropy -= p * math.log2(p)
    return entropy

def calculate_theoretical_entropies():
    # Representative simulated byte samples of the storage states
    baseline_sample = b"<map><string name=\"auth_token\">CANARY_RUN001_A1B2C3D4_1795551882</string></map>"
    plain_sample = b"" # deleted / 0 bytes
    secure_zero_sample = b"\x00" * 256 # zero-filled blocks
    encrypted_sample = os.urandom(256) # AES-256 ciphertext representation

    return {
        "BASELINE (Plaintext Canary)": shannon_entropy(baseline_sample),
        "PLAIN_CLEANUP (Unlinked File)": 0.0,
        "SECURE_WIPE (0x00 Shredded)": shannon_entropy(secure_zero_sample),
        "HARDWARE_ENCRYPTED (AES-GCM)": shannon_entropy(encrypted_sample)
    }

def generate_svg_bar_chart(title, categories, values, output_path, y_label="Milliseconds (ms)", max_y=None):
    width = 700
    height = 400
    margin_left = 80
    margin_bottom = 60
    margin_top = 50
    margin_right = 40
    
    plot_w = width - margin_left - margin_right
    plot_h = height - margin_top - margin_bottom
    
    if max_y is None:
        max_y = max(values) * 1.25 if values and max(values) > 0 else 1.0

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" style="background-color: #1e1e2e; font-family: sans-serif;">')
    svg.append(f'<text x="{width/2}" y="30" fill="#cdd6f4" font-size="16" font-weight="bold" text-anchor="middle">{title}</text>')
    
    # Grid lines & Y axis labels
    for i in range(5):
        y_val = max_y * (i / 4.0)
        y_pos = margin_top + plot_h - (plot_h * (i / 4.0))
        svg.append(f'<line x1="{margin_left}" y1="{y_pos}" x2="{width - margin_right}" y2="{y_pos}" stroke="#45475a" stroke-dasharray="4" />')
        svg.append(f'<text x="{margin_left - 10}" y="{y_pos + 4}" fill="#a6adc8" font-size="11" text-anchor="end">{y_val:.1f}</text>')
        
    svg.append(f'<text x="25" y="{margin_top + plot_h/2}" fill="#a6adc8" font-size="12" text-anchor="middle" transform="rotate(-90 25 {margin_top + plot_h/2})">{y_label}</text>')

    # Bars
    n = len(categories)
    bar_width = (plot_w / n) * 0.6
    spacing = plot_w / n
    colors = ["#89b4fa", "#a6e3a1", "#fab387", "#f38ba8", "#cba6f7"]

    for i, (cat, val) in enumerate(zip(categories, values)):
        x_center = margin_left + (i * spacing) + (spacing / 2)
        bar_x = x_center - (bar_width / 2)
        h = (val / max_y) * plot_h if max_y > 0 else 0
        bar_y = margin_top + plot_h - h
        c = colors[i % len(colors)]
        
        svg.append(f'<rect x="{bar_x:.1f}" y="{bar_y:.1f}" width="{bar_width:.1f}" height="{h:.1f}" rx="4" fill="{c}" />')
        svg.append(f'<text x="{x_center:.1f}" y="{bar_y - 6:.1f}" fill="#cdd6f4" font-size="11" font-weight="bold" text-anchor="middle">{val:.2f}</text>')
        svg.append(f'<text x="{x_center:.1f}" y="{margin_top + plot_h + 20}" fill="#cdd6f4" font-size="11" text-anchor="middle">{cat}</text>')

    svg.append('</svg>')
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"[CHART] Generated SVG: {output_path}")

def main():
    print("====================================================================")
    print("   CROSS-DEVICE STATISTICAL PLOTTER & ENTROPY ANALYZER              ")
    print("   Role: Cross-Validation & Statistical Plotting Lead               ")
    print("   Lead: Muhammad Rashad Wahedna                                    ")
    print("====================================================================")

    csv_path = os.path.join("output", "master_compiled_experiment_data.csv")
    if not os.path.exists(csv_path):
        print(f"[ERROR] Master dataset not found at {csv_path}")
        return

    os.makedirs(os.path.join("output", "figures"), exist_ok=True)

    with open(csv_path, "r", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    print(f"[DATASET] Loaded {len(rows)} trials from master CSV.")

    # Device metrics
    dev_names = {
        "REDMI_13_HYPEROS": "Redmi 13 (HyperOS)",
        "SAMSUNG_GALAXY_1": "Galaxy A34 (OneUI 6)",
        "SAMSUNG_GALAXY_2": "Galaxy A15 (OneUI 6)",
        "RASHAD_ANDROID": "Galaxy A56 (Android 17)"
    }

    by_dev = defaultdict(list)
    for r in rows:
        by_dev[r["device_tag"]].append(r)

    devices = ["REDMI_13_HYPEROS", "SAMSUNG_GALAXY_1", "SAMSUNG_GALAXY_2", "RASHAD_ANDROID"]
    dev_labels = [dev_names[d] for d in devices if d in by_dev]

    # 1. Setup Latency Comparison
    setup_means = []
    for d in devices:
        if d in by_dev:
            vals = [float(r["t_setup_ms"]) for r in by_dev[d]]
            setup_means.append(sum(vals) / len(vals))

    generate_svg_bar_chart(
        "Mean Storage Setup Latency Across 4 Physical Devices",
        [d.split()[0] for d in dev_labels],
        setup_means,
        os.path.join("output", "figures", "chart_setup_latency_comparison.svg"),
        "Latency (ms)"
    )

    # 2. Secure Wipe Latency Comparison
    wipe_means = []
    for d in devices:
        if d in by_dev:
            vals = [float(r["t_cleanup_ms"]) for r in by_dev[d] if r["config_name"] == "SECURE_WIPE" and r["exit_mode_name"] in ("HOME", "BACK")]
            wipe_means.append(sum(vals) / len(vals) if vals else 0.0)

    generate_svg_bar_chart(
        "Cryptographic Secure Wipe Latency (T_c) Across Devices",
        [d.split()[0] for d in dev_labels],
        wipe_means,
        os.path.join("output", "figures", "chart_secure_wipe_latency.svg"),
        "Wipe Time (ms)"
    )

    # 3. Shannon Entropy by Configuration
    entropies = calculate_theoretical_entropies()
    entropy_labels = ["BASELINE", "PLAIN", "SECURE_WIPE", "ENCRYPTED"]
    entropy_vals = list(entropies.values())

    generate_svg_bar_chart(
        "Forensic Shannon Information Entropy by Defense Configuration",
        entropy_labels,
        entropy_vals,
        os.path.join("output", "figures", "chart_shannon_entropy_comparison.svg"),
        "Entropy (bits / byte)",
        max_y=8.5
    )

    print("\n[SUMMARY] All figures generated successfully in output/figures/!")
    print("These SVG charts can be opened in any web browser, inserted into Word, or used in your presentation slides.")

if __name__ == "__main__":
    main()
