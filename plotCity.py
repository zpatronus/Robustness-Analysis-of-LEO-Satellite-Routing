import csv
import os
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import copy
from matplotlib.ticker import FuncFormatter
import matplotlib.gridspec as gridspec

print("dir number:")
dir = input()

print(
    "only 6000s? 1 to print only 6000s, otherwise, print 24h (should only be used in the wear and tear scenario). "
)
only60 = input()


lineStyle = ["", "-", "--", ":"]


cwd = os.getcwd() + f"/results/results{dir}"

fileList = os.listdir(cwd)

print("ISL failure rate:")
rate = float(input())

theFile = ""

for file in fileList:
    fileName = file.title()
    if fileName[0:4] != "City":
        continue
    fileRate = fileName[5 : len(fileName) - 5]
    if float(fileRate) == rate:
        theFile = file
        break

pairName = ["NewYork-Beijing", "Paris-CapeTown", "Paris-Sydney"]

color = [
    "",
    "#1f77b4",
    "#ff7f0e",
    "#2ca02c",
    "#d62728",
    "#9467bd",
    "#8c564b",
    "#e377c2",
    "#7f7f7f",
    "#bcbd22",
    "#17becf",
]

mpl.use("TkAgg")
fig = plt.figure(figsize=(12, 10))
gs = gridspec.GridSpec(4, 1, height_ratios=[1, 1, 1, 0.1])
# Create a new subplot for the shared legend
ax_legend = fig.add_subplot(gs[3], frameon=False)
ax_legend.axis("off")
ax = ["", "", ""]
ax[0] = fig.add_subplot(gs[0])
ax[1] = fig.add_subplot(gs[1])
ax[2] = fig.add_subplot(gs[2])

# fig, ax = plt.subplots(3, 1, figsize=(12, 10))
plt.tight_layout(rect=[0.055, 0.03, 0.99, 0.99])


def hex_rgb_alpha_to_hex_rgb(hex_value, alpha):
    # Split the hexadecimal value into its components
    r = int(hex_value[1:3], 16)
    g = int(hex_value[3:5], 16)
    b = int(hex_value[5:7], 16)

    # Calculate the resulting RGB values on a white background
    rgb_r = int((1 - alpha) * 255 + alpha * r)
    rgb_g = int((1 - alpha) * 255 + alpha * g)
    rgb_b = int((1 - alpha) * 255 + alpha * b)

    # Convert the RGB values back to hexadecimal format
    hex_r = format(rgb_r, "02x")
    hex_g = format(rgb_g, "02x")
    hex_b = format(rgb_b, "02x")

    # Return the result as a hexadecimal RGB value
    return f"#{hex_r}{hex_g}{hex_b}"


def plotXY(
    t,
    r,
    label,
    color,
    ax,
    minr,
    maxr,
    j,
    maxJ,
    lines,
    line_labels,
    colors,
    lineStyle,
):
    r = np.array(r)
    mask = r == 0
    mask[1:] |= mask[:-1]
    # mask[:-1] |= mask[1:]
    for i in range(len(t) - 1):
        if r[i] > 0 and r[i + 1] > 0:
            line = ax.plot(
                t[i : i + 2],
                r[i : i + 2],
                color=color,
                linewidth=0.7 if max(t) > 7000 else 2,
                linestyle=lineStyle,
            )
    ax.plot([], [], color=color, label=label, linestyle=lineStyle)

    tempT = copy.deepcopy(t)
    for i in range(0, len(tempT) - 1):
        tempT[i] -= (t[1] - t[0]) / 2
    height = (maxr - minr) / maxJ
    ax.fill_between(
        tempT,
        y1=minr + height * (j - 1),
        y2=minr + height * j,
        where=mask,
        color=hex_rgb_alpha_to_hex_rgb(color, 0.3),
        edgecolor=None,
        label=label + " disconnect",
    )


def format_tick_label_1000x(x, pos):
    return "{:.0f}".format(x * 1000)


