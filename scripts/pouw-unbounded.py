import pouw
from revenues import *
import copy
from multiprocessing import Pool, cpu_count


def pouw(params):
    # Note: R_b here is per calculation power unit, not cpu count, so multiplied by
    # the performance slowdown and the cpu utilization.

    aUseful, aStd = var("aUseful, aStd")
    revUseful, _ = useful_work(params)
    RwStd, RwUseful = var("RwStd, RwUseful")
    perfSlowdown = var("perfSlowdown")
    subs = [
        (RwStd, R_w.subs(age, aStd).subs(u, 1)),
        (RwUseful, R_w.subs(age, aUseful).subs(u, 1)),
        (perfSlowdown, perfSlowdownExpression.subs(age, aStd))
    ]

    # Standard mining:
    perCPUCostStandard = (eCost + overheadStd + cost).subs(u, 1).subs(params).subs(age, aStd)
    # perCPURevUseful = (R_w - perCPUCostUseful).subs(u, 1).subs(params).subs(age, aUseful)
    perCPURevStandard = (RwStd * overheadSFI + R_b * u * perfSlowdown - perCPUCostStandard).subs(u, 1)
    cpuCountStandard = (total_cap / perCPUCostStandard).subs(params)
    revStandard = (perCPURevStandard * cpuCountStandard).subs(params)

    n_std_operator_sym = var("n_std_operator_sym")
    pouwRb = annualCryptoRev / (n_std_operator_sym * cpuCountStandard * u * perfSlowdown).subs(u, 1)

    # revStandard depends on two vars (ratio & aStd) so we can't optimize for
    # rather we need to find a equilibrium
    simplified = nsimplify((revUseful - revStandard).subs(R_b, pouwRb), tolerance=1e-5)
    n_std_operator_opt = solve(simplified, n_std_operator_sym)[0]

    # fully expand now
    pouwRb = pouwRb.subs(subs)
    n_std_operator_opt = n_std_operator_opt.subs(subs)

    # f = lambda aOthers: aOthers - minimize_scalar(
    #     lambdify(aStd,
    #             -(revStandard
    #             .subs(subs)
    #             .subs(R_b, pouwRb.subs(aStd, aOthers))
    #             .subs(n_std_operator_sym, n_std_operator_opt.subs(aStd, aOthers))
    #             .subs(params))),
    #     bounds = (0, MAX_STALENESS),
    #     method='bounded').x

    # bestAgeStd = fsolve(f, MAX_STALENESS / 2)[0]
    bestAgeStd = 3.48
    print 'bestAgeStd is %.2f' % bestAgeStd

    n_std_operator_opt = n_std_operator_opt.subs(aStd, bestAgeStd).subs(params)
    print 'equilStdOpRatio_v is %.2f' % n_std_operator_opt

    pouwRb_v = pouwRb.subs(params).subs(aStd, bestAgeStd).subs(n_std_operator_sym, n_std_operator_opt)

    print("R_b is %.2f" % pouwRb_v)
    print("PoUW standard optimal staleness is %s%.2f%s. " % (colors.BOLD, bestAgeStd, colors.ENDC))
    print("PoUW has %s%.2f standard operation%s at equilibrium. " % (colors.BOLD, n_std_operator_opt, colors.ENDC))

    revenue = (revStandard.subs(subs).subs(R_b, pouwRb_v).subs(n_std_operator_sym, n_std_operator_opt).subs(params).subs(aStd, bestAgeStd))
    print "Revenue is %.2f" % revenue

    return (params["overheadSFI"], n_std_operator_opt)


def varyOverheadSFI(annualCryptoRev=-1.0):
    params = []
    for osfi in np.linspace(.6, .99, 40):
        tmp = copy.copy(valSubs)
        tmp["overheadSFI"] = osfi
        if annualCryptoRev > 0:
            tmp["annualCryptoRev"] = annualCryptoRev
        params.append(tmp)

    pool = Pool(processes=cpu_count())
    results = np.array(pool.map(pouw, params))
    np.savetxt('../oakland/figs/unbounded/%d-osfi.txt' % annualCryptoRev, results, fmt='%.4f')


if __name__ == '__main__':
    varyOverheadSFI(3e6)
    varyOverheadSFI(3e7)
    varyOverheadSFI(3e8)
    varyOverheadSFI(3e9)