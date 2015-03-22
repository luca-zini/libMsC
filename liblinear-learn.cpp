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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/mutex.hpp>
#include "roc.h"
#include "extern/Liblinear/linear.h"
#include "cmd-utils.h"
#include "parfor.h"
#include "dataset_management.h"
#include "errors_evaluation.h"

struct parameter buildPar(int algo, double eps, double c){
    struct parameter param;
    param.solver_type = algo;
    param.C = c;
    param.eps = eps;
    param.p = 0.1;
    param.nr_weight = 0;
    param.weight_label = 0;
    param.weight = 0;
    return param;
}

void free_problem(problem& prob){
    delete[] prob.y;
    for(unsigned i = 0; i < prob.l; i++){
        delete[] prob.x[i];
    }
    delete[] prob.x;
}

namespace po = boost::program_options;
using namespace std;



template <class ERR_EVAL>
class ParLibLinear : public ERR_EVAL{
public:
    void operator()(int cIndex){
        struct parameter par = buildPar(algorithm, epsilon, cs.at(cIndex));
        double* ris = 0;
        if(validation){
            ris = new double[val.l];
        }else if(kcv > 1){
            ris = new double[prob.l];
        }
        std::pair<double, double> prec(0,0);
        if(validation){
            model* mod = train(&prob, &par);
            if(saveAll){
                std::stringstream modelPath;
                modelPath << baseModelPath <<  "_"<< cs.at(cIndex);
                modelPath << ".model";
                save_model(modelPath.str().c_str(),mod);
            }
            for(unsigned j = 0; j < val.l; j++){
                predict_values(mod, val.x[j], &(ris[j]));
            }
            prec = ERR_EVAL::accuracy(ris, val.y, val.l);
            {
                boost::mutex::scoped_lock l(m_mutex);
                if(rocBasePath != "" && saveAll){
                    std::stringstream rocName;
                    rocName << rocBasePath <<  "_"<< cs.at(cIndex);
                    rocName << ".roc";
                    std::ofstream outroc(rocName.str().c_str());
                    saveRoc(outroc, val.y, ris, val.l);
                    outroc.close();
                }else if(rocBasePath != "" && prec.first > bestPrecision.first){
                    std::ofstream outroc(rocBasePath.c_str());
                    saveRoc(outroc, val.y, ris, val.l);
                    outroc.close();
                }
                if(prec > bestPrecision){
                    if(bestModel != 0){
                        free_and_destroy_model(&bestModel);
                    }
                    bestPrecision = prec;
                    bestIndex = cIndex;
                    bestModel = mod;
                }else{
                    free_and_destroy_model(&mod);
                }
                dstream << "Liblinear classifier C = " << par.C<< "\n"  << "Accuracy: " << prec.first << "\n";
            }

        }else if(kcv > 1){
            cross_validation_fuzzy(&prob, &par, kcv, ris);
            prec = ERR_EVAL::accuracy(ris, prob.y,  prob.l);
            {
                boost::mutex::scoped_lock l(m_mutex);
                if(rocBasePath != "" && saveAll){
                    std::stringstream rocName;
                    rocName << rocBasePath <<  "_"<< cs.at(cIndex);
                    rocName << ".roc";
                    std::ofstream outroc(rocName.str().c_str());
                    saveRoc(outroc, prob.y, ris, prob.l);
                    outroc.close();
                }else  if(rocBasePath != ""&& prec > bestPrecision){
                    std::ofstream outroc(rocBasePath.c_str());
                    saveRoc(outroc, prob.y, ris, prob.l);
                    outroc.close();
                }
                if(prec > bestPrecision){
                    bestPrecision = prec;
                    bestIndex = cIndex;
                }
                dstream << "Liblinear classifier C = " << par.C<< "\n" << "Accuracy: " << prec.first << "\n \n";
            }
        }else if(saveAll || cs.size() == 1){
            model* mod = train(&prob, &par);
            {
                boost::mutex::scoped_lock l(m_mutex);
                std::stringstream modelPath;
                if(saveAll){
                    modelPath << baseModelPath <<  "_"<< cs.at(cIndex);
                    modelPath << ".model";
                }else{
                    modelPath << baseModelPath;
                }
                save_model(modelPath.str().c_str(),mod);
                dstream << "Liblinear classifier C = " << par.C  << "\n";
                if(cs.size() > 1){
                    free_and_destroy_model(&mod);
                }else{
                    bestModel = mod;
                }
            }
        }else{
            throw std::runtime_error("Undefined");
        }
        delete[] ris;
    }

