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
#ifndef DATASETMANAGEMENT_H
#define DATASETMANAGEMENT_H

#include <vector>
#include <fstream>
#include <eigen3/Eigen/Dense>
#include "extern/Liblinear/linear.h"
#include "extern/Libsvm/svm.h"
#include <iostream>

void readProblemASCIILibSVM(struct svm_problem& prob, std::string& trainingSetFile);

void readProblemASCIILibLinear(struct problem& prob, std::string& trainingSetFile, double bias);

template <class T>
void readProblemBinaryLibLinear(struct problem& prob, std::string& trainingSetFile, double bias);

template <class T>
void readProblemBinaryLibSVM(struct svm_problem& prob, std::string& trainingSetFile);

template <class T>
void readProblemBinaryLibLinear(struct problem& prob, std::string& trainingSetFile, double bias){
    std::ifstream in(trainingSetFile.c_str(), std::ios_base::binary);
    in.read((char*)&(prob.l), sizeof(prob.l));
    in.read((char*)&(prob.n), sizeof(prob.n));
    prob.bias = bias;
    prob.y = new double[prob.l];
    prob.x = new struct feature_node*[prob.l];
    for(unsigned i = 0; i < prob.l; i++){
        if(bias > 0){
            prob.x[i] = new struct feature_node[prob.n + 2 ];
        }else{
            prob.x[i] = new struct feature_node[prob.n + 1 ];
        }

        for(unsigned j = 0; j < prob.n; j++){
            prob.x[i][j].index = j+1;
            prob.x[i][j].value = 0;
        }

        if(bias > 0){
            prob.x[i][prob.n].index = prob.n+1;
            prob.x[i][prob.n].value = 1;
            prob.x[i][prob.n+1].index = -1;
            prob.x[i][prob.n+1].value = -1;
        }else{
            prob.x[i][prob.n].index = -1;
            prob.x[i][prob.n].value = -1;
        }
    }
    T v[prob.n];
    T l;
    for(unsigned i = 0; i < prob.l; i++) {
        in.read((char*)&l, sizeof(T));
        in.read((char*)&v, sizeof(T) * prob.n);
        prob.y[i] = l;
        for(int j = 0; j < prob.n; j++){
            prob.x[i][j].value = v[j];
            prob.x[i][j].index = j + 1;
        }
    }
    if(bias > 0){
        prob.n++;
    }
    in.close();
}



template <class T>
void readProblemBinaryLibSVM(struct svm_problem& prob, std::string& trainingSetFile){
    std::ifstream in(trainingSetFile.c_str(), std::ios_base::binary);
    int size;
    in.read((char*)&(prob.l), sizeof(prob.l));
    in.read((char*)&(size), sizeof(size));

    prob.y = new double[prob.l];
    prob.x = new struct svm_node*[prob.l];
    for(unsigned i = 0; i < prob.l; i++){
        prob.x[i] = new struct svm_node[size + 1 ];

        for(unsigned j = 0; j < size; j++){
            prob.x[i][j].index = j+1;
            prob.x[i][j].value = 0;
        }
        prob.x[i][size].index = -1;
        prob.x[i][size].value = -1;
    }
    T v[size];
    T l;
    for(unsigned i = 0; i < prob.l; i++) {
        in.read((char*)&l, sizeof(T));
        in.read((char*)&v, sizeof(T) * size);
        prob.y[i] = l;
        for(int j = 0; j < size; j++){
            prob.x[i][j].value = v[j];
            prob.x[i][j].index = j + 1;
        }
    }
    in.close();
}


void readProblemASCIILibSVM(svm_problem &prob, std::string &trainingSetFile){
    prob.l =  0;
    int size = -1;
    {
        std::ifstream in(trainingSetFile.c_str());
        while(!in.eof()){
            std::string line;
            getline(in, line);
            std::stringstream parse(line);
            int index;
            double val;
            int cont = 0;
            parse >> index;
            char tmp;
            while(parse >> index >> tmp >> val){
                if(index > size){
                    size = index;
                }
                cont++;
            }
            if(cont > 0){
                prob.l++;
            }
        }
        in.close();
    }
    prob.y = new double[prob.l];
    prob.x = new struct svm_node*[prob.l];
    for(unsigned i = 0; i < prob.l; i++){

        prob.x[i] = new struct svm_node[size + 1 ];
        for(unsigned j = 0; j < size; j++){
            prob.x[i][j].index = j+1;
            prob.x[i][j].value = 0 ;
        }

        prob.x[i][size].index = -1;
        prob.x[i][size].value = -1;

    }

    std::ifstream in(trainingSetFile.c_str());
    int nline = 0;
    while(!in.eof()){
        std::string line;
        getline(in, line);
        std::stringstream parse(line);
        int index;
        double val;
        parse >> prob.y[nline];
        char tmp;
        while(parse >> index >> tmp >> val){
            prob.x[nline][index - 1].value = val;
            prob.x[nline][index - 1].index = index;
        }
        nline++;
    }

    in.close();
}


void readProblemASCIILibLinear(problem &prob, std::string &trainingSetFile, double bias){
    prob.bias = bias;
    prob.l =  0;
    prob.n = -1;

    {
        std::ifstream in(trainingSetFile.c_str());
        while(!in.eof()){
            std::string line;
            getline(in, line);
            std::stringstream parse(line);
            int index;
            double val;
            int cont = 0;
            parse >> index;
            char tmp;
            while(parse >> index >> tmp >> val){
                if(index > prob.n){
                    prob.n = index;
                }
                cont++;
            }
            if(cont > 0){
                prob.l++;
            }
        }
        in.close();
    }

    prob.y = new double[prob.l];
    prob.x = new struct feature_node*[prob.l];
    for(unsigned i = 0; i < prob.l; i++){
        if(bias > 0){
            prob.x[i] = new struct feature_node[prob.n + 2 ];
        }else{
            prob.x[i] = new struct feature_node[prob.n + 1 ];
        }
        for(unsigned j = 0; j < prob.n; j++){
            prob.x[i][j].index = j+1;
            prob.x[i][j].value = 0 ;
        }
        if(bias > 0){
            prob.x[i][prob.n].index = prob.n+1;
            prob.x[i][prob.n].value = 1;
            prob.x[i][prob.n+1].index = -1;
            prob.x[i][prob.n+1].value = -1;
        }else{
            prob.x[i][prob.n].index = -1;
            prob.x[i][prob.n].value = -1;
        }
    }

    std::ifstream in(trainingSetFile.c_str());
    int nline = 0;
    while(!in.eof()){
        std::string line;
        getline(in, line);
        std::stringstream parse(line);
        int index;
        double val;
        parse >> prob.y[nline];
        char tmp;
        while(parse >> index >> tmp >> val){
            prob.x[nline][index - 1].value = val;
            prob.x[nline][index - 1].index = index;
        }
        nline++;
    }

    if(bias > 0){
        prob.n++;
    }
    in.close();
}


#endif // DATASETMANAGEMENT_H
