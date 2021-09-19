import json
import math
import pandas as pd
import sys
import git
from pathlib import Path
from inspect import cleandoc

from plotly.subplots import make_subplots
import plotly.express as px
import plotly.graph_objects as go
import plotly

# plot colors
pal = px.colors.qualitative.Plotly
color_sequence = ["#BBB", "#777", "#111", pal[9], pal[4], pal[6], pal[1], pal[0], "#58a2c4", pal[5], pal[2], pal[7], pal[8], pal[3]]

# plot labels
plot_labels = dict(
    cpu_time_per_key='ns per key',
    dataset_elem_count='dataset size',
    elem_magnitude='dataset size',
    hashfn_bits_per_key='bits per key',
    throughput='keys per second')


file = "benchmark_results.json" if len(sys.argv) < 2 else sys.argv[1]
with open(file) as data_file:
    data = json.load(data_file)

    # convert json results to dataframe
    df = pd.json_normalize(data, 'benchmarks')

    # augment additional computed columns
    df["hashfn"] = df["label"].apply(lambda x : x.split(":")[0])
    df["dataset"] = df["label"].apply(lambda x : x.split(":")[1])

    # order data (important for legend & colors)
    def order(x):
        x = x.lower()
        if x == "donothinghash":
            return 1
        if x == "rankhash":
            return 2
        if x == "recsplit_leaf12_bucket9":
            return 3
        if x == "compacttrie":
            return 4
        if x == "fastsuccincttrie":
            return 5
        if x == "simplehollowtrie":
            return 6
        if x == "hollowtrie":
            return 7
        if x == "mwhc":
            return 8
        if x == "compressedmwhc":
            return 9
        if x == "compactedmwhc":
            return 10
        if x == "learnedlinear":
            return 11
        if x == "mapomphf":
            return 12
        return 0
    df["order"] = df.apply(lambda x : order(x["hashfn"]), axis=1)
    df = df.sort_values(by=["order", "dataset_elem_count"])

    # augment plotting datasets
    def magnitude(x):
        l = math.log(x, 10)
        rem = round(x/pow(10, l), 2)
        exp = int(round(l, 0))
        #return f'${rem} \cdot 10^{{{exp}}}$'
        return f'{rem}e-{exp}'
    df["elem_magnitude"] = df.apply(lambda x : magnitude(x["dataset_elem_count"]), axis=1)

    # prepare datasets for plotting & augment dataset specific columns
    lt_df = df[df["name"].str.lower().str.contains("lookuptime")].copy(deep=True)
    bt_df = df[df["name"].str.lower().str.contains("buildtime")].copy(deep=True)

    lt_df["cpu_time_per_key"] = lt_df['cpu_time']
    lt_df["throughput"] = lt_df.apply(lambda x : 10**9 / x["cpu_time_per_key"], axis=1)

    bt_df["cpu_time_per_key"] = bt_df.apply(lambda x : x["cpu_time"] / x["dataset_elem_count"], axis=1)
    bt_df["throughput"] = bt_df.apply(lambda x : 10**9 / x["cpu_time_per_key"], axis=1)
    bt_df["sorted"] = bt_df.apply(lambda x : x["name"].lower().startswith("presorted"), axis=1)

    # ensure export output folder exists
    results_path = "docs" if len(sys.argv) < 3 else sys.argv[2]
    Path(results_path).mkdir(parents=True, exist_ok=True)

    def convert_to_html(fig):
        #fig.show()
        return plotly.offline.plot(fig, include_plotlyjs=False, output_type='div')

    def plot_lookup_times():
        name = "lookup_time"
        fig = px.line(
            lt_df,
            x="dataset_elem_count",
            y="cpu_time_per_key",
            color="hashfn",
            facet_col="dataset",
            facet_col_wrap=3,
            markers=True,
            log_x=True,
            labels=plot_labels,
            color_discrete_sequence=color_sequence
            )
        return convert_to_html(fig)

    def plot_hashfn_bits_per_key():
        name = "bits_per_key"
        fig = px.line(
            lt_df,
            x="dataset_elem_count",
            y="hashfn_bits_per_key",
            color="hashfn",
            facet_col="dataset",
            facet_col_wrap=3,
            log_x=True,
            markers=True,
            labels=plot_labels,
            color_discrete_sequence=color_sequence
            )
        return convert_to_html(fig)

    def plot_build_time():
        # cpu to enable value changes
        f_bt_df = bt_df.copy(deep=True)
        f_bt_df = f_bt_df[f_bt_df["dataset_elem_count"].isin([10**4, 10**6, 10**8])
                & (f_bt_df["dataset"].str.lower() != "gap_10")]
        f_bt_df["throughput"] = f_bt_df.apply(lambda x : 0 if x["hashfn"].lower() == 'donothinghash' else x['throughput'], axis=1)
        name = "build_time"
        fig = px.bar(
            f_bt_df,
            x="elem_magnitude",
            y="throughput",
            color="hashfn",
            barmode="group",
            facet_col="dataset",
            facet_row="sorted",
            labels=plot_labels,
            color_discrete_sequence=color_sequence
            )
        return convert_to_html(fig)

    def plot_raw_data():
        raw_data = df.sort_values(by=["name"])
        raw_data = raw_data.rename({"cpu_time": "ns", 'hashfn': 'function', 'dataset_elem_count': 'keys', 'hashfn_bits_per_key': 'bits per key'}, axis='columns')

        return raw_data.to_html(
                columns=["name", "function", "dataset", "keys", "bits per key", "ns"],
                index=False,
                formatters={"ns": lambda x : str(int(float(x))), 'keys': lambda x: str(int(x))}
                )

    with open(f'{results_path}/index.html', 'w') as readme:
        readme.write(cleandoc(f"""
        <!doctype html>
        <html>
          <head>
              <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
          </head>

          <body>
            <h2>Lookup - Times in nanoseconds per key</h2>
            {plot_lookup_times()}

            <h2>Space - Total bits per key occupied by datastructure</h2>
            {plot_hashfn_bits_per_key()}

            <h2>Build - Build time per key in nanoseconds</h2>
            {plot_build_time()}

            <h2>Raw Data</h2>
            {plot_raw_data()}
          </body>
        </html>
        """))
