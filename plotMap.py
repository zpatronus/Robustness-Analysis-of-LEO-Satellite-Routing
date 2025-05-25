# this file is used to plot all kinds of map and the disruption pattern

from models.Constellation import Constellation
import json
import utils

# init
configs = json.load(open("configs/simConfig.json"))
consData = configs["nameOfConstellation"]
# consData = json.load(open(f'configs/{consData}.json'))
consData = json.load(open(f"configs/StarLink-gen2.json"))
radiusOfEarth = configs["radiusOfEarth"]
cons = Constellation(
    consData["nOrbit"],
    consData["nSatPerOrbit"],
    consData["offset"],
    consData["cycle"],
    (consData["alt"] + radiusOfEarth) * 1000,
    consData["inclination"],
    consData["ISLpattern"],
    consData["routingStrategy"],
)

if cons.ISLpattern == 1:
    cons.setPhaseShift(consData["phaseShift"])

cons.buildISL(2000)

# this corresponds to the solar superstorm scenario
# cons.randomlyBreakISL(0.15)

# this corresponds to the high workload scenario
# cons.breakHighLatISL(0.15, 2000)

# this corresponds to the direct sunlight scenario
# cons.breakISLforSunLight(2000, 25)

# this corresponds to the wear and tear scenario
# cons.breakISLforOperation(0.15)

# cons.plotOnlySats(2000)
cons.plotSatsAndISL(2000)

# cons.printISL()
