//
// Created by alican on 09.05.2016.
//

#include "GeneticAlgorithm.h"

void GeneticAlgorithm::run() {


    Population p = createBasePopulation();
    p.process();
    generations.push_back(p);

    double fitness = 0.001;
    int generation = 0;

    while(generation < params.generations){
        generation++;
        p = p.tournament_selection(generation);
        p.crossover_selection(params.crossoverPercent);
        p.mutation(params.mutationPercent);
        p.process();

        p.calcDiversity();
        generations.push_back(p);
     }

    // generations.back().printBestCandidate();
}

Population GeneticAlgorithm::createBasePopulation() {
    Population base(params.populationSize);
    return base;
}

bool GeneticAlgorithm::keepGoing() {
    return false;
}

