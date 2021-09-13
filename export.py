import json
import sys
import pandas as pd
from pathlib import Path

file = f"benchmark_results.json" if len(sys.argv) < 2 else sys.argv[1]

with open(file) as data_file:
    data = json.load(data_file)

    results_path = "results" if len(sys.argv) < 3 else sys.argv[2]
    results_file = "{}/results.csv".format(results_path)
    Path(results_path).mkdir(parents=True, exist_ok=True)
    df = pd.json_normalize(data, 'benchmarks')
    df.to_csv(results_file)

