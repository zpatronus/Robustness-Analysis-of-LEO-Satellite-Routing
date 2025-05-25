# this file is to plot the disconnection rate in the direct sunlight scenario

import csv
import os
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

# phase shift 0
dirList = ["43", "45", "44"]
legends = [
    "PS 0, Shortest path, Sunlight",
    "PS 0, Multipath, Sunlight",
    "PS 0, Distributed routing, Sunlight",
]

# phase shift 5/72
# dirList = ["52", "54", "53"]
# legends = [
#     "PS 5/72, Shortest path, Sunlight",
#     "PS 5/72, Multipath, Sunlight",
#     "PS 5/72, Distributed routing, Sunlight",
# ]

# phase shift 11/72
# dirList = ["72", "74", "73"]
# legends = [
#     "PS 11/72, Shortest path, Sunlight",
#     "PS 11/72, Multipath, Sunlight",
#     "PS 11/72, Distributed routing, Sunlight",
# ]

mpl.use("TkAgg")
fig, ax = plt.subplots(figsize=(14, 4.5))
plt.tight_layout(rect=[0.06, 0.09, 0.95, 1])


xtick_values = [*range(0, 181, 30)]
xtick_labels = [
    "Spring (Fall)",
    "",
    "",
    "Summer (Winter)",
    "",
    "",
    "Fall (Spring)",
]
ax.set_xlabel("Time in 6 months", fontsize=23)
ax.set_ylabel("Disconnection Rate", fontsize=23)
plt.xticks(fontsize=23)
plt.yticks(fontsize=23)
ax.grid(alpha=0.2)


lineStyle = ["-", "--", ":"]

maxy = 0

for i in range(0, len(dirList)):
    cwd = os.getcwd() + f"/results/results{dirList[i]}"
    x = []
    y = []

    with open(f"{cwd}/integrate.csv", "r") as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            if row[0] != "theta":
                x.append(float(row[0]))
                y.append(float(row[3]))

    maxy = max(maxy, max(y))

    # plt.scatter(x, y)

    # plt.plot(x, y)
    plt.plot(x, y, label=legends[i], linestyle=lineStyle[i])


ax.set_xlim([0, 180])
ax.set_ylim([0, maxy + 0.05])
plt.xticks(xtick_values, xtick_labels)
ax.set_yticks(np.arange(0, maxy + 0.05, 0.05 if (maxy < 0.3) else 0.1))

plt.legend(
    fontsize=23, loc="upper center" if dirList[0] != "72" else "upper left"
)
plt.show()
