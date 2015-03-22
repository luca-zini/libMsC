/**********************************************************
Copyright 2014 by Luca Zini and Francesca Odone,
Department of Informatics, Bioengineering, Robotics, and Systems
Engineering University of Genova.

This is a free software: you can redistribute it and/or modify it
under the terms of the CC-BY Public License, Version 4.0, 25
November 2013. This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY, including but not limited to
the warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the CC-BY Public License copy provided with this software for more
details.

@author Luca Zini (luca.zini@gmail.com)

***********************************************************/
#ifndef PARFOR_H
#define PARFOR_H

#include <boost/thread/thread.hpp>
#include "thread_manager.h"

template<class Function>
class PredExecutor{
public:
        PredExecutor(int first, int last, Function& f, int step) : first(first), last(last), f(f), step(step){
        }

        void operator()(){
                for ( ; first < last; first += step){
                        f(first);
                }
        }

private:
        int first, last;
        Function& f;
        int step;
};

template<class Function>
void par_for_each(int first, int last, Function& f){
        const ThreadManager* const manager = ThreadManager::getInstance();
        int workersNumber = manager->getThreadNumber();
        const int N = last - first;
        if(workersNumber > N){
                workersNumber = N;
        }
        std::vector<boost::thread*> workers(workersNumber);
        for(unsigned i = 0; i < workersNumber; i++){
                workers[i] = new boost::thread(PredExecutor<Function>(first + i, last, f, workersNumber));
        }
        for(unsigned i = 0; i < workersNumber; i++){
                workers[i]->join();
                delete workers[i];
        }
}

#endif // PARFOR_H
