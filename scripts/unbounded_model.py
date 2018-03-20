from revenues import *
import copy
from multiprocessing import Pool, cpu_count
import sys
from sympy.core.numbers import Float

aStd, aFarming = var("aStd, aFarming")
# these symbols are declared here in order to differentiate standard
# operation from mining, because they have different ages.
eCostStd, eCostFarming, costStd, RwStd = var("eCostStd, eCostFarming, costStd, RwStd")
subs = [
    (eCostStd, eCost.subs(u, 1)),
    (eCostFarming, eCost.subs(u, 0)),
    (costStd, cost.subs(age, aStd)),
    (RwStd, R_w.subs(age, aStd).subs(u, 1))
]

from scipy.optimize import newton


class EquilibriumPoint:
    def __init__(self, num_farmer, num_std, utility_farming, utility_std, opt_age_farming, opt_age_std, num_cpu_farming,
                 num_cpu_std, block_revenue):
        self.num_farmer = num_farmer
        self.num_std = num_std
        self.utility_farming = utility_farming
        self.utility_std = utility_std

        self.opt_age_farming = opt_age_farming
        self.opt_age_std = opt_age_std

        self.num_cpu_farming = num_cpu_farming
        self.num_cpu_std = num_cpu_std

        self.per_cpu_cost_farming = -1
        self.per_cpu_cost_std = -1

        self.block_revenue = block_revenue

    def __str__(self):
        return 'Equilibrium: %.2f farmers (@%.2f, CPU=%.2f, <= %.2f), %.2f std miners (@%.2f, CPU=%.2f, <= %.2f), rb is %f' % (
            self.num_farmer, self.opt_age_farming, self.num_cpu_farming, self.utility_farming,
            self.num_std, self.opt_age_std, self.num_cpu_std, self.utility_std,
            self.block_revenue)


