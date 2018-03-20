from revenues import *

# PoW:

aUseful, aFarming = var("aUseful, aFarming")
eCostUseful, eCostFarming, costUseful, costFarming = var("eCostUseful, eCostFarming, costUseful, costFarming")
# define a bunch of laze variable
RwUseful = var("RwUseful")
perfSlowdown = var("perfSlowdown")
subs = [
    (RwUseful, R_w.subs(age, aUseful).subs(u, 1)),
    (eCostUseful, eCost.subs(u, 1).subs(age, aUseful)),
    (eCostFarming, eCost.subs(u, 1).subs(age, aFarming)),
    (costUseful, cost.subs(age, aUseful)),
    (costFarming, cost.subs(age, aFarming)),
    (perfSlowdown, perfSlowdownExpression.subs(age, aFarming))
]

def pow(params):
# Useful work:
    # there will be infinitely many useful works
    perCPUCostUseful = (eCost + overheadStd + cost).subs(u, 1).subs(age, aUseful)
    perCPURevUseful = (R_w - perCPUCostUseful).subs(u, 1).subs(params).subs(age, aUseful)

    # we can optimize for bestAgeUseful because it doesn't depend on anything else
    # in the world
    bestAgeUseful = minimize_scalar(lambdify(Symbol("aUseful"), -perCPURevUseful),
        bounds = (0, MAX_STALENESS),
        method='bounded').x

    cpuCountUseful = (total_cap / perCPUCostUseful).subs(params).subs(aUseful, bestAgeUseful)
    revUseful = (perCPURevUseful * cpuCountUseful).subs(params).subs(aUseful, bestAgeUseful)
    usefulPriceBaseline = (total_cap / cpuCountUseful).subs(params).subs(aUseful, bestAgeUseful)
    # make sure revUseful is now a number
    assert (type(revUseful) == Float)
    assert (type(usefulPriceBaseline) == Float)

# Farming:
    perCPUCostStandard = (eCostFarming + overheadFarm + costFarming)
    perCPURevStandard = (R_b*perfSlowdown - perCPUCostStandard)
    cpuCountStandard = (total_cap / perCPUCostStandard).subs(params)
    revStandard = (perCPURevStandard * cpuCountStandard).subs(params)

    var("standardOperatorRatio")
    PoW_R_b = annualCryptoRev / (standardOperatorRatio * operatorCount * cpuCountStandard * perfSlowdown).subs(u, 1)
    PoW_R_b = PoW_R_b.subs(params) # (standardOperatorRatio and s)

# In quilibrium the revenues are the same:
    revDiff = (revUseful - revStandard).subs(R_b, PoW_R_b)
    equilFarmingOpRatio = solve(revDiff, standardOperatorRatio)[0]

# fully expand now
    equilFarmingOpRatio = equilFarmingOpRatio.subs(subs).subs(aUseful, bestAgeUseful).subs(params)
    PoW_R_b = PoW_R_b.subs(subs)

    f = lambda aOthers: aOthers - minimize_scalar(
        lambdify(aFarming,-(revStandard
                        .subs(subs)
                        .subs(R_b, PoW_R_b.subs(aFarming, aOthers))
                        .subs(standardOperatorRatio, equilFarmingOpRatio.subs(aFarming, aOthers))
                        .subs(aUseful, bestAgeUseful)
                    .subs(params))),
        bounds = (0, MAX_STALENESS),
        method='bounded').x

    bestSFarming = fsolve(f, MAX_STALENESS / 2)

# get the equilFarmingOpRatio number
    equilFarmingOpRatio_v = equilFarmingOpRatio.subs(aFarming, bestSFarming).subs(aUseful, bestAgeUseful)

    usefulCount = (1 - equilFarmingOpRatio_v) * cpuCountUseful.subs(subs).subs(params).subs(aUseful, bestAgeUseful)
    usefulPrice = (total_cap / usefulCount).subs(subs).subs(params).subs(aUseful, bestAgeUseful) / usefulPriceBaseline

    # if everyone is mining, then the price for useful work is natually infinity
    if (equilFarmingOpRatio_v >= 1):
        usefulPrice = -1

    return params['annualCryptoRev'], bestSFarming, bestAgeUseful, equilFarmingOpRatio_v, usefulPrice

import copy
import sys
from multiprocessing import Pool

if __name__ == '__main__':
    params = []
    for an in np.linspace(100000, 3000000, 50):
        tmp = copy.copy(valSubs)
        tmp["annualCryptoRev"] = an
        params.append(tmp)

    pool = Pool(processes=8)
    result = np.array(pool.map(pow, params))
    np.savetxt('../oakland/figs/pow.txt', result, fmt='%.4f',
            header='annualCryptoRev, bestSFarming, bestAgeUseful, equilFarmingOpRatio, usefulPrice')