    double getTH() const {
        return bestPrecision.second;
    }

    double getPrecision() const {
        return bestPrecision.first;
    }

    unsigned getBestIndex() const {
        return bestIndex;
    }

    model* getBestModel() const {
        return bestModel;
    }

    ParLibLinear( std::vector<double> cs, struct problem prob, struct problem val, bool validation, unsigned kcv, int algorithm, double epsilon, std::string rocBasePath, std::string modelBasePath, bool saveAll):cs(cs), prob(prob), val(val),validation(validation), kcv(kcv),algorithm(algorithm), epsilon(epsilon), rocBasePath(rocBasePath), saveAll(saveAll), baseModelPath(modelBasePath){
        bestPrecision = std::pair<double, double>(-1, -1);
        bestIndex = -1;
        bestModel = 0;
    }

private:
    model* bestModel;
    std::string rocBasePath;
    std::pair<double, double> bestPrecision;
    int bestIndex;
    std::vector<double> cs;
    struct problem prob;
    struct problem val;
    bool validation;
    int kcv;
    double epsilon;
    int algorithm;
    boost::mutex m_mutex;
    bool saveAll;
    std::string baseModelPath;
};



int main(int argc, char** argv){
    double bias = 1;
    double requiredTP = 0.995;
    int ERR_TYPE = 1;
    bool singlePrecision = false;
    bool binary = false;
    int kcv = 1, algorithm = 2;
    double* ris = 0;
    std::string rocBasePath = "", debugFile, trainFile, testFile, cRange, modelFile;
    struct problem val;
    double epsilon = 0.01;
    bool saveAll = false;
    std::vector<double> cs;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("save-all", "save all ROCs. If omitted saves only the roc selected with kcv or validation")
            ("binary", "use binary dataset instead of libsvm format")
            ("roc", po::value<std::string>(&rocBasePath), "roc base path and name")
            ("log", po::value<std::string>(&debugFile),"log file")
            ("train-set", po::value<std::string>(&trainFile),"train data file")
            ("validation-set", po::value<std::string>(&testFile), "validation data file used for model selection")
            ("c", po::value<std::string>(&cRange), "C parameter of the SVM. It can be a scalar or a range in the form first:N:end. In this second case a geometric serie of N elements will be used to sample in the range first-end")
            ("epsilon", po::value<double>(&epsilon), " set tolerance of termination criterion\n -s 0 and 2\n |f'(w)|_2 <= eps*min(pos,neg)/l*|f'(w0)|_2,\n where f is the primal function and pos/neg are # of\n positive/negative data (default 0.01)\n -s 1, 3, 4 and 7\n Dual maximal violation <= eps; similar to libsvm (default 0.1)\n -s 5 and 6\n |f'(w)|_1 <= eps*min(pos,neg)/l*|f'(w0)|_1,\n	where f is the primal function (default 0.01)\n")
            ("algorithm", po::value<int>(&algorithm), "set type of solver (default 1)\n	 0 -- L2-regularized logistic regression (primal)\n 1 -- L2-regularized L2-loss support vector classification (dual)\n 2 -- L2-regularized L2-loss support vector classification (primal)\n 3 -- L2-regularized L1-loss support vector classification (dual)\n 4 -- multi-class support vector classification by Crammer and Singer\n 5 -- L1-regularized L2-loss support vector classification\n 6 -- L1-regularized logistic regression\n 7 -- L2-regularized logistic regression (dual)\n")
            ("model", po::value<std::string>(&modelFile), "output model file")
            ("folds", po::value<int>(&kcv), "kcv folds")
            ("err-type", po::value<int>(&ERR_TYPE), " Function used to evaluate the error: \n 1 -- error counting \n 2 -- equal error rate \n 3 -- fixed true positive rate ")
            ("required-tp", po::value<double>(&requiredTP), " required TP if is selected fixedTPerror ")
            ("single-precision", "the algorithms use single precision floating point instead of double")
            ("mt", "it use multithread algorithm")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    check(fileExists(trainFile), "Cannot open training set file", desc);
    if(vm.count("validation-set")){
        check(fileExists(testFile), "Cannot open validation set file", desc);
    }

    if (vm.count("save-all")) {
        saveAll = true;
    }

    check(vm.count("c"), "no C specified", desc);
    if(vm.count("roc")){
        check(vm.count("validation-set") || vm.count("folds"), "KCV or validation set is needed to compute a ROC", desc);
    }

    singlePrecision = vm.count("single-precision");
    binary = vm.count("binary");

    {
        ThreadManager* tmp = ThreadManager::getInstance();
        if (vm.count("mt")) {
            tmp->setThreadNumber();
        }else{
            tmp->setThreadNumber(1);
        }
    }

    if(vm.count("log")){
        dstream.setStream(new std::ofstream(debugFile.c_str()));
        for(unsigned i = 0; i < argc; i++){
            dstream << argv[i] << " ";
        }
        dstream << "\n";
    }


    dstream << "C range: ";
    cs = getRange(cRange);
    if(cs.size() > 1){
        check(vm.count("save-all") || vm.count("validation-set") || vm.count("folds"), "Multiple parameter require --save-all or --fold or --validation-set", desc);
    }

    struct problem prob;
    if(binary){
        if(singlePrecision){
            readProblemBinaryLibLinear<float>(prob, trainFile, bias);
        }else{
            readProblemBinaryLibLinear<double>(prob, trainFile, bias);
        }
    }else{
        readProblemASCIILibLinear(prob, trainFile, bias);
    }

    if(vm.count("validation-set")){
        if(binary){
            if(singlePrecision){
                readProblemBinaryLibLinear<float>(val, testFile, bias);
            }else{
                readProblemBinaryLibLinear<double>(val, testFile, bias);
            }
        }else{
            readProblemASCIILibLinear(val, testFile, bias);
        }
    }
    if(kcv > 1){
        ris = new double[prob.l];
    }
    double bestPrecision = -1;
    int bestIndex = -1;
    model* bestModel = 0;
    double offset = 0;

    if(ERR_TYPE == 1){
        ParLibLinear<StandardClassificationAccuracy<double> > MT(cs, prob, val,vm.count("validation-set"),  kcv, algorithm, epsilon, rocBasePath, modelFile, saveAll);
        par_for_each(0, cs.size(), MT);
        bestPrecision = MT.getPrecision();
        bestIndex = MT.getBestIndex();
        bestModel = MT.getBestModel();
    }else if(ERR_TYPE == 2){
        ParLibLinear<EqualErrorRateAccuracy<double> > MT(cs, prob, val,vm.count("validation-set"),  kcv, algorithm, epsilon, rocBasePath, modelFile, saveAll);
        par_for_each(0, cs.size(), MT);
        bestPrecision = MT.getPrecision();
        bestIndex = MT.getBestIndex();
        bestModel = MT.getBestModel();
    }else if(ERR_TYPE == 3){
        check(vm.count("required-tp") , "required true positive rate not set", desc);
        ParLibLinear<FixedTPAccuracy<double> > MT(cs, prob, val,vm.count("validation-set"),  kcv, algorithm, epsilon, rocBasePath, modelFile, saveAll);
        MT.setRequiredTP(requiredTP);
        par_for_each(0, cs.size(), MT);
        bestPrecision = MT.getPrecision();
        bestIndex = MT.getBestIndex();
        bestModel = MT.getBestModel();
        offset = MT.getTH();
    }else{
        throw std::runtime_error("Undefined");
    }

    if(bestIndex >= 0){
        dstream << "Precision: " << bestPrecision << "\n" << "C: " << cs.at(bestIndex) << "\n";
        if(vm.count("validation-set") || kcv > 1){
            cout << "Precision: " << bestPrecision << std::endl << "C: " << cs.at(bestIndex) << std::endl;
        }
    }
    if(bestIndex >= 0 && modelFile != ""){
        model* mod = bestModel;
        if(mod == 0){
            assert(kcv > 0);
            std::cout << "Recomputing model with the selected parameters..." << std::endl;
            struct parameter par = buildPar(algorithm, epsilon, cs.at(bestIndex));
            const char* err = check_parameter(&prob, &par);
            if(err != 0){
                check(false, err, desc);
            }
            mod = train(&prob, &par);
        }
        std::cout << "Saving final model..." << std::endl;
        save_model(modelFile.c_str(), mod);
        free_and_destroy_model(&mod);
    }
    free_problem(prob);
    if(vm.count("validation-set")){
        free_problem(val);
    }
    std::cout << "Done" << std::endl;
    return 0;
}
