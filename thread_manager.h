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
#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <stdlib.h>
#include <unistd.h>

class ThreadManager{
public:

    static ThreadManager* getInstance(){
        if(instance == 0){
            instance = new ThreadManager();
        }
        return instance;
    }

    int getThreadNumber() const {
        return threadNumber;
    }

    void setThreadNumber(int n = -1){
        if(n < 0){
            threadNumber = sysconf( _SC_NPROCESSORS_ONLN );
        }else{
            threadNumber = n;
        }
    }
private:
    ThreadManager(){
        setThreadNumber();
    }
    static ThreadManager* instance;
    int threadNumber;
};

ThreadManager* ThreadManager::instance = 0;

#endif // THREADMANAGER_H
