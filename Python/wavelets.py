import numpy as np

from Python.stock_data_source import load_stock_data_file

import fcwt


def test_algorithm(
        df,
        time,
        cwt,
        averaged_frequencies
):
    # price_diff = (df[1:]["<LOW>"] - df["<HIGH>"].shift(1))[1:]
    # positive_diffs = price_diff[price_diff > 0]
    # freq_y = get_frequency_magnitudes_of_closest_frequency(
    #     cwt,
    #     averaged_frequencies,
    #     freq
    # )
    # day_after_peak_of_frequency_magnitude = (
    #         find_peaks(freq_y)[0] + 1
    # )
    # day_after_trough_of_frequency_magnitude = (
    #         find_peaks(-freq_y)[0] + 1
    # )
    # combined_indices = [(index, 'p') for index in day_after_peak_of_frequency_magnitude]
    # combined_indices += [(index, 't') for index in day_after_trough_of_frequency_magnitude]
    # combined_indices.sort(key=lambda x: x[0])

    # price_in = None
    # day_in = None
    # net = 1.0
    #
    # lag = 10
    # for i, kind in combined_indices:
    #     if kind == "p":
    #         if i + lag < len(df):
    #             price_in = df["<HIGH>"].iloc[i + lag]
    #             day_in = i
    #     else:
    #         if i + lag < len(df):
    #             price_out = df["<LOW>"].iloc[i + lag]
    #             if price_in is not None:
    #                 day_out = i
    #                 # print("day diff: " + str(day_out - day_in))
    #                 # print("price diff: " + str(price_out - price_in))
    #                 net = (price_out / price_in) * net

    net = 1.0
    price_in = None
    bought = False
    for i, t in enumerate(time):
        total = 0.0
        for j, frequency in enumerate(averaged_frequencies):
            if i > 2:
                magnitude = df["<LOW>"][i - 2]
                last_magnitude = df["<LOW>"][i - 3]
                direction = magnitude - last_magnitude
                total += (direction * cwt[j][i])
        if total > 0 and not bought:
            price_in = df["<HIGH>"][i]
            bought = True
        elif bought:
            price_out = df["<LOW>"][i]
            bought = False
            net = (price_out / price_in) * net
    print(str(net))


data_folder = "../data/final/direction/"
test_file = data_folder + "ibm.txt"

df = load_stock_data_file(test_file)

# original_df = df[df["<YEAR>"] > 2022][:]
# df = original_df["<VOL>"]
original_df = df[-500:]
df = original_df["low"]
# time = (df.index - df.index[0]).days.values
# sampling_frequency = int(np.diff(time).mean())
sampling_frequency = 1
lowest_frequency = 1
highest_frequency = 101
number_of_frequencies = 200

frequencies, out = fcwt.cwt(
    df,
    sampling_frequency,
    f0=lowest_frequency,
    f1=highest_frequency,
    fn=number_of_frequencies
)

plot_df = df.reset_index()["low"]
fcwt.plot(
    plot_df,
    sampling_frequency,
    f0=lowest_frequency,
    f1=highest_frequency,
    fn=number_of_frequencies
)

# test_algorithm(
#     original_df,
#     time,
#     out,
#     frequencies
# )
