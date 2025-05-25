import csv
import os

print("Input folder id")

folderid = input()

cwd = os.getcwd()
cwd += f"/results/results{folderid}"
fileList = os.listdir(cwd)
success = [0 for i in range(0, 40100)]
fail = [0 for i in range(0, 40100)]

variable = ""

for file in fileList:
    if file[0:4] != "brea":
        continue
    with open(f"{cwd}/{file}", "r") as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            if row[0] != "ratio" and row[0] != "theta":
                success[round(float(row[0]) * 100)] += float(row[1])
                fail[round(float(row[0]) * 100)] += float(row[2])
            else:
                title = row[0]

f = open(f"{cwd}/integrate.csv", "w")
f.write(f"{title},success,fail,fail_rate\n")
for i in range(0, 40001):
    if success[i] + fail[i] == 0:
        continue
    f.write(f"{i/100},{success[i]},{fail[i]},{fail[i]/(success[i]+fail[i])}\n")
