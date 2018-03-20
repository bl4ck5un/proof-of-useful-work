import bisect

import numpy as np
from scipy.stats import poisson


def rejectPolicy(n, rate_max, time, alpha):
    plambda = rate_max * time
    threshold = poisson.ppf(1-alpha, plambda)

    return n > threshold


def simulation(alpha, tmax, nround=5):
    fBest = 10 ** 9  # guess per sec.
    p = 1.0 / (fBest * 3600)  # one block every hour
    rate = fBest * p

    time_stamps = np.linspace(0, tmax, 200)
    adversary_blocks = [poisson.ppf(1-alpha, rate*t) for t in time_stamps]

    tmp_accept_blocks = []
    tmp_reject_blocks = []
    for r in range(nround):
        t = 0
        accepted = 0
        reject = 0
        accept_events = dict()
        reject_events = dict()
        n_rejected_blks = []
        n_accepted_blks = []
        while t < tmax:
            interval = np.random.exponential(1.0 / rate)
            t += interval
            if rejectPolicy(accepted, rate, t, alpha):
                reject += 1
            else:
                accepted += 1
            reject_events[t] = reject
            accept_events[t] = accepted

        sample_time = sorted(accept_events.keys())
        for t in time_stamps:
            # find a j such that honest_blocks_samples.keys[j] >= t && [j+1] < t
            j = bisect.bisect_left(sample_time, t, hi=len(sample_time)-1)
            n_accepted_blks.append(accept_events[sample_time[j]])

        sample_time = sorted(reject_events.keys())
        for t in time_stamps:
            j = bisect.bisect_left(sample_time, t, hi=len(sample_time)-1)
            n_rejected_blks.append(reject_events[sample_time[j]])

        tmp_reject_blocks.append(n_rejected_blks)
        tmp_accept_blocks.append(n_accepted_blks)

    accept_blocks_avg = np.mean(tmp_accept_blocks, axis=0)
    reject_blocks_avg = np.mean(tmp_reject_blocks, axis=0)


    advantage = [float(adv) / honest for (adv, honest) in zip(adversary_blocks, accept_blocks_avg)]
    waste = [float(reject) / (reject + accept) for (reject, accept) in zip(reject_blocks_avg, accept_blocks_avg)]
    return time_stamps, advantage, waste, adversary_blocks, accept_blocks_avg, reject_blocks_avg

secondsPerDay = 60 * 60 * 24.0
secondsPerYear = secondsPerDay * 365
tmax = 60 * secondsPerDay
params = (.6, 0.4, 0.2, 0.1)

for alpha in params:
    time_stamps, advantage, waste, adversary_blocks, accepted, rejected = simulation(alpha, tmax, 200)
    time_stamps /= secondsPerDay
    a = np.array([time_stamps, advantage, waste, adversary_blocks, accepted, rejected]).T
    np.savetxt('../paper/figs/mc/brokenChipAdvAndWaste_alpha=%f' % alpha, a, fmt="%.6f", header="time, adv, waste, attacker_blks, honest_blks, rejected_blks")