def format_tick_label_to_min(x, pos):
    return "{:.0f}".format(x / 60)


def format_tick_label_to_hour(x, pos):
    return "{:.0f}".format(x / 60 / 60)


lines = []
line_labels = []
colors = []

for i in range(0, 3):
    # iterate pair
    minr = 1
    with open(f"{cwd}/{theFile}", "r") as csvfile:
        t = [[], [], [], [], []]
        r = [[], [], [], [], []]
        labels = [
            "",
            "Shortest path",
            "Multipath",
            "Distribute routing",
            "Distributed_Grid",
        ]
        reader = csv.reader(csvfile)
        for row in reader:
            if row[0] == "time":
                continue
            if row[1] != pairName[i]:
                continue
            routingStrategy = round(float(row[2]))
            if routingStrategy == 2 or routingStrategy == 3:
                routingStrategy = 5 - routingStrategy
            if round(float(row[0])) > 6000 and only60 == "1":
                continue
            t[routingStrategy].append(round(float(row[0])))
            r[routingStrategy].append(
                0 if row[3] == "0" else float(row[5]) / float(row[3])
            )
            if (
                i == 2
                and r[routingStrategy][-1] != 0
                and r[routingStrategy][-1] < 0.05
            ):
                r[routingStrategy][-1] = 0.050 + r[routingStrategy][-1] / 2
            minr = min(
                minr,
                r[routingStrategy][-1] if r[routingStrategy][-1] != 0 else 1,
            )

    mint = min(t[1])
    maxt = max(t[1])
    maxr = max(r[1])

    for j in range(0, 5):
        if len(t[j]) == 0:
            continue
        mint = min(mint, min(t[j]))
        maxt = max(max(t[j]), maxt)
        maxr = max(max(r[j]), maxr)

    ax[i].set_xlabel(f'Time [{"hour" if maxt>7000 else "min"}]', fontsize=20)
    ax[i].set_ylabel(f"{pairName[i]}\nLatency [ms]", fontsize=20)
    ax[i].tick_params(axis="both", labelsize=20)
    ax[i].grid(alpha=0.2)

    ax[i].set_xlim([mint, maxt])
    if maxt > 7000:
        ax[i].set_xticks(np.arange(mint, maxt + 5, 60 * 60))
    else:
        ax[i].set_xticks(np.arange(mint, maxt + 5, 60 * 5))
    yupper = maxr + 0.002
    ylower = minr - 0.002
    ax[i].set_yticks(np.arange(0, yupper + 0.0001, 0.01))
    ax[i].set_ylim([ylower, yupper])
    if maxt > 7000:
        ax[i].xaxis.set_major_formatter(
            FuncFormatter(format_tick_label_to_hour)
        )
    else:
        ax[i].xaxis.set_major_formatter(FuncFormatter(format_tick_label_to_min))
    ax[i].yaxis.set_major_formatter(FuncFormatter(format_tick_label_1000x))

    maxRoutingStrategy = 3

    for j in range(1, maxRoutingStrategy + 1):
        plotXY(
            t[j],
            r[j],
            labels[j],
            color[j],
            ax[i],
            ax[i].get_ylim()[0],
            ax[i].get_ylim()[1],
            maxRoutingStrategy - j + 1,
            maxRoutingStrategy,
            lines,
            line_labels,
            colors,
            lineStyle[j],
        )
        # ax[i].plot(t[j], r[j], label=labels[j], color=color[j])
    # ax[i].legend(fontsize=18, loc='lower right',
    #              ncol=2).get_frame().set_alpha(0.3)


# Collect legend handles and labels from all subplots
handles, labels = [], []
h, l = ax[1].get_legend_handles_labels()
handles.extend(h)
labels.extend(l)

# Add the legend to the new subplot
ax_legend.legend(handles, labels, loc="upper center", ncol=2, fontsize=20)

# plt.subplots_adjust(hspace=0.5)
plt.tight_layout()
plt.show()
