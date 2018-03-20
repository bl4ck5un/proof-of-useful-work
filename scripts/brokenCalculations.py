#!/usr/bin/python

import random
import sys
from scipy.optimize import minimize
from scipy.stats import chi2, poisson
from numpy import linspace, isnan


# http://stackoverflow.com/a/14832525/385482
def poissonInterval(k, alpha=0.05, volume=1):
    """
    uses chisquared info to get the poisson interval. Uses scipy.stats
    (imports in function).
    """
    from scipy.stats import chi2
    a = alpha
    low, high = (chi2.ppf(a / 2, 2 * k) / 2, chi2.ppf(1 - a / 2, 2 * k + 2) / 2)
    if k == 0:
        low = 0.0
    if volume > 0:
        return low / volume, high / volume
    else:
        return low, high


def genAttackBlocks(alpha, p, fBest, rateIncreaseRatio, tMax):
    blocks = [0]  # First block at t = 0
    while blocks[-1] < tMax:
        constraints = ({"type": "ineq",
                        "fun": lambda x: fBest * rateIncreaseRatio - poissonInterval(len(blocks), alpha, p * x)[0]},
                       {"type": "ineq",
                        "fun": lambda x: x - blocks[-1]})
        res = minimize(fun=lambda x: x,
                       x0=blocks[-1] + 0.1,
                       constraints=constraints,
                       method="SLSQP")
        if isnan(res.x[0]):
            print("Failed at block %d" % len(blocks))
            print res
        blocks.append(res.x[0])

    return blocks


def run(p, tMax, fBest, rateIncreaseRatio, outputFilename):
    alpha = 0.005

    badBlocks = genAttackBlocks(alpha, p, fBest, rateIncreaseRatio, tMax)

    # for i in xrange(len(badBlocks)):
    # print("Block at %.2f, bounds: (%.2f, %.2f)" %
    # ((badBlocks[i],) +
    # poissonInterval(i, alpha, p * badBlocks[i])))
    # print("Actual rate: %.2f" % (len(badBlocks) / badBlocks[-1]))

    # Generate CSV
    output = ""

    output += "t, GoodFreqLow, GoodFreqAvg, GoodFreqHigh, BadFreqAvg, AdvantageRatio\n"
    pointCount = 100
    badBlockI = 0
    for t in linspace(0, badBlocks[-2], num=pointCount)[1:]:
        while badBlocks[badBlockI + 1] <= t:
            badBlockI += 1
        output += "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n" % (
            t,  # Current time
            poisson.ppf(0.05, t * fBest * p),  # Number of good blocks: lower bound
            round(t * fBest * p),  # Rounded number of good blocks estimate
            poisson.ppf(0.95, t * fBest * p),  # Number of good blocks: upper bound
            round(badBlockI),  # Number of bad blocks
            float(badBlockI) / (t * fBest * p)  # Advantage ratio
        )

    outputFile = open(outputFilename, "w")
    outputFile.write(output)
    outputFile.close()


if __name__ == "__main__":
    # Daily calculation:
    # fBest = 10 ** 9     # 1/sec
    # p = 1.0 / (fBest * 60 * 60 * 24) # Once a day
    # tMax = 365 * 60 * 60 * 24

    # filename = "brokenStatsDaily.csv"
    # print("%s..." % filename)
    # run(p, tMax, fBest, 1.0, filename)

    # Annual calculation:
    # secondsPerYear = 60 * 60 * 24 * 365
    secondsPerYear = 10.0 ** 3  # To avoid numerical error; no effect on anything by X axis
    fBest = 10 ** 9  # 1/sec
    p = 1.0 / (fBest * secondsPerYear)  # Once every 1 years
    tMax = secondsPerYear * 5

    # Estimate upper bound rate based on 3 samples, one every year:
    bestLowerBoundRate = poissonInterval(k=2, alpha=0.05, volume=5)[0]
    increaseRatio = (2.0 / 5) / bestLowerBoundRate
    print increaseRatio

    filename = "brokenStatsAnnual.csv"
    print("%s..." % filename)
    # 1.0/bestLowerBoundRate due to the lack of tightness of the upper bound estimation
    run(p, tMax * 1.1, fBest, increaseRatio, filename)
