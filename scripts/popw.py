from revenues import *
import copy
from multiprocessing import Pool, cpu_count
import sys

# PoPW:

# Note: R_b here is per calculation power unit, not cpu count, so multiplied by
# the performance slowdown

# Standard mining: (working without mining makes no sense)

# a bunch of lazy variables
aUseful, aStd, aFarming = var("aUseful, aStd, aFarming")
eCostStd, eCostFarming, costStd, costFarming, RwStd = var("eCostStd, eCostFarming, costStd, costFarming, RwStd")
perfSlowdownStd, perfSlowdownFarm = var("perfSlowdownStd, perfSlowdownFarm")

subs = [
    (eCostStd, eCost.subs(u, 1).subs(age, aStd)),
    (eCostFarming, eCost.subs(u, 0).subs(age, aFarming)),
    (costStd, cost.subs(age, aStd)),
    (costFarming, cost.subs(age, aFarming)),
    (RwStd, R_w.subs(age, aStd).subs(u, 1)),
    (perfSlowdownStd, perfSlowdownExpression.subs(age, aStd)),
    (perfSlowdownFarm, perfSlowdownExpression.subs(age, aFarming))
]

def popw(params):
# get the baseline
    _, usefulPriceBaseline = useful_work(params)
    print 'usefulPriceBaseline is %.2f' % usefulPriceBaseline

    perCPUCostStandard = (eCostStd + overheadStd + costStd)
    perCPURevStandard = (RwStd + R_b * perfSlowdownStd - perCPUCostStandard)
    cpuCountStandard = (total_cap / perCPUCostStandard).subs(params)
    revStandard = (perCPURevStandard * cpuCountStandard)

    perCPUCostFarm = (eCostFarming + overheadFarm + costFarming)
    perCPURevFarm = (R_b * perfSlowdownFarm - perCPUCostFarm)
    cpuCountFarm = (total_cap / perCPUCostFarm).subs(params)
    revFarm = (perCPURevFarm * cpuCountFarm)

    stdOpRatio = var("stdOpRatio")
    PoPW_R_b = (
            annualCryptoRev / (
                (stdOpRatio * operatorCount * cpuCountStandard * perfSlowdownStd) +
                ((1 - stdOpRatio) * operatorCount * cpuCountFarm *  perfSlowdownFarm))
            ).subs(params)

    revDiff = (revStandard - revFarm).subs(R_b, PoPW_R_b)
    equiStdOpRatio = solve(revDiff, stdOpRatio)[0]

    PoPW_R_b = PoPW_R_b.subs(subs)
    equiStdOpRatio = equiStdOpRatio.subs(subs)

    aF = MAX_STALENESS / 2
    aS = MAX_STALENESS / 2
    turn = 'S'
    ageStdDone = False
    ageFarmingDone = False

    while True:
        print 'In turn %s aF=%.2f and aS=%.2f' % (turn, aF, aS)
        if turn == 'S' and not ageStdDone:
            tmp = aS
            # one guy works at aSS, others all work at aOthers
            # R_b and equiStdOpRatio is determined by aOthers
            f = lambda aOthers: aOthers - minimize_scalar(
                lambdify(aStd,
                        -(revStandard
                        .subs(R_b, PoPW_R_b.subs(aStd, aOthers))
                        .subs(stdOpRatio, equiStdOpRatio.subs(aStd, aOthers))
                        .subs(subs).subs(params)
                        .subs(aFarming, aF))),
                bounds=(0, MAX_STALENESS),
                method='bounded').x

            aS = fsolve(f, MAX_STALENESS / 2)
            if abs(tmp - aS) < 0.01:
                ageStdDone = True
            turn = 'F'
        elif turn == 'F' and not ageFarmingDone:
            tmp = aF
            # one guy works at aFF, others all work at aOthers
            # R_b and equiStdOpRatio is determined by aOthers
            f = lambda aOthers: aOthers - minimize_scalar(
                    lambdify(aFarming,
                            -(revFarm.subs(R_b, PoPW_R_b.subs(aFarming, aOthers))
                                .subs(stdOpRatio, equiStdOpRatio.subs(aFarming, aOthers))
                                .subs(subs).subs(params)
                                .subs(aStd, aS))),
                    bounds=(0, MAX_STALENESS),
                    method='bounded').x
            aF = fsolve(f, MAX_STALENESS / 2)
            if abs(tmp - aF) < 0.01:
                ageFarmingDone = True
            turn = 'S'
        else:
            break

    equiStdOpRatio_v = equiStdOpRatio.subs(subs).subs(params).subs(aStd, aS).subs(aFarming, aF)
    print 'Find equilibrium %.2f' % (equiStdOpRatio_v)

    if equiStdOpRatio_v <= 0:
        equiStdOpRatio_v = 0
        # if no standard mining at all
        f = lambda aOthers: aOthers - minimize_scalar(
                lambdify(aFarming,
                        -(revFarm
                        .subs(R_b, PoPW_R_b.subs(aFarming, aOthers))
                        .subs(stdOpRatio, equiStdOpRatio_v)
                        .subs(subs)
                        .subs(params))),
                bounds=(0, MAX_STALENESS),
                method='bounded').x
        aF = fsolve(f, MAX_STALENESS / 2)
        aS = -1
    elif equiStdOpRatio_v > 1:
        equiStdOpRatio_v = 1
        f = lambda aOthers: aOthers - minimize_scalar(
                lambdify(aStd,
                        -(revStandard
                        .subs(R_b, PoPW_R_b.subs(aStd, aOthers))
                        .subs(stdOpRatio, equiStdOpRatio_v)
                        .subs(subs)
                        .subs(params))),
                bounds=(0, MAX_STALENESS),
                method='bounded').x
        aS = fsolve(f, MAX_STALENESS / 2)
        aF = -1

    print "Result: annualCryptoRev %.2f aStd %.2f ; aFarming %.2f equilibrium is %.2f" % (
        params["annualCryptoRev"], aS, aF, equiStdOpRatio_v)

    v_PoPW_Rb = PoPW_R_b\
            .subs(stdOpRatio, equiStdOpRatio_v)\
            .subs(aFarming, aF)\
            .subs(aStd, aS)\
            .subs(params)

    # if one guy is farming using an older chip
    revStandard_v = revStandard\
            .subs(R_b, v_PoPW_Rb)\
            .subs(stdOpRatio, equiStdOpRatio_v)\
            .subs(subs)\
            .subs(params)\
            .subs(aStd, aS)\
            .subs(aFarming, aF)

    revFarm_v = revFarm\
            .subs(R_b, v_PoPW_Rb)\
            .subs(stdOpRatio, equiStdOpRatio_v)\
            .subs(subs)\
            .subs(params)\
            .subs(aStd, aS)\
            .subs(aFarming, aF)

    print("PoPW revStandard@%.2f: %s%.2f%s" % (aS, colors.BOLD, revStandard_v, colors.ENDC))
    print("PoPW revFarm@%.2f: %s%.2f%s" % (aF, colors.BOLD, revFarm_v, colors.ENDC))

    # useful price
    usefulPrice = (total_cap / (equiStdOpRatio_v*cpuCountStandard*perfSlowdownExpression)
        ).subs(subs).subs(valSubs).subs(age, aS).subs(aStd, aS) / usefulPriceBaseline

    if equiStdOpRatio_v == 0:
        usefulPrice = -1
        revFarm_10_v = revFarm.subs(R_b, v_PoPW_Rb).subs(stdOpRatio, equiStdOpRatio_v).subs(subs).subs(params).subs(aFarming, MAX_STALENESS)
        pprint(revFarm)
        pprint(v_PoPW_Rb)
        pprint (revFarm_10_v)
        print("PoPW revFarm@%.2f: %s%.2f%s" % (MAX_STALENESS, colors.BOLD, revFarm_10_v, colors.ENDC))
    elif equiStdOpRatio_v == 1:
        revStd_10_v = revStandard.subs(R_b, v_PoPW_Rb).subs(stdOpRatio, equiStdOpRatio_v).subs(subs).subs(params).subs(aStd, MAX_STALENESS)
        print("PoPW revStandard@%.2f: %s%.2f%s" % (MAX_STALENESS, colors.BOLD, revStd_10_v, colors.ENDC))

    print 'usefulPrice is %.2f' % usefulPrice
    return (params["annualCryptoRev"], aS, aF, equiStdOpRatio_v, usefulPrice)

def test(params):
    return params['annualCryptoRev']

if __name__ == '__main__':
# comment these two lines if you want to run multiple times
    popw(valSubs)
    sys.exit()

    params = []
    for an in np.linspace(300000, 4000000, 20):
        tmp = copy.copy(valSubs)
        tmp["annualCryptoRev"] = an
        params.append(tmp)

    pool = Pool(processes=cpu_count()-1)
    results = np.array(pool.map(popw, params))
    np.savetxt('../oakland/figs/popw.txt',
            results,
            fmt='%.4f',
            header='annualCryptoRev, aS, aF, equiStdOpRatio_v, usefulPrice')

