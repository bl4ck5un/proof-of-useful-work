import numpy as np


def rejectPolicy(n, rate_max, time, alpha):
    from scipy.stats import poisson
    plambda = rate_max * time
    threshold = poisson.ppf(1 - alpha, plambda)

    reject = n > threshold
    if (reject):
        # print 'n vs threshold: %d %d' % (n, threshold)
        # print poisson.pmf(threshold, plambda)
        # print poisson.pmf(threshold + 1, plambda)
        pass

    return reject


def __simulate_once(alpha):
    fBest = 10 ** 9  # guess per sec.
    p = 1.0 / (fBest * 3600)  # one block every hour
    rate = fBest * p

    accepted = 0
    reject = 0

    for i in range(10000):
        interval = np.random.exponential(1.0 / rate)
        if rejectPolicy(1, rate, interval, alpha):
            reject += 1
        else:
            accepted += 1

    return accepted, reject


def __simulate_all_history(alpha):
    secondsPerDay = 60 * 60 * 24.0
    secondsPerYear = secondsPerDay * 365

    fBest = 10 ** 9  # guess per sec.
    p = 1.0 / (fBest * 3600)  # one block every 10 mins
    rate = fBest * p

    t = 0
    tMax = 10 * secondsPerYear
    accepted = 0
    reject = 0
    while t < tMax:
        interval = np.random.exponential(1.0 / rate)
        t += interval
        if rejectPolicy(accepted, rate, t, alpha):
            reject += 1
        else:
            accepted += 1

    return accepted, reject


def __simulate_last_epoch(alpha):
    secondsPerDay = 60 * 60 * 24.0
    secondsPerYear = secondsPerDay * 365

    fBest = 10 ** 9  # guess per sec.
    p = 1.0 / (fBest * 3600)  # one block every 10 mins
    rate = fBest * p

    t = 0
    tMax = 10 * secondsPerYear
    accepted = [0]
    reject = 0
    while t < tMax:
        interval = np.random.exponential(1.0 / rate)
        t += interval
        if rejectPolicy(1, rate, t - accepted[-1], alpha):
            reject += 1
        else:
            accepted.append(t)

    return len(accepted), reject


def run(__simulate, alpha=0.05):
    ratio = []
    kRounds = 5
    for i in range(kRounds):
        accepted, reject = __simulate(alpha)
        total = accepted + reject
        ratio.append(float(reject) / total)

    print '  alpha=%.4f%%, rejection ratio %.4f%% (%.4f%%)' % (100 * alpha, 100 * np.mean(ratio), 100 * np.std(ratio))


print 'Simulate OneBlockPerLife'
# run(__simulate_once)
print 'Simulate long term rejection rate (based on last epoch)'
# run(__simulate_last_epoch)
print 'Simulate long term rejection rate (based on the entire history)'
run(__simulate_all_history, 0.8)
run(__simulate_all_history, 0.6)
run(__simulate_all_history, 0.4)
run(__simulate_all_history, 0.2)
run(__simulate_all_history, 0.1)
run(__simulate_all_history, 0.05)