class EquilibriumFinder:
    params_m = dict()

    farming_price_dist = False

    def __init__(self, params):
        self.params = params
        self.ALL_FARMING = 1
        self.ALL_STD_MINING = 2
        self.SOMETHING_ELSE = 3

        self.n_std_miner_sym = var("numStdMiner")
        self.n_farmer_sym = var("numFarmer")
        self.individual_age_sym = var("aSingleMiner")
        self.cost_coef_sym = var("costCoefficiency")
        self.overhead_poet_sym = var("overheadPoET")
        self.overhead_farming_sym = var("overheadFarming")

        # self.overhead_poet = .9
        # self.overhead_farming = 10.0
        # self.revenue_useful_work = 10000.0
        self.revenue_useful_work = 10000

        # definitions
        self.perCPUCostStandard = (eCostStd + overheadStd + costStd)
        self.perCPURev = (self.overhead_poet_sym * RwStd + R_b - self.perCPUCostStandard)
        self.cpuCountStandard = (total_cap / self.perCPUCostStandard).subs(params)
        self.revStandard = (self.perCPURev * self.cpuCountStandard)

        if self.farming_price_dist:
            self.perCPUCostFarm = self.overhead_farming_sym * self.cost_coef_sym
        else:
            self.perCPUCostFarm = self.overhead_farming_sym
        self.perCPURevFarm = (R_b - self.perCPUCostFarm).subs(subs)
        self.cpuCountFarm = (total_cap / self.perCPUCostFarm).subs(subs).subs(params)
        self.revFarm = (self.perCPURevFarm * self.cpuCountFarm).subs(subs)

        mining_revenue_sym = annualCryptoRev / (self.n_std_miner_sym * self.cpuCountStandard + self.n_farmer_sym * self.cpuCountFarm)
        self.mining_revenue_sym = mining_revenue_sym.subs(subs).subs(params)

    def set_params(self, params):
        self.params = params

    def get_cost_coef(self, num):
        stepStart = 100
        stepStop = 1000
        slope = 1 / (stepStop - stepStart)
        intercept = -stepStart * slope

        if num == 0:
            return 1.0

        if num < stepStart:
            return .001

        mid = np.linspace(0, 1, num=stepStop - stepStart + 1)
        if num > stepStop:
            return .001 + (sum(mid) + (num - stepStop)) / num

        return .001 + sum(mid[:num - stepStart]) / num

    def only_farmers(self, overhead_poet, overhead_farming, revenue_useful_work):
        self.revenue_useful_work = revenue_useful_work

        self.params_m = (
            (self.overhead_poet_sym, overhead_poet),
            (self.overhead_farming_sym, overhead_farming),
        )
        singleFarmerUtility = self.revFarm.subs(R_b, self.mining_revenue_sym).subs(self.n_std_miner_sym, 0).subs(self.params_m)

        # since we assume the CPU age is very old so the only condition here is:
        condRevFarmingEqUseful = singleFarmerUtility - self.revenue_useful_work

        def numericalCondRevFarmingEqUseful(num):
            return condRevFarmingEqUseful.subs(self.n_farmer_sym, num).subs(self.cost_coef_sym,
                                                                            self.get_cost_coef(num))

        # solve for num of farmers
        numFarmer_v = newton(numericalCondRevFarmingEqUseful, 100)
        # now we can calculate Rb
        busypoetRb_v = self.mining_revenue_sym.subs(self.n_std_miner_sym, 0).subs(self.n_farmer_sym, numFarmer_v).subs(self.params).subs(self.params_m).subs(
            self.cost_coef_sym,
            self.get_cost_coef(
                numFarmer_v))
        # and the utility for a single farmer
        utilityFarming = self.revFarm.subs(R_b, busypoetRb_v).subs(self.params).subs(self.params_m).subs(self.cost_coef_sym,
                                                                                self.get_cost_coef(numFarmer_v))

        # now examine a standard miner's revenue.
        singleMinerUtility = self.revStandard.subs(subs).subs([
            (aStd, self.individual_age_sym),
            (R_b, busypoetRb_v),
            (self.cost_coef_sym, self.get_cost_coef(numFarmer_v))]).subs(self.params).subs(self.params_m)

        # find the best age for std miners
        bestAgeStd = minimize_scalar(lambdify(self.individual_age_sym, -singleMinerUtility),
                                     bounds=(0, MAX_STALENESS), method='bounded').x

        utilityStd = singleMinerUtility.subs(self.individual_age_sym, bestAgeStd).subs(self.params).subs(self.params_m)
        cpuCountStandard_v = self.cpuCountStandard.subs(subs).subs(self.params).subs(aStd, bestAgeStd).subs(self.params_m)
        cpuCountFarm_v = self.cpuCountFarm.subs(self.cost_coef_sym, self.get_cost_coef(numFarmer_v)).subs(self.params_m)

        # collect results
        eq = EquilibriumPoint(numFarmer_v, 0, utilityFarming, utilityStd, -1, bestAgeStd,
                              cpuCountFarm_v, cpuCountStandard_v, busypoetRb_v)

        eq.per_cpu_cost_std = self.perCPUCostStandard.subs(subs).subs(params).subs(aStd, bestAgeStd)
        eq.per_cpu_cost_farming = self.perCPUCostFarm.subs(self.params_m)

        if utilityStd < self.revenue_useful_work:
            return True, eq
        else:
            return False, eq

    def only_miners(self, overhead_poet, overhead_farming, revenue_useful_work):
        self.revenue_useful_work = revenue_useful_work

        self.params_m = (
            (self.overhead_poet_sym, overhead_poet),
            (self.overhead_farming_sym, overhead_farming),
        )
        n_free_farmers = 0

        singleMinerUtility = self.revStandard \
            .subs(subs) \
            .subs(R_b, self.mining_revenue_sym) \
            .subs(self.n_farmer_sym, n_free_farmers) \
            .subs(aStd, self.individual_age_sym) \
            .subs(self.params) \
            .subs(self.params_m) \
            .subs(self.cost_coef_sym, self.get_cost_coef(n_free_farmers))

        # condAgeStdOptimal = diff(singleMinerUtility, self.individual_age_sym).subs(self.individual_age_sym, aStd)
        bestAgeStd = 3.48
        condRevStdEqUseful = singleMinerUtility.subs(self.individual_age_sym, bestAgeStd) - self.revenue_useful_work
        numStdMiner_v, = solve(condRevStdEqUseful, self.n_std_miner_sym)

        # def twoConditions(arg):
        #     x, y = arg
        #     x = min(MAX_STALENESS, x)
        #     y = abs(y)
        #     return (condAgeStdOptimal.subs([(aStd, x), (self.n_std_miner_sym, y)]),
        #             condRevStdEqUseful.subs([(aStd, x), (self.n_std_miner_sym, y)]))
        #
        # bestAgeStd, numStdMiner_v = fsolve(twoConditions, [5, 100])
        # now we can compute Rb
        busypoetRb_v = self.mining_revenue_sym\
                       .subs(self.n_std_miner_sym, numStdMiner_v) \
                       .subs(self.n_farmer_sym, n_free_farmers).subs(aStd, bestAgeStd) \
                       .subs(self.cost_coef_sym, self.get_cost_coef(n_free_farmers)).subs(self.params_m)
        utilityStd = self.revStandard.subs(R_b, busypoetRb_v).subs(subs).subs(self.params).subs(aStd, bestAgeStd).subs(self.params_m)
        utilityFarming = self.revFarm.subs(subs).subs(self.params).subs(R_b, busypoetRb_v).subs(self.cost_coef_sym,
                                                                                           self.get_cost_coef(n_free_farmers)).subs(self.params_m)

        cpuCountStandard_v = self.cpuCountStandard.subs(subs).subs(self.params).subs(aStd, bestAgeStd).subs(self.params_m)
        cpuCountFarm_v = self.cpuCountFarm.subs(self.cost_coef_sym, self.get_cost_coef(n_free_farmers)).subs(self.params_m)



        eq = EquilibriumPoint(n_free_farmers, numStdMiner_v, utilityFarming, utilityStd, -1, bestAgeStd,
                              cpuCountFarm_v, cpuCountStandard_v, busypoetRb_v)

        eq.per_cpu_cost_std = self.perCPUCostStandard.subs(subs).subs(params).subs(aStd, bestAgeStd)
        eq.per_cpu_cost_farming = self.perCPUCostFarm.subs(self.params_m)

        if utilityFarming < self.revenue_useful_work:
            return True, eq
        else:
            return False, eq

    def mixed_equilibrium(self):
        singleMinerUtility = self.revStandard \
            .subs(subs) \
            .subs(aStd, self.individual_age_sym) \
            .subs(R_b, self.mining_revenue_sym) \
            .subs(self.params) \
            .subs(self.params_m)

        condAgeStdOptimal = diff(singleMinerUtility, self.individual_age_sym).subs(self.individual_age_sym, aStd)
        condRevStdEqUseful = singleMinerUtility.subs(self.individual_age_sym, aStd) - self.revenue_useful_work

        singleFarmerUtility = self.revFarm.subs(R_b, self.mining_revenue_sym).subs(self.params_m)
        condRevFarmingEqUseful = singleFarmerUtility - self.revenue_useful_work

        def threeConditions(args):
            num_std_miner, num_farmer, age_std = args
            r1 = condAgeStdOptimal.subs(self.n_farmer_sym, num_farmer)\
                .subs(self.cost_coef_sym, self.get_cost_coef(num_farmer)).subs(self.n_std_miner_sym, num_std_miner)\
                .subs(aStd, age_std)

            r2 = condRevStdEqUseful.subs(self.n_farmer_sym, num_farmer)\
                .subs(self.cost_coef_sym, self.get_cost_coef(num_farmer)).subs(self.n_std_miner_sym, num_std_miner)\
                .subs(aStd, age_std)

            r3 = condRevFarmingEqUseful.subs(self.n_farmer_sym, num_farmer)\
                .subs(self.cost_coef_sym, self.get_cost_coef(num_farmer)).subs(self.n_std_miner_sym, num_std_miner)\
                .subs(aStd, age_std)

            return (r1, r2, r3)

        num_std_miner, num_farmer, age_std = fsolve(threeConditions, (10, 10, 10))
        return False, None

    def find_equilibrium(self, overhead_poet, overhead_farming, revenue_useful_work):
        self.revenue_useful_work = revenue_useful_work

        self.params_m = (
            (self.overhead_poet_sym, overhead_poet),
            (self.overhead_farming_sym, overhead_farming),
        )

        assert 0 <= overhead_poet <= 1

        find_solution, eq = self.only_farmers(overhead_poet, overhead_farming, revenue_useful_work)
        if find_solution:
            return self.ALL_FARMING, eq
        find_solution, eq = self.only_miners(overhead_poet, overhead_farming, revenue_useful_work)
        if find_solution:
            return self.ALL_STD_MINING, eq
        return self.SOMETHING_ELSE, None

    def find_threshold_low(self, overhead_std, revenue_useful_work):
        l = .1
        r = 1e4
        eq = None
        while abs(l - r) > 1:
            m = (l + r) / 2
            result, eq = self.find_equilibrium(overhead_std, m, revenue_useful_work)
            if result == self.ALL_FARMING:
                l = m
            elif result == self.ALL_STD_MINING or result == self.SOMETHING_ELSE:
                r = m

        return (l + r) / 2, eq

    def find_threshold_high(self, overhead_std, revenue_useful_work):
        l = .1
        r = 1e4
        eq = None
        while abs(l - r) > 1:
            m = (l + r) / 2
            result, eq = self.find_equilibrium(overhead_std, m, revenue_useful_work)
            if result == self.ALL_FARMING or result == self.SOMETHING_ELSE:
                l = m
            elif result == self.ALL_STD_MINING:
                r = m

        return (l + r) / 2, eq


