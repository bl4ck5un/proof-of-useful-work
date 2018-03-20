from revenues import *
import copy
from multiprocessing import Pool, cpu_count



def pouw(params):
# PoUW:

# Note: R_b here is per calculation power unit, not cpu count, so multiplied by
# the performance slowdown and the cpu utilization.

    aUseful, aStd = var("aUseful, aStd")

# define a bunch of laze variable
    RwStd, RwUseful = var("RwStd, RwUseful")
    perfSlowdown = var("perfSlowdown")
    subs = [
        (RwStd, R_w.subs(age, aStd).subs(u, 1)),
        (RwUseful, R_w.subs(age, aUseful).subs(u, 1)),
        (perfSlowdown, perfSlowdownExpression.subs(age, aStd))
    ]

# Useful work:
    perCPUCostUseful = (eCost + overheadStd + cost).subs(u, 1).subs(age, aUseful)
    perCPURevUseful = (RwUseful - perCPUCostUseful).subs(subs).subs(params)

# we can now optimize for bestAgeUseful because useful revenue
# does not depend on anything else
    bestAgeUseful = minimize_scalar(lambdify(aUseful, -perCPURevUseful),
                            bounds = (0, MAX_STALENESS),
                            method='bounded').x
    cpuCountUseful = (total_cap / perCPUCostUseful)
    revUseful = (perCPURevUseful * cpuCountUseful).subs(params).subs(aUseful, bestAgeUseful)
    print("Non-mining optimal staleness is %s%.2f%s" % (colors.BOLD, bestAgeUseful, colors.ENDC))

    usefulPrice = (total_cap / cpuCountUseful).subs(aUseful, bestAgeUseful).subs(params)
    usefulPriceBaseline = usefulPrice
    print("Non-mining useful work price is %s%.2f%s" % (colors.BOLD, usefulPrice, colors.ENDC))

# Standard mining:
    perCPUCostStandard = (eCost + overheadStd + cost).subs(u, 1).subs(params).subs(age, aStd)
    perCPURevStandard = (RwStd / overheadSFI + R_b * u * perfSlowdown - perCPUCostStandard).subs(u, 1)
    cpuCountStandard = (total_cap / perCPUCostStandard).subs(params)
    revStandard = (perCPURevStandard * cpuCountStandard).subs(params)

    stdOpRatio = var("stdOpRatio")
    pouwRb = annualCryptoRev / (stdOpRatio * operatorCount * cpuCountStandard * u * perfSlowdown).subs(u, 1)

# revStandard depends on two vars (ratio & aStd) so we can't optimize for
# rather we need to find a equilibrium
    simplified = nsimplify((revUseful - revStandard).subs(R_b, pouwRb), tolerance=1e-5)
    equilibriumstdOpRatio = solve(simplified, stdOpRatio)[0]

