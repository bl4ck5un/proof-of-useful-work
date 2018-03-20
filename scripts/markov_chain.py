
from discreteMarkovChain import markovChain
from scipy.stats import poisson
from collections import OrderedDict
import numpy as np

n_state = 20
mu = 1
alpha = .8


def mc(alpha):
    min_positive_state = max(1, int(poisson.ppf(1 - alpha, mu)))

    def state_to_index(state):
        # states are in the range [-n_state, n_state] but array indices begin at 0
        return state + n_state + 1

    def index_to_state(index):
        return -n_state + index - 1

    trans = dict()
    for i in range(-n_state, n_state + 1):
        trans[i] = dict()
        for j in range(-n_state, n_state + 1):
            trans[i][j] = .0

    # Pr[i->j] where i is non-positive
    for i in range(-n_state, min_positive_state):
        for j in range(-n_state, n_state + 1):
            if j >= i - 1:
                trans[i][j] = poisson.pmf(j - i + 1, mu)

    trans[-n_state][-n_state] += 1 - sum(trans[-n_state].values())

    for i in range(min_positive_state, n_state + 1):
        for j in range(-1, n_state):
            trans[i][j] = poisson.pmf(j + 1, mu)


    trans_matrix = []
    for i in range(-n_state, n_state + 1):
        od = OrderedDict(sorted(trans[i].items()))
        trans_matrix.append(od.values())


    trans_matrix = np.matrix(trans_matrix)

    markov = markovChain(trans_matrix)
    markov.computePi('linear')  # We can also use 'power', 'krylov' or 'eigen'
    stationary = markov.pi

    p_rej = 0.
    for i in range(state_to_index(min_positive_state), len(stationary)):
        p_rej += index_to_state(i) * stationary[i]

    return p_rej, 1 / (1 - p_rej)


for a in (.8, .6, .4, .2, .1):
    p_rej, adv = mc(a)
    print a, p_rej, adv