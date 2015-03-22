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

#ifndef CMDUTILS_H
#define CMDUTILS_H

#include <math.h>
#include<boost/program_options.hpp>
#include <fstream>
#include <debug_stream.h>

bool fileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}


std::vector<double> getRange(std::string str){
    int cutAt;
    int c = 0;
    char delim = ':';
    std::vector<double> results;
    while( (cutAt = str.find_first_of(delim)) != str.npos ){
        if(cutAt > 0){
            c++;
            results.push_back(boost::lexical_cast<double>(str.substr(0,cutAt)));
        }
        str = str.substr(cutAt+1);
    }
    if(str.length() > 0){
        results.push_back(boost::lexical_cast<double>(str));
    }
    std::vector<double> ret;
    if(results.size() == 1){
        dstream << results[0] << "\n";

        return results;
    }
    if(results.size() != 3 || results[0] >= results[2] || results[1] <= 0){
        std::cerr << "wrong range parameters " << std::endl;
        exit(1);
    }
    double start = log(results[0]);
    double end = log(results[2]);
    dstream << "from " << results[0] << " to " << results[2]  << " with step of " << results[1] << "\n";

    double i = start;
    for(int j = 0; j < results[1]; j++){

         dstream << exp(i) << " ";

        ret.push_back(exp(i));
        i+= (end - start) /(results[1] - 1);
    }
    dstream << "\n";
    return ret;
}

std::vector<double> getLinearRange(std::string str){
    int cutAt;
    int c = 0;
    char delim = ':';
    std::vector<double> results;
    while( (cutAt = str.find_first_of(delim)) != str.npos ){
        if(cutAt > 0){
            c++;
            results.push_back(boost::lexical_cast<double>(str.substr(0,cutAt)));
        }
        str = str.substr(cutAt+1);
    }
    if(str.length() > 0){
        results.push_back(boost::lexical_cast<double>(str));
    }
    std::vector<double> ret;
    if(results.size() == 1){
        dstream << results[0] << "\n";
        return results;
    }
    if(results.size() != 3 || results[0] >= results[2] || results[1] <= 0){
        std::cerr << "wrong range parameters " << std::endl;
        exit(1);
    }
    double start = results[0];
    double end = results[2];
    dstream << "from " << results[0] << " to " << results[2]  << " with step of " << results[1] << "\n";

    double i = start;
    double step = fabs(results[1] - results[0]) / results[2];
    for(int j = results[0]; j <= results[2]; j += step){
         dstream << j    << " ";
        ret.push_back(j);
        j+= (end - start) /(results[1] - 1);
    }
    return ret;
}



void check(bool v, std::string what, boost::program_options::options_description& desc){
    if(!v){
        std::cerr << "Error: " << what << std::endl << "Usage: " << std::endl << desc << std::endl;
        exit(1);
    }
}


#endif // CMDUTILS_H
