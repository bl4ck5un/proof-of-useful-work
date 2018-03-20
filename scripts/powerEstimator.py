#!/usr/bin/python 

import random 
from math import * 
from scipy.stats import chi2

freq = 20.0            # 1/sec 
slotDuration = 0.001  # sec 
expTime = 20050        # sec 

def genProb(t): 
    #return 0.01
    if floor(t) % 3 == 0: 
        return 0.0008 
    
    if floor(t) % 3 == 1: 
        return 0.0004 

    if floor(t) % 3 == 2: 
        return 0.0006 

# http://stackoverflow.com/a/14832525/385482 
def poissonInterval(k, alpha=0.05): 
    """
    uses chisquared info to get the poisson interval. Uses scipy.stats 
    (imports in function). 
    """
    from scipy.stats import chi2
    a = alpha
    low, high = (chi2.ppf(a/2, 2*k) / 2, chi2.ppf(1-a/2, 2*k + 2) / 2)
    if k == 0: 
        low = 0.0
    return low, high

rand = random.Random() 
n = 0 # Block count in range 
volume = 0 
for i in xrange(int(float(expTime) / slotDuration)): 
    volume += genProb(i * slotDuration) * slotDuration 

#deltas = [] 
#tPrev = 0 
for i in xrange(int(float(expTime) * freq)): 
    prob = genProb(i / freq) # TODO Makes sense? 
    if rand.random() < prob: 
        #print("%.2f, %.2f" % (tPrev, i / freq))
        n += 1 
        #deltas.append(i / freq - tPrev) 
        #tPrev = i / freq 

alpha = 0.05 
z = 1.96 # 95% 
# alpha = 5% ==> 2.5% = 0.025, 2n degrees of freedom

#avgDelta = float(sum(deltas)) / len(deltas) 
#confLower = 2 * n * avgDelta / chi2.ppf(1.0 - alpha / 2.0, 2 * n)
#confUpper = 2 * n * avgDelta / chi2.ppf(alpha / 2.0      , 2 * n)

#print avgDelta

n = float(n)
#confLower = n / volume * (1 - 1.0 / (9.0 * n) - z / (3.0 * sqrt(n))) ** 3.0 
#confUpper = (n + 1.0) / volume * (1.0 - 1.0 / (9.0 * (n + 1.0)) - z / (3.0 * sqrt(n + 1.0))) ** 3.0 
l, h = poissonInterval(n, alpha) 
l, h = l / volume, h / volume
#confLower = n / volume * (1 - 1.0 / (9.0 * n) - z / (3.0 * sqrt(n))) ** 3.0 
#confUpper = (n + 1.0) / volume * (1.0 - 1.0 / (9.0 * (n + 1.0)) - z / (3.0 * sqrt(n + 1.0))) ** 3.0 
#estimate = float(n) / volume

print("Block count: %d" % n) 
#print("Freq estimate: %.3f" % estimate) 
print("Frequency bounds: %.4f <? %.4f <? %.2f" % 
      (l, freq, h)) 
