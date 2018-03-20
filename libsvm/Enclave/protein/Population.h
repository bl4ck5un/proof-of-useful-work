//
// Created by alican on 10.05.2016.
//
#ifndef GENETICALGORITHM_POPULATION_H
#define GENETICALGORITHM_POPULATION_H

#include "Chromosome.h"
#include <vector>

class Population {
public:

    Population() { }

    Population(unsigned long size) {

        for (int i = 0; i < size; i++){
            Chromosome chrom;
            chrom.createRandomTurnList();
            chrom.process();
            chrom.setId();
            chromosomes.push_back((chrom));
        }
    }

    Population(std::vector<Chromosome> selection){
        chromosomes = selection;
    }

    Population selection(int);
    void mutation(double);
    void crossover_selection(double crossover_rate);
    void process();
    void printChromos();
    void printBestCandidate();

    void calcDiversity();

    double minFitness;
    double maxFitness;
    double averageFitness;

    float diversity;


    Population tournament_selection(int generation);

private:
    std::vector<Chromosome> chromosomes;

};

#endif //GENETICALGORITHM_POPULATION_H
