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
#ifndef ERRORS_EVALUATION_H
#define ERRORS_EVALUATION_H


template <class T>
class StandardClassificationAccuracy{
public:
    typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> MatrixD;

    std::pair<double, double> accuracy(double* v1, double *gt, int N){
        int corr =0;
        for(int i = 0; i < N; i++){
            if(gt[i]*v1[i] >= 0){
                corr++;
            }
        }
        return std::pair<double, double>(corr / (double) N, 0);
    }

};

template <class T>
class FixedTPAccuracy{
    double requiredTP;
public:

    FixedTPAccuracy(): requiredTP(0){

    }

    void setRequiredTP(double v){
        requiredTP = v;
    }

    std::pair<double, double> accuracy(double* v1, double *gt, int N){
        double bestTH = 0;
        double bestFP  = 1;
        int p =0, n = 0;
        for(int i = 0; i < N; i++){
            if(gt[i] >= 0){
                p++;
            }else{
                n++;
            }
        }
        for(int i = 0; i < N; i++){
            double tp = 0;
            double fp =  0;
            const double th = v1[i];
            for(int j = 0; j < N; j++){
                if(v1[j] >= th && gt[j] >= 0){
                    tp++;
                }
                if(v1[j] >=  th && gt[j] < 0){
                    fp++;
                }
            }
            fp /= n;
            tp /= p;
            if(tp >= requiredTP && fp < bestFP){
                bestFP = fp;
                bestTH = th;
            }
        }
        return std::pair<double, double>(1 - bestFP, bestTH);
    }
};


template <class T>
class EqualErrorRateAccuracy{
public:
    typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> MatrixD;

    std::pair<double, double> accuracy(double* v1, double *gt, int N){
        double closestErr = 0;
        double errDist = +INFINITY;
        int p =0, n = 0;
        for(int i = 0; i < N; i++){
            if(gt[i] >= 0){
                p++;
            }else{
                n++;
            }
        }
        for(int i = 0; i < N; i++){
            double tp = 0;
            double fp =  0;
            const double th = v1[i];
            for(int j = 0; j < N; j++){
                if(v1[j] >= th && gt[j] >= 0){
                    tp++;
                }
                if(v1[j] >= th && gt[j] < 0){
                    fp++;
                }
            }
            fp /= n;
            tp /= p;
            double tn = 1 - fp;
            if(fabs(tp - tn) < errDist){
                errDist = fabs(tp-tn);
                closestErr = (tn+tp) / 2;
            }
        }
        return std::pair<double, double>(closestErr, 0);
    }
};

#endif // ERRORS_EVALUATION_H
