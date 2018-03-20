//
// Created by alican on 10.05.2016.
//

//#include <functional>
#include "Population.h"

Population Population::selection(int generation) {

    // std::default_random_engine generator;

    float ex = (float) (0.5 + (generation / 300.0));
    // std::exponential_distribution<double> distribution(ex);

    std::vector<Chromosome> new_chromosomes;

    for(int i = 0; i < chromosomes.size();){
        // double number = distribution(generator);
        double number = rand_exp(ex);

        if (number<1.0){
            i++;
            int position = int(chromosomes.size()*number);
            new_chromosomes.push_back(chromosomes.at(position));
        }
    }

    return Population(new_chromosomes);
}


Population Population::tournament_selection(int generation) {
    std::vector<Chromosome> new_chromosomes;

    //Fisher-Yates shuffle
    int t_rounds = (int) chromosomes.size();
    //std::cout << crossover_count << std::endl;

    long left = std::distance(chromosomes.begin(), chromosomes.end());
    Chromosome* current = chromosomes.begin();

    while (t_rounds) {
        Chromosome* rI1 = current;
        Chromosome* rI2 = current;
        std::advance(rI1, rand() % left);
        std::advance(rI2, rand() % left);
        //std::swap(*current, *r);
        Chromosome &ch1 = (*(rI1));
        Chromosome &ch2 = (*(rI2));

        Chromosome winner = (ch1.getFitness() > ch2.getFitness()) ? ch1 : ch2;
        new_chromosomes.push_back((winner));

        --t_rounds;
    }

/*
    static const float t = 0.6f;
    std::uniform_int_distribution<int> uniform_dist(0, chromosomes.size());

    for (size_t i = 0; i < chromosomes.size(); ++i) {
        Chromosome* c1 = nullptr;
        while (!c1){
            int index = uniform_dist(randomEngine) - 1;

            Chromosome& c = chromosomes[index];
            if (&c != c1) {
                c1 = &c;
            }
        }

        Chromosome* c2 = nullptr;
        while (!c2) {
            int index = uniform_dist(randomEngine) - 1;
            Chromosome& c = chromosomes[index];
            if (&c != c2) {
                c2 = &c;
            }
        }



        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (r >= t) {
            winner = *c2;
        }

        new_chromosomes.push_back(std::move(winner));

    }
*/
    return Population(new_chromosomes);

 }



void Population::crossover_selection(double crossover_rate) {

    // 0.25 der chromosome.
    //Fisher-Yates shuffle
    int crossover_count = (int) (chromosomes.size() * crossover_rate );
    //std::cout << crossover_count << std::endl;

    long left = std::distance(chromosomes.begin(), chromosomes.end());
    Chromosome* current = chromosomes.begin();

    while (crossover_count) {
        Chromosome* r = current;
        std::advance(r, rand()%left);
        //std::swap(*current, *r);
        Chromosome& ch1 = (*(current));
        Chromosome& ch2 = (*(r));

        if(ch1.id != ch2.id){
            ch1.crossover(ch2);
            --crossover_count;
        }
        ++current;
        --left;
    }

  /*  for(auto it = chromosomes.begin(); it < current; it +=2) {
        Chromosome& ch1 = (*(it));
        Chromosome& ch2 = *(it +1);
        ch1.crossover(ch2);
    }*/

}

void Population::mutation(double mutation_rate) {
    /*
     *   Wahl der Mutationen innerhalb der Population
     *
     */
    double mutation_count = std::ceil((chromosomes.size() * mutation_rate ));
    //double mutation_count = chromosomes.size();

    long size = chromosomes.size();
    Chromosome* current = chromosomes.begin();

    while (mutation_count--) {
        Chromosome* r = current;
        std::advance(r, rand()%size);
        (*(r)).mutate();
    }


}

void Population::process() {

    double totalFitness = 0;
    // re-calculate changed chromosomes
    // for (Chromosome& chromo : chromosomes){
    for (unsigned i = 0; i < chromosomes.size(); i++) {
        Chromosome& chromo = chromosomes.at(i);
        chromo.process();
        totalFitness += chromo.getFitness();
    }
    // sort list by fitness
    std::sort(chromosomes.rbegin(), chromosomes.rend());
    // calculate min/max fitness of population

    minFitness = chromosomes.back().getFitness();
    maxFitness = chromosomes.front().getFitness();
    averageFitness = totalFitness/chromosomes.size();
}

void Population::printBestCandidate() {
    std::sort(chromosomes.rbegin(), chromosomes.rend());
    chromosomes.front().printInfo();
}

void Population::calcDiversity() {
    size_t orientationCount = chromosomes.front().turnList.size();

    int populationSize = (int) chromosomes.size();

    uint64_t sum = 0;

    for (size_t i = 0; i < populationSize; ++i)
    {
        for (size_t j = i + 1; j < populationSize; ++j)
        {
            const Chromosome & c1 = chromosomes[i];
            const Chromosome & c2 = chromosomes[j];

            for (size_t pos = 0; pos < orientationCount; ++pos)
            {
                if (c1.turnList[pos] != c2.turnList[pos])
                {
                    ++sum;
                }
            }
        }
    };

    uint64_t numComparisons = (uint64_t) ((populationSize * populationSize / 2) - (populationSize / 2));

    uint64_t total = numComparisons * orientationCount;

    diversity = static_cast<float>(static_cast<double>(sum) / total);
    diversity *= 100.0f;

    diversity = roundf(diversity);
}

