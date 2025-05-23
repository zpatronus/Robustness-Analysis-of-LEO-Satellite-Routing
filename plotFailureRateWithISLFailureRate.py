# this file is used to plot disconnection rate with ISL failure percentage.

import csv
import os
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.gridspec as gridspec

lineStyle = ["-", "--", ":", "-."]


dirList = ["81", "83", "82"]
legends = [
    "Shortest path, Solar superstorm",
    "Multipath, Solar superstorm",
    "Distributed routing, Solar superstorm",
]

# least hop is just an experiment, dont use this
# dirList = ["81", "83", "82", "97"]
# legends = [
#     "Shortest path, Solar superstorm",
#     "Multipath, Solar superstorm",
#     "Distributed routing, Solar superstorm",
#     "Least Hops",
# ]


# dirList = ["84", "86", "85"]
# legends = [
#     "Shortest path, High workload",
#     "Multipath, High workload",
#     "Distributed routing, High workload",
# ]


# dirList = ["75", "77", "76"]
# legends = [
#     "Shortest path, Wear and tear",
#     "Multipath, Wear and tear",
#     "Distributed routing, Wear and tear",
# ]

mpl.use("TkAgg")
fig = plt.figure(figsize=(14, 6))
gs = gridspec.GridSpec(2, 2, width_ratios=[1, 1], height_ratios=[1.2, 0.1])
ax = ["", ""]
ax[0] = fig.add_subplot(gs[0, 0])
ax[1] = fig.add_subplot(gs[0, 1])
# Span the entire second row
ax_legend = fig.add_subplot(gs[1, :], frameon=False)
ax_legend.axis("off")

# fig, ax = plt.subplots(figsize=(14, 6))


for print0_1 in range(0, 2):
    ax[print0_1].set_xlabel(
        f'ISL Failure Percentage [%]\n{"(a) from 0-100%" if print0_1==0 else "(b) from 0-1%"}',
        fontsize=24,
    )

    if print0_1 == 0:
        ax[print0_1].set_ylabel("Disconnection Rate", fontsize=24)

    ax[print0_1].tick_params(axis="both", labelsize=24)

    # plt.xticks(fontsize=24)
    # plt.yticks(fontsize=24)

    ax[print0_1].grid(alpha=0.2)
    maxDis = 0

    for i in range(0, len(dirList)):
        cwd = os.getcwd() + f"/results/results{dirList[i]}"
        x = []
        y = []

        with open(f"{cwd}/integrate.csv", "r") as csvfile:
            reader = csv.reader(csvfile)
            for row in reader:
                if row[0] != "ratio":
                    if not (float(row[0]) > 1 and print0_1):
                        x.append(float(row[0]))
                        y.append(float(row[3]))

        # plt.scatter(x, y)
        maxDis = max(max(y), maxDis)
        # plt.plot(x, y, label=legends[i])
        ax[print0_1].plot(x, y, label=legends[i], linestyle=lineStyle[i])

    if print0_1:
        ax[print0_1].set_xlim([0, 1])
        ax[print0_1].set_ylim([0, maxDis + 0.02])
        ax[print0_1].set_xticks(np.arange(0, 1.0001, 0.2))
        ax[print0_1].set_yticks(np.arange(0, maxDis + 0.02 + 0.000001, 0.04))
    else:
        ax[print0_1].set_xlim([0, 100])
        ax[print0_1].set_ylim([0, 1])
        ax[print0_1].set_xticks(np.arange(0, 101, 20))
        ax[print0_1].set_yticks(np.arange(0, 1.0000001, 0.2))

# if (print0_1):
#     plt.legend(fontsize=24, loc='upper left')
# else:
#     plt.legend(fontsize=24, loc='lower right')

# Collect legend handles and labels from all subplots
handles, labels = [], []
h, l = ax[1].get_legend_handles_labels()
handles.extend(h)
labels.extend(l)

# Add the legend to the new subplot
ax_legend.legend(handles, labels, loc="upper center", ncol=2, fontsize=22)
# plt.tight_layout()
plt.tight_layout(rect=[0.01, 0.02, 1.02, 1])
plt.show()
