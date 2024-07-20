import numpy as np
import pandas


def load_stock_data_file(stock_file):
    df = pandas.read_csv(
        filepath_or_buffer=stock_file,
        # index_col="<DATE>",
        # usecols=[
        #     "<DATE>",
        #     "<OPEN>",
        #     "<HIGH>",
        #     "<LOW>",
        #     "<CLOSE>",
        #     "<VOL>"
        # ],
        # dtype={
        #     "<OPEN>": np.float64,
        #     "<HIGH>": np.float64,
        #     "<LOW>": np.float64,
        #     "<CLOSE>": np.float64,
        #     "<VOL>": np.float64
        # },
        usecols=[
            "month",
            "day",
            "high",
            "low",
            "close",
            "volume"
        ],
        dtype={
            "month": np.uint64,
            "day": np.uint64,
            "high": np.uint64,
            "low": np.uint64,
            "close": np.uint64,
            "volume": np.uint64
        },
        engine="c",
        parse_dates=True,
        on_bad_lines="error",
        memory_map=True,
        float_precision="high",
    )

    # return df.assign(
    #     **{
    #         '<DAY>': df.index.day,
    #         '<MONTH>': df.index.month,
    #         '<YEAR>': df.index.year
    #     }
    # )
    return df