params = valSubs
# using Bitcoin annual revenue data
params['annualCryptoRev'] = 248.87 * 25 * 365 * 24 * 6
revUseful, usefulPriceBaseline = useful_work(params)
oStdRange = np.linspace(.60, 1.0, endpoint=False, num=20)
equilibriumPoints = []
solver = EquilibriumFinder(params)
for oStd in oStdRange:
    _, eq = solver.only_miners(oStd, 100, revUseful)
    equilibriumPoints.append(eq)
    print oStd, eq.num_std * eq.num_cpu_std


# thresholdPoints = []
# thresholdHighPoints = []
# threshold_x = np.linspace(.6, 1.0, num=50)
# for oStd in threshold_x:
#     t_low, eq = solver.find_threshold_low(oStd, revUseful)
#     t_high, eq = solver.find_threshold_high(oStd, revUseful)
#     # print 'Threshold for %.2f is %.2f' % (oStd, threshold)
#     thresholdPoints.append(t_low)
#     thresholdHighPoints.append(t_high)
#     print oStd, t_low, t_high

# plt.subplot(2, 1, 1)
# plt.plot(threshold_x, thresholdPoints, 'bo-')
# plt.plot(threshold_x, thresholdHighPoints, 'bo-')
# plt.xlabel('efficiency of PoET')
# plt.ylabel('Threshold for farming')
#
# plt.subplot(2, 1, 2)
# plt.plot(oStdRange, [p.num_std * p.num_cpu_std for p in equilibriumPoints], 'bo')
# plt.xlabel('efficiency of PoET')
# plt.ylabel('number of mining CPUs')
# plt.show()
