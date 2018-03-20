from revenues import *
import copy
from multiprocessing import Pool, cpu_count
import sys
import cPickle
import os

aUseful, aStd, aFarming = var("aUseful, aStd, aFarming")

def lazy_poet(params):
    # Lazy PoET:
    ## Useful work:
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

    print 'Lazy PoET bestAgeUseful', bestAgeUseful
    print 'Lazy PoET cpuCountUseful', cpuCountUseful

    # Farming: (Mining in standard form doesn't make sense)
    perCPUCostFarm = (eCost + overheadFarm + cost).subs(u, 0).subs(age, aFarming)
    perCPURevFarm = (R_b - perCPUCostFarm).subs(u, 0).subs(params).subs(age, aFarming)
    cpuCountFarm = (total_cap / perCPUCostFarm).subs(params)
    revFarm = (perCPURevFarm * cpuCountFarm).subs(params)

    # we can not optimize for best aFarming because the ratio is still unknown.
    # To find the ratio of useful workers, we need to find the equilibrium
    usefulRatio = var("usefulRatio")
    # revenue is distributed among farming miners
    lazyPoETRb = (annualCryptoRev / ((1 - usefulRatio) * operatorCount * cpuCountFarm )).subs(params)
    # revUseful is a value
    revDiff = (revUseful - revFarm).subs(R_b, lazyPoETRb)

    # equiUsefulRatio can be solved directly to a number
    equiUsefulRatio = solve(revDiff, usefulRatio)[0]

    if equiUsefulRatio > 1:
        equiUsefulRatio = 1
    elif equiUsefulRatio < 0:
        equiUsefulRatio = 0

    # we can now optimize for best aFarming
    f = lambda aOthers: aOthers - minimize_scalar(
            lambdify(aFarming, -revFarm
                     .subs(R_b, lazyPoETRb.subs(aFarming, aOthers))
                     .subs(usefulRatio, equiUsefulRatio)),
            bounds = (0, MAX_STALENESS),
            method='bounded').x
    bestAgeFarming = fsolve(f, MAX_STALENESS / 2)[0]

    usefulPrice = (total_cap / (equiUsefulRatio * cpuCountUseful)).subs(params)

    print("Lazy PoET has %s%.2f farming%s at equilibrium." % (colors.BOLD, 1.0 - equiUsefulRatio, colors.ENDC))
    print("Lazy PoET has best mining staleness of %s%.2f%s at equilibrium." % (colors.BOLD, bestAgeFarming, colors.ENDC))

    if equiUsefulRatio > 0:
        revFarm_v = revFarm.subs(R_b, lazyPoETRb).subs(usefulRatio, equiUsefulRatio).subs(aFarming, bestAgeFarming)
        print("Lazy PoET unit price is %s%.2f%s" % (colors.BOLD, usefulPrice, colors.ENDC))
        print("Useful revenue %.2f and farming revenue %.2f. They should equal." % (revUseful, revFarm_v))
    else:
        usefulPrice = -1
        print("Lazy PoET unit price is %sinfinity%s" % (colors.BOLD, colors.ENDC))


    return (params["annualCryptoRev"], params['operatorCount'],
            bestAgeUseful, bestAgeFarming, equiUsefulRatio, usefulPrice / usefulPriceBaseline)

# Busy POET
def busy_poet(params):
    _, usefulPriceBaseline = useful_work(params)

    # these symbols are declared here in order to differentiate standard
    # operation from mining, because they have different ages.
    eCostStd, eCostFarming, costStd, costFarming, RwStd = var("eCostStd, eCostFarming, costStd, costFarming, RwStd")
    subs = [
        (eCostStd, eCost.subs(u, 1).subs(age, aStd)),
        (eCostFarming, eCost.subs(u, 0).subs(age, aFarming)),
        (costStd, cost.subs(age, aStd)),
        (costFarming, cost.subs(age, aFarming)),
        (RwStd, R_w.subs(age, aStd).subs(u, 1))
    ]

# Standard mining: (working without mining is strictly inferior)
    perCPUCostStandard = (eCostStd + overheadStd + costStd)
    perCPURev = (RwStd + R_b - perCPUCostStandard)
    opCountTmp = params["operatorCount"] # Disgusting hack step 1/4
    del params["operatorCount"]          # Disgusting hack step 2/4
    cpuCountStandard = (total_cap / perCPUCostStandard).subs(params)
    revStandard = (perCPURev * cpuCountStandard)

