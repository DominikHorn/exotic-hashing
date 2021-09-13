import json
import math
import pandas as pd
import sys
from pathlib import Path

from plotly.subplots import make_subplots
import plotly.express as px
import plotly.graph_objects as go

pal = px.colors.qualitative.Plotly
color_sequence = ["#BBB", "#777", "#111", pal[9], pal[4], pal[6], pal[1], pal[0], "#58a2c4", pal[5], pal[2], pal[7], pal[8], pal[3]]

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
        return 0
    df["order"] = df.apply(lambda x : order(x["hashfn"]), axis=1)
    df = df.sort_values(by=["order", "dataset_elem_count"])

    # prepare datasets for plotting
    lt_df = df[df["name"].str.lower().str.contains("lookuptime")].copy(deep=True)
    bt_df = df[df["name"].str.lower().str.contains("buildtime")].copy(deep=True)

    # augment plotting datasets
    lt_df["throughput"] = lt_df.apply(lambda x : 10**9 / x["cpu_time"], axis=1)
    def magnitude(x):
        l = math.log(x, 10)
        rem = round(x/pow(10, l), 2)
        exp = int(round(l, 0))
        return f'${rem} \cdot 10^{{{exp}}}$'
    lt_df["elem_magnitude"] = lt_df.apply(lambda x : magnitude(x["dataset_elem_count"]), axis=1)
    bt_df["cpu_time_per_key"] = bt_df.apply(lambda x : x["cpu_time"] / x["dataset_elem_count"], axis=1)
    bt_df["sorted"] = bt_df.apply(lambda x : x["name"].lower().startswith("presorted"), axis=1)

    # ensure export output folder exists
    results_path = "results" if len(sys.argv) < 3 else sys.argv[2]
    Path(results_path).mkdir(parents=True, exist_ok=True)

    def save_as_csv():
        df.to_csv(f'{results_path}/results.csv')

    def commit(fig, name):
        fig.write_image(f'{results_path}/{name}.png', width=1000,height=500, scale=8)
        #fig.write_image(f'{results_path}/{name}.pdf', width=1000,height=500)
        #fig.show()

    def plot_lookup_times():
        name = "lookup_time"
        fig = px.line(
            lt_df,
            x="dataset_elem_count",
            y="cpu_time",
            color="hashfn",
            facet_col="dataset",
            markers=True,
            log_x=True,
            title="Lookup Time",
            color_discrete_sequence=color_sequence
            )
        commit(fig, name)
        fig.update_yaxes(range=[-5, 1400])
        commit(fig, f'zoomed_{name}')

    def plot_lookup_throughput():
        f_lt_df = lt_df[lt_df["dataset_elem_count"].isin([10**4, 10**6, 10**8])]
        name = "lookup_throughput"
        fig = px.bar(
            f_lt_df,
            x="elem_magnitude",
            y="throughput",
            color="hashfn",
            barmode="group",
            facet_col="dataset",
            title="Lookup Throughput",
            color_discrete_sequence=color_sequence
            )
        fig.update_xaxes(type='category')
        commit(fig, name)
        fig.update_yaxes(range=[0, 30 * 10**6])
        commit(fig, f'zoomed_{name}')

    def plot_hashfn_bits_per_key():
        name = "bits_per_key"
        fig = px.line(
            lt_df,
            x="dataset_elem_count",
            y="hashfn_bits_per_key",
            color="hashfn",
            facet_col="dataset",
            log_x=True,
            markers=True,
            title="Bits per Key",
            color_discrete_sequence=color_sequence
            )
        commit(fig, name)
        fig.update_yaxes(range=[-5, 170])
        commit(fig, f'zoomed_{name}')

    def plot_build_time():
        name = "build_time"
        fig = px.line(
            bt_df,
            x="dataset_elem_count",
            y="cpu_time_per_key",
            color="hashfn",
            facet_col="dataset",
            facet_row="sorted",
            log_x=True,
            markers=True,
            title="Build time per Key",
            color_discrete_sequence=color_sequence
            )
        commit(fig, name)
        fig.update_yaxes(matches=None, range=[-50, 4650], row=1)
        fig.update_yaxes(matches=None, range=[-50, 2350], row=2)
        commit(fig, f'zoomed_{name}')

    save_as_csv()
    plot_lookup_times()
    plot_hashfn_bits_per_key()
    plot_lookup_throughput()
    plot_build_time()
