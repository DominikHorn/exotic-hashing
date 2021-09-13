import json
import sys
import pandas as pd
from pathlib import Path

import plotly.express as px
from plotly.subplots import make_subplots
import plotly.graph_objects as go

file = "benchmark_results.json" if len(sys.argv) < 2 else sys.argv[1]
with open(file) as data_file:
    data = json.load(data_file)

    # convert json results to dataframe
    df = pd.json_normalize(data, 'benchmarks')

    # augment additional computed columns
    df["hashfn"] = df["label"].apply(lambda x : x.split(":")[0])
    df["dataset"] = df["label"].apply(lambda x : x.split(":")[1])

    # ensure export output folder exists
    results_path = "results" if len(sys.argv) < 3 else sys.argv[2]
    Path(results_path).mkdir(parents=True, exist_ok=True)

    def save_as_csv():
        df.to_csv(f'{results_path}/results.csv')

    def plot_lookup_times():
        lt = df[df["name"].str.lower().str.contains("lookuptime")]

        fig = px.line(
            lt,
            x="dataset_elem_count",
            y="cpu_time",
            color="hashfn",
            facet_col="dataset",
            markers=True,
            log_x=True,
            title="Lookup Time",
            )

        fig.write_image(f'{results_path}/lookup_times.png', width=1000,height=500, scale=8)
        #fig.write_image(f'{results_path}/lookup_times.pdf', width=1000,height=500)
        #fig.show()

    def plot_hashfn_bits_per_key():
        lt = df[
                df["name"].str.lower().str.contains("lookuptime")
                # compact trie is 2 orders of magnitude worse -> don't plot
                & (df["hashfn"].str.lower() != "compacttrie")]

        fig = px.line(
            lt,
            x="dataset_elem_count",
            y="hashfn_bits_per_key",
            color="hashfn",
            facet_col="dataset",
            log_x=True,
            markers=True,
            title="Bits per Key",
            )

        fig.write_image(f'{results_path}/hashfn_bits_per_key.png', width=1000,height=500, scale=8)
        #fig.write_image(f'{results_path}/hashfn_bits_per_key.pdf', width=1000,height=500)
        #fig.show()

    save_as_csv()
    plot_lookup_times()
    plot_hashfn_bits_per_key()
