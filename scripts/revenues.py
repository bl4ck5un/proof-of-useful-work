import numpy as np
from sympy import *
from sympy.solvers import *
from scipy.optimize import minimize_scalar
from scipy.optimize import curve_fit
from scipy.optimize import fsolve
from tabulate import tabulate
import math
import sys
from colors import colors
import pandas as pd
import matplotlib.pyplot as plt

###############################################################################
# Symbols, functions, and substitution arrays:

E_min, E_max, eta, total_cap, u, overheadStd, \
overheadFarm, overheadPoET, overheadSFI, R_b, beta, annualCryptoRev, \
operatorCount = var("E_min, E_max, eta, total_cap, u, overheadStd, overheadFarm, overheadPoET, overheadSFI, R_b, beta, annualCryptoRev, operatorCount")

age = var("age")
RwBase = var("RwBase")

MAX_STALENESS = 10
valSubs = {
    "E_min": .030 * 24 * 365 * .11, # idle, 45W & 11cent/kwh
    "E_max": .165 * 24 * 365 * .11, # 165W & 11cent/kwh
    "eta": 1,                       # TODO: E parameter
    "total_cap": 10000,             # Amount available per year USD/year]
    "overheadStd": 100,             # TODO: Find reasonable values
    "overheadFarm": 10,             # TODO: Find reasonable values
    "annualCryptoRev": 3000000,
    "operatorCount": 100,
    "RwBase": .129*365*24,
    "overheadSFI": 1.1, # Secure operations per regular operation
}

cpuInfo = pd.read_csv('../oakland/cpus.csv')


staleness = 16.75 - cpuInfo[cpuInfo.columns[2]].values
# prices are taken from https://wefound.en.alibaba.com

prices = cpuInfo[cpuInfo.columns[3]].values
# perf scores are taken from https://www.cpubenchmark.net
def expoential_fun(s, a, b, c):
    return a * np.exp(-b * s) + c

# curve_fit to get price-age curve
p_price, _ = curve_fit(expoential_fun, staleness, prices, p0=(500, 0, 0))

cost = (p_price[0] * exp(1) ** (-p_price[1] * age) + p_price[2])
eCost = (E_min + u*(E_max - E_min) )
perfSlowdownExpression = 1/(1+exp(-5.75+.88*age))
R_w = (RwBase * u * perfSlowdownExpression)

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

## Useful work is shared by all schemes so let's just put it here:
def useful_work(params, isPrint=False):
    aUseful = var("aUseful")
    # there will be infinitely many useful works
    perCPUCostUseful = (eCost + overheadStd + cost).subs(u, 1).subs(age, aUseful)
    perCPURevUseful = (R_w - perCPUCostUseful).subs(u, 1).subs(params).subs(age, aUseful)

    # we can optimize for bestAgeUseful because it doesn't depend on anything else
    # in the world
    bestAgeUseful = minimize_scalar(lambdify(Symbol("aUseful"), -perCPURevUseful),
        bounds = (0, MAX_STALENESS),
        method='bounded').x

    if isPrint:
        print 'best useful age is %.2f' % bestAgeUseful
        print 'useful work cost is %.2f' % perCPUCostUseful.subs(params).subs(aUseful, bestAgeUseful)

    cpuCountUseful = (total_cap / perCPUCostUseful).subs(params).subs(aUseful, bestAgeUseful)
    revUseful = (perCPURevUseful * cpuCountUseful).subs(params).subs(aUseful, bestAgeUseful)
    usefulPriceBaseline = (total_cap / cpuCountUseful).subs(params).subs(aUseful, bestAgeUseful)
    # make sure revUseful is now a number
    assert (type(revUseful) == Float)
    assert (type(usefulPriceBaseline) == Float)

    return (revUseful, usefulPriceBaseline)