# fully expand now
    pouwRb = pouwRb.subs(subs)
    equilibriumstdOpRatio = equilibriumstdOpRatio.subs(subs)

    f = lambda aOthers: aOthers - minimize_scalar(
        lambdify(aStd,
                -(revStandard
                .subs(subs)
                .subs(R_b, pouwRb.subs(aStd, aOthers))
                .subs(stdOpRatio, equilibriumstdOpRatio.subs(aStd, aOthers))
                .subs(params))),
        bounds = (0, MAX_STALENESS),
        method='bounded').x

    bestAgeStd = fsolve(f, MAX_STALENESS / 2)
    print 'bestAgeStd is %.2f' % bestAgeStd

    equilStdOpRatio_v = equilibriumstdOpRatio.subs(aStd, bestAgeStd).subs(params)
    print 'equilStdOpRatio_v is %.2f' % equilStdOpRatio_v

    if equilStdOpRatio_v >= 1:
        equilStdOpRatio_v = 1
        # now we know everyone is doing std mining
        f = lambda aOthers: aOthers - minimize_scalar(
            lambdify(aStd,
                    -(revStandard
                    .subs(subs)
                    .subs(R_b, pouwRb.subs(aStd, aOthers))
                    .subs(stdOpRatio, equilStdOpRatio_v)
                    .subs(aUseful, bestAgeUseful)
                    .subs(params))),
            bounds = (0, MAX_STALENESS),
            method='bounded').x

        bestAgeStd = fsolve(f, MAX_STALENESS / 2)
    elif equilStdOpRatio_v <= 0:
        equilStdOpRatio_v = 0
        # now we know everyone is doing useful work
        bestAgeStd = -1


    print("PoUW standard optimal staleness is %s%.2f%s. " % (colors.BOLD, bestAgeStd, colors.ENDC))
    print("PoUW has %s%.2f standard operation%s at equilibrium. " % (colors.BOLD, equilStdOpRatio_v, colors.ENDC))

    # useful work is done by both useful worker and standard miner
    usefulWorkDone = \
        (((1-equilStdOpRatio_v) * cpuCountUseful * 1) + (equilStdOpRatio_v * cpuCountStandard * 1 * perfSlowdown / overheadSFI))\
        .subs(subs).subs(aStd, bestAgeStd).subs(aUseful, bestAgeUseful)
    usefulPrice = (total_cap / usefulWorkDone).subs(params)

    print("PoUW unit cost for useful work is %s%.2f%s" %
    (colors.BOLD, usefulPrice, colors.ENDC))

    print "Result: annualCryptoRev %.2f operatorCount %.2f aStd %.2f ; aFarming %.2f equilibrium is %.2f" % (
        params["annualCryptoRev"],
        params['operatorCount'],
        bestAgeUseful, bestAgeStd, equilStdOpRatio_v)
    # return params.values() + [bestAgeUseful, bestAgeStd, equilStdOpRatio_v, usefulPrice / usefulPriceBaseline]
    return (params["annualCryptoRev"], params["operatorCount"], bestAgeUseful, bestAgeStd, equilStdOpRatio_v, usefulPrice / usefulPriceBaseline)

def varyAnnaulCryptoRev():
    params = []
    for an in np.linspace(1000, 500000, 50):
        tmp = copy.copy(valSubs)
        tmp["annualCryptoRev"] = an
        params.append(tmp)

    pool = Pool(processes=cpu_count())
    results = np.array(pool.map(pouw, params))
    np.savetxt('../oakland/figs/pouw-rev-2.txt', results, fmt='%.4f',header='aUseful, aStd, equilStdOpRatio_v, usefulPrice')

def varyOverheadSFI():
    params = []
    an = 50000
    for osfi in np.linspace(1, 2, 20):
        tmp = copy.copy(valSubs)
        tmp["overheadSFI"] = osfi
        tmp["annualCryptoRev"] = an
        params.append(tmp)

    pool = Pool(processes=cpu_count())
    results = np.array(pool.map(pouw, params))
    np.savetxt('../oakland/figs/%d-osfi.txt' % an, results, fmt='%.4f',header='aUseful, aStd, equilStdOpRatio_v, usefulPrice')

def varyRw():
    params = []
    # for a in np.linspace(1, 4, 5):
    #     for an in np.linspace(1000, 50000000, 20):
    #         tmp = copy.copy(valSubs)
    #         tmp["RwBase"] *= a
    #         tmp["annualCryptoRev"] = an
    #         params.append(tmp)

    tmp = copy.copy(valSubs)
    tmp["RwBase"] *= 2
    tmp["annualCryptoRev"] = 500
    params.append(tmp)

    pool = Pool(processes=cpu_count())
    results = np.array(pool.map(pouw, params))
    np.savetxt('../oakland/figs/pouw-rw.txt', results, fmt='%.4f',header='aUseful, aStd, equilStdOpRatio_v, usefulPrice')


if __name__ == '__main__':
    varyAnnaulCryptoRev()
    # varyOverheadSFI()
    # varyRw()
