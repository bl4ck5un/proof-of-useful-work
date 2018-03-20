//
// Created by alican on 10.05.2016.
//

#include "Chromosome.h"
#include "GeneticAlgorithm.h"

#include "Log.h"


//std::random_device randomDevice2;
//std::default_random_engine randomEngine(randomDevice2());
//std::uniform_int_distribution<int> uniform_dist(0, 2);


Chromosome::Chromosome() {
    id = idGlobal++;
}


void Chromosome::calcFitness() {

   // std::cout << "pairs" << pairs.size() << std::endl;
   // std::cout << "collisions" << collisions << std::endl;

    fitness = 1;
    fitness += pairs.size() * 100;
    if ((collisions) > 0){
        if (collisions == 1){
            collisions++;
        }
        fitness /= (collisions*collisions);
    }

   // std::cout << "fitness" << fitness << std::endl;


}


void Chromosome::createRandomTurnList() {
        for (int i = 0; i  < GeneticAlgorithm::params.sequence.length() - 1; i++ ){
//            int turn = uniform_dist(randomEngine) - 1;
            int turn = (rand() % 3) - 1;
            turnList.push_back((TURNCODE)turn);
        }
}

void Chromosome::createCoordinatePath(Coordinate start) {
    pathList.clear();
    Coordinate current = start;
    pathList.push_back(current);
    for (unsigned i = 0; i < turnList.size(); i++) {
        TURNCODE turn = turnList.at(i);
        Coordinate next = current;
        next.turnTo(turn);
        pathList.push_back(next);
        current = next;
    }
}



void Chromosome::printCoordinates() {
//    for (Coordinate coordinate : pathList){
    for (unsigned i = 0; i < pathList.size(); i++) {
        Coordinate& coordinate = pathList.at(i);
        printf_sgx(coordinate.representation().c_str());
    }
    printf_sgx("\n");
}

void Chromosome::printTurns() {

//    for (TURNCODE turn: turnList){
    for (unsigned i = 0; i < turnList.size(); i++) {
        TURNCODE turn = turnList.at(i);
        printf_sgx("%d ", turn);
    }
    printf_sgx("\n");
}

bool Chromosome::isPair(Coordinate &first, Coordinate &second) {
    return abs((second.x - first.x)) + abs((second.y - first.y)) == 1;
}

void Chromosome::walkPath() {

    // reset collisions counter;
    this->collisions = 0;
    pairs.clear();


    Coordinate cord1;
    Coordinate cord2;

    for (size_t i = 0; i < pathList.size(); i++){
        cord1 =  pathList.at(i);
        for(size_t j = i + 2; j < pathList.size(); j++){
            cord2 = pathList.at(j);
            if(cord1.x == cord2.x && cord1.y == cord2.y) {
                collisions++;
            }
            if(GeneticAlgorithm::params.sequence.at(i) == '1' && GeneticAlgorithm::params.sequence.at(j)){
                if (isPair(pathList.at(i), pathList.at(j))){
                    pairs.push_back(std::make_pair<Coordinate, Coordinate>(
                            (Coordinate) pathList.at(i),
                            (Coordinate) pathList.at(j))
                    );
                }
            }
        }
    }
}

void Chromosome::process() {

    if (changed){
        Coordinate start;
        createCoordinatePath(start);
        walkPath();
        calcFitness();
        changed = false;
    }

}

void Chromosome::crossover(Chromosome& other) {

    int position = rand_int_uniform(1, turnList.size()-1);

    std::vector<TURNCODE>& list1 = this->turnList;
    std::vector<TURNCODE>& list2 = other.turnList;

    std::vector<TURNCODE>::iterator it1 = list1.begin();
    std::advance(it1, position);
    std::vector<TURNCODE>::iterator it2 = list2.begin();
    std::advance(it2, position);

    std::vector<TURNCODE> nlist1;
    std::vector<TURNCODE> nlist2;
    nlist1.insert(nlist1.begin(), list1.begin(), it1);
    nlist1.insert(nlist1.end(), it2, list2.end());
    nlist2.insert(nlist2.begin(), list2.begin(), it2);
    nlist2.insert(nlist2.end(), it1, list1.end());

    list1 = (nlist1);
    list2 = (nlist2);


    // Mark as changed for recalculation of fitness, collusion, path etc.
    this->changed = true;
    this->setId();
    other.changed = true;
    other.setId();
}

int Chromosome::idGlobal = 0;

void Chromosome::mutate() {
//    std::uniform_int_distribution<int> randomAccess(0, (int) turnList.size()-1);
//    std::uniform_int_distribution<int> randomTurnCode(0, 2);
//    int position = randomAccess(randomDevice2) ;
//    int mutation = randomTurnCode(randomDevice2) -1 ;
//    turnList.at(position) = (TURNCODE) mutation;
    int position = (int) rand_int_uniform(0, (int) turnList.size()-1);
    int mutation = (int) rand_int_uniform(0, 2);
    this->turnList.at(position) = (TURNCODE) mutation;

    // Mark as changed for recalculation of fitness, collusion, path etc.
    this->changed = true;
    this->setId();
}

void Chromosome::printInfo() {

    printf_sgx("### Best Candidate ###\n");
    printf_sgx("Pairs: %d\n", pairs.size());
    printf_sgx("Collisions: %d\n", collisions);
    printf_sgx("Fitness: %f\n", fitness);
    printf_sgx("ID: %d\n",id);

    printTurns();
    printCoordinates();

}

void Chromosome::setId() {
    this->id = ++idGlobal;
}