# Farming: (Mining in standard form doesn't make sense)
    perCPUCostFarm = (eCostFarming + overheadFarm + costFarming)
    perCPURevFarm = (R_b - perCPUCostFarm)
    cpuCountFarm = (total_cap / perCPUCostFarm).subs(params)
    revFarm = (perCPURevFarm * cpuCountFarm)

# equilibrium:
    stdOpRatio = var("stdOpRatio")
    busyPoET_R_b = annualCryptoRev / ( stdOpRatio * operatorCount * cpuCountStandard +
        (1 - stdOpRatio) * operatorCount * cpuCountFarm )
#    busyPoET_R_b = busyPoET_R_b.subs(params) # (1)

    revDiff = (revStandard - revFarm).subs(R_b, busyPoET_R_b)
    simplified = revDiff
    #simplified = nsimplify(revDiff, tolerance=1e-5)
    cacheFileName = "equiStdOpRatio.cache"
    equiStdOpRatio = None
    if not os.path.isfile(cacheFileName): # Memoize:
        print("Single time calculation...")
        equiStdOpRatio = solve(simplified, stdOpRatio)[0]
        cPickle.dump(equiStdOpRatio, open(cacheFileName, "wb"))
        print("Done.")
    else: # Load:
        equiStdOpRatio = cPickle.load(open(cacheFileName, "rb"))
    equiStdOpRatio = equiStdOpRatio.subs(operatorCount, opCountTmp) # Disugsting hack step 3/4
    params["operatorCount"] = opCountTmp # Disgusting hack step 4/4

# fully expand now
    busyPoET_R_b = busyPoET_R_b.subs(subs)
    equiStdOpRatio = equiStdOpRatio.subs(subs)

    aF = MAX_STALENESS / 2
    aS = MAX_STALENESS / 2
    turn = 'S'
    ageStdDone = False
    ageFarmingDone = False

    while True:
        # print 'In turn %s aF=%.2f and aS=%.2f' % (turn, aF, aS)
        if turn == 'S' and not ageStdDone:
            tmp = aS
            # one guy works at aSS, others all work at aOthers
            # R_b and equiStdOpRatio is determined by aOthers
            f = lambda aOthers: aOthers - minimize_scalar(
                lambdify(aStd,
                        -(revStandard
                        .subs(R_b, busyPoET_R_b.subs(aStd, aOthers))
                        .subs(stdOpRatio, equiStdOpRatio.subs(aStd, aOthers))
                        .subs(subs).subs(params)
                        .subs(aFarming, aF))),
                bounds=(0, MAX_STALENESS),
                method='bounded').x

            print 'before fsolve S'
            aS = fsolve(f, MAX_STALENESS / 2)
            print 'after fsolve S'
            if abs(tmp - aS) < 0.01:
                ageStdDone = True
            turn = 'F'
        elif turn == 'F' and not ageFarmingDone:
            tmp = aF
            # one guy works at aFF, others all work at aOthers
            # R_b and equiStdOpRatio is determined by aOthers
            f = lambda aOthers: aOthers - minimize_scalar(
                    lambdify(aFarming,
                            -(revFarm.subs(R_b, busyPoET_R_b.subs(aFarming, aOthers))
                                .subs(stdOpRatio, equiStdOpRatio.subs(aFarming, aOthers))
                                .subs(subs).subs(params)
                                .subs(aStd, aS))),
                    bounds=(0, MAX_STALENESS),
                    method='bounded').x
            print 'before fsolve T'
            aF = fsolve(f, MAX_STALENESS / 2)
            print 'after fsolve T'
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
                        .subs(R_b, busyPoET_R_b.subs(aFarming, aOthers))
                        .subs(stdOpRatio, equiStdOpRatio_v)
                        .subs(subs)
                        .subs(params))),
                bounds=(0, MAX_STALENESS),
                method='bounded').x
        aF = fsolve(f, MAX_STALENESS / 2)[0]
        aS = -1
    elif equiStdOpRatio_v >= 1:
        equiStdOpRatio_v = 1
        f = lambda aOthers: aOthers - minimize_scalar(
                lambdify(aStd,
                        -(revStandard
                        .subs(R_b, busyPoET_R_b.subs(aStd, aOthers))
                        .subs(stdOpRatio, equiStdOpRatio_v)
                        .subs(subs)
                        .subs(params))),
                bounds=(0, MAX_STALENESS),
                method='bounded').x
        aS = fsolve(f, MAX_STALENESS / 2)[0]
        aF = -1

    print "Result: annualCryptoRev %.2f aStd %.2f ; aFarming %.2f equilibrium is %.2f" % (
        params["annualCryptoRev"], aS, aF, equiStdOpRatio_v)


    v_busypoet_Rb = busyPoET_R_b\
            .subs(stdOpRatio, equiStdOpRatio_v)\
            .subs(aFarming, aF)\
            .subs(aStd, aS)\
            .subs(params)

