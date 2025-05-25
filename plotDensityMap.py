# this file is used to plot the estimate traffic distribution density

import csv
import matplotlib.pyplot as plt
import numpy as np

bucket = []
lat = []
resolution = 3

for i in range(0, 300):
    lat.append(-90 + i * resolution)
    bucket.append(0)
    if lat[i] > 90:
        break


def latToIndex(lat, latitude):
    for i in range(0, 180):
        if lat[i] <= latitude and latitude <= lat[i + 1]:
            return i if latitude - lat[i] < lat[i + 1] - latitude else i + 1


with open("all_in_one_real_csv.csv", "r") as speedFile:
    reader = csv.reader(speedFile)
    for row in reader:
        if row[0] == "region":
            continue
        value = float(row[11])  # * float(row[1])
        index = latToIndex(lat, float(row[6]))
        bucket[index] += value / 2
        bucket[index + 1] += value / 4
        bucket[index - 1] += value / 4


fig, ax = plt.subplots(figsize=(14, 4))
ax.set_xscale("log")

ax.plot(bucket, lat)

ax.set_ylim([-90, 90])
ax.set_yticks(np.arange(-90, 91, 30))
# ax.xaxis.set_major_locator(plt.LogLocator(base=10, subs=(1.0,)))
ax.set_xlim([1e3, 1100000])
ax.set_xticks([1e3, 5e3, 1e4, 5e4, 1e5, 5e5, 1e6])
ax.tick_params(axis="both", labelsize=23)
ax.set_xlabel("Average network speed [Mbps] × Number of netizens", fontsize=23)
ax.set_ylabel("Latitude", fontsize=23)
ax.grid(alpha=0.2)
plt.tight_layout()
plt.show()
