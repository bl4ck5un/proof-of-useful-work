from revenues import *
import matplotlib.pyplot as plt

def pow(costASIC, ASICPerfBoost):
    # PoW:

    aUseful, aStd, aFarming = var("aUseful, aStd, aFarming")
    eCostUseful, eCostStd, costUseful, costStd = var("eCostUseful, eCostStd, costUseful, costStd")
    # define a bunch of laze variable
    RwStd, RwUseful = var("RwStd, RwUseful")
    perfSlowdown = var("perfSlowdown")
    subs = [
        (RwStd, R_w.subs(age, aStd).subs(u, 1)),
        (RwUseful, R_w.subs(age, aUseful).subs(u, 1)),
        (eCostUseful, eCost.subs(u, 1).subs(age, aUseful)),
        (eCostStd, eCost.subs(u, 1).subs(age, aStd)),
        (costUseful, cost.subs(age, aUseful)),
        (costStd, cost.subs(age, aStd)),
        (perfSlowdown, perfSlowdownExpression.subs(age, aStd))
    ]

    # Useful work:
    perCPUCostUseful = (eCostUseful + overheadStd + costUseful)
    perCPURevUseful = (RwUseful - perCPUCostUseful)

    # we can not optimize for the useful because the revenue doesn't
    # depend on anything else

    bestSUseful = minimize_scalar(lambdify(aUseful, -perCPURevUseful.subs(subs).subs(valSubs)),
                                  bounds=(0, MAX_STALENESS),
                                  method='bounded').x

    cpuCountUseful = (total_cap / perCPUCostUseful).subs(valSubs).subs(aUseful, bestSUseful)
    revUseful = (perCPURevUseful * cpuCountUseful).subs(valSubs).subs(aUseful, bestSUseful) # number, not expression
    # print("Non-mining optimal staleness is %s%.2f%s. " % (colors.BOLD, bestSUseful, colors.ENDC))

    usefulPrice = (total_cap / cpuCountUseful).subs(subs).subs(valSubs).subs(aUseful, bestSUseful)
    # print("Non-mining unit cost for useful work is %s%.2f%s. " % (colors.BOLD, usefulPrice, colors.ENDC))

    # Farming with ASIC:
    perCPUCostASIC = (costASIC)
    perCPURevSASIC = (R_b*ASICPerfBoost - perCPUCostASIC)
    cpuCountASIC = (total_cap / perCPUCostASIC).subs(valSubs)
    revASIC = (perCPURevSASIC * cpuCountASIC).subs(valSubs)

    var("standardOperatorRatio")
    ASICRb = annualCryptoRev / (standardOperatorRatio * operatorCount * cpuCountASIC * ASICPerfBoost).subs(u, 1)
    ASICRb = ASICRb.subs(valSubs) # (standardOperatorRatio and s)


    # In quilibrium the revenues are the same:
    revDiff = (revUseful - revASIC).subs(R_b, ASICRb)
    equilStdOpRatio = solve(revDiff, standardOperatorRatio)[0]

    # fully expand now
    equilStdOpRatio_v = equilStdOpRatio.subs(subs).subs(aUseful, bestSUseful).subs(valSubs)
    ASICRb_v = ASICRb.subs(subs).subs(standardOperatorRatio, equilStdOpRatio_v)

    usefulCount = (1 - equilStdOpRatio_v) * cpuCountUseful
    usefulCost = (total_cap / usefulCount).subs(subs).subs(valSubs).subs(aUseful, bestSUseful)
    if (equilStdOpRatio_v > 1):
        usefulCost = -1

    pprint ((equilStdOpRatio_v*operatorCount*cpuCountASIC* ASICRb_v).subs(valSubs))
    pprint (annualCryptoRev.subs(valSubs))

    return (equilStdOpRatio_v, usefulCost)




def print2y(x, y1, y2, xlabel, ylabel1, ylabel2):
    fig, ax1 = plt.subplots()
    ax1.plot(x, y1, 'b-')

    ax1.set_xlabel(xlabel)
    ax1.set_ylabel(ylabel1, color='b')
    for tl in ax1.get_yticklabels():
        tl.set_color('b')

    ax2 = ax1.twinx()
    ax2.plot(x, y2, 'r-')

    ax2.set_ylabel(ylabel2, color='r')
    for tl in ax2.get_yticklabels():
        tl.set_color('r')

    plt.show()

result = []
for asciCost in np.linspace(10, 100, 20):
    r, c = pow(asciCost, 1)
    result.append((asciCost, r, c))

r = np.array(result)

# print2y(r[:,0], r[:,1], r[:,2], 'ASIC Cost ($)', 'ratio', 'usefulCost')

result = []
for perfBoost in np.linspace(1, 10, 20):
    r, c = pow(100, perfBoost)
    result.append((perfBoost, r, c))

import pprint
pprint.pprint(result)

r = np.array(result)
print2y(r[:,0], r[:,1], r[:,2], 'ASIC Speed (compared to CPU)', 'ratio', 'usefulCost')