# if one guy is farming using an older chip
    revStandard_v = revStandard\
            .subs(R_b, v_busypoet_Rb)\
            .subs(stdOpRatio, equiStdOpRatio_v)\
            .subs(subs)\
            .subs(params)\
            .subs(aStd, aS)\
            .subs(aFarming, aF)

    revFarm_v = revFarm\
            .subs(R_b, v_busypoet_Rb)\
            .subs(stdOpRatio, equiStdOpRatio_v)\
            .subs(subs)\
            .subs(params)\
            .subs(aStd, aS)\
            .subs(aFarming, aF)

    print("Busy PoET revStandard@%.2f: %s%.2f%s" % (aS, colors.BOLD, revStandard_v, colors.ENDC))
    print("Busy PoET revFarm@%.2f: %s%.2f%s" % (aF, colors.BOLD, revFarm_v, colors.ENDC))

    print (usefulPriceBaseline)
    usefulPrice = (total_cap / (equiStdOpRatio_v*cpuCountStandard*perfSlowdownExpression)
        ).subs(subs).subs(valSubs).subs(age, aS).subs(aStd, aS) / usefulPriceBaseline

    if equiStdOpRatio_v == 0:
# if no one is doing useful work
        usefulPrice = -1
        revFarm_10_v = revFarm.subs(R_b, v_busypoet_Rb).subs(stdOpRatio, equiStdOpRatio_v).subs(subs).subs(params).subs(aFarming, MAX_STALENESS)
        print("Busy PoET revFarm@%.2f: %s%.2f%s" % (MAX_STALENESS, colors.BOLD, revFarm_10_v, colors.ENDC))
    elif equiStdOpRatio_v == 1:
        revFarm_v = revFarm.subs(R_b, v_busypoet_Rb).subs(stdOpRatio, equiStdOpRatio_v).subs(subs).subs(params).subs(aFarming, MAX_STALENESS)

    print 'Usefulprice is', usefulPrice
    return (params["annualCryptoRev"], params['operatorCount'], aS, aF, equiStdOpRatio_v, revStandard_v, revFarm_v, usefulPrice)

def varyAnnaulCryptoRev(func, minRev, maxRev, n):
    params = []
    for an in np.linspace(minRev, maxRev, n):
        tmp = copy.copy(valSubs)
        tmp["annualCryptoRev"] = an
        params.append(tmp)

    pool = Pool(processes=cpu_count())
    results = np.array(pool.map(func, params))
    np.savetxt('../oakland/figs/%s-rev-2.txt' % func.__name__,
            results, fmt='%.4f',
            header='annualCryptoRev, operatorCount, aStd, aFarm, equilStdRatio, usefulPrice')

def varyOperatorCount(func):
    params = []
    for nOp in np.linspace(10, 5000, 10, dtype=float):
        tmp = copy.copy(valSubs)
        tmp["operatorCount"] = nOp
        params.append(tmp)

    pool = Pool(processes=cpu_count())
    results = np.array(pool.map(func, params))
    np.savetxt('../oakland/figs/%s-opcount.txt' % func.__name__,
            results, fmt='%.4f',
            header='annualCryptoRev, operatorCount, aUseful, aStd, equil, usefulPrice')

if __name__ == '__main__':
    # varyAnnaulCryptoRev(busy_poet, 2.5e6, 4e6, 20)
    varyOperatorCount(busy_poet)
    # lazy_poet(valSubs)
    # busy_poet(valSubs)
