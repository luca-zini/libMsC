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
#include "extern/Libsvm/svm.h"
#include "cmd-utils.h"
#include "parfor.h"
#include "dataset_management.h"
#include "errors_evaluation.h"


void free_svm_problem(svm_problem& prob){
    delete[] prob.y;
    for(unsigned i = 0; i < prob.l; i++){
        delete[] prob.x[i];
    }
    delete[] prob.x;
}


int cacheSize = 1000;

struct svm_parameter buildPar(int kernel, double kernelpar, double eps, double c){
    struct svm_parameter param;
    param.cache_size = cacheSize;
    param.eps = eps;
    param.C = c;
    if(kernel == RBF || kernel == SIGMOID){
        param.gamma = kernelpar;
    }else if(kernel == POLY){
        param.gamma = 1;
        param.degree = kernelpar;
    }
    param.kernel_type = kernel;
    param.svm_type = C_SVC;
    param.shrinking = 1;
    param.probability = 0;
    param.nr_weight = 0;
    param.weight_label = 0;
    param.weight = 0;

    //DEFAULT
    param.coef0 = 0;
    param.nu = 0.5;
    param.p = 0.1;
    return param;
}


namespace po = boost::program_options;
using namespace std;


template <class ERR_EVAL>
class LibSVMPool : public ERR_EVAL{
public:
    void operator()(int cIndex){
        assert(cs.at(cIndex).size() == 2);

        struct svm_parameter par = buildPar(kernelType, cs.at(cIndex)[1], epsilon, cs.at(cIndex)[0]);
        double* ris = 0;
        if(validation){
            ris = new double[val.l];
        }else if(kcv > 1){
            ris = new double[prob.l];
        }

        std::pair<double, double> prec(0,0);
        if(validation){
            svm_model* mod = svm_train(&prob, &par);
            if(saveAll){
                std::stringstream modelPath;
                modelPath << baseModelPath <<  "_";
                for(unsigned p = 0; p < cs[cIndex].size(); p++){
                    modelPath <<  "_" << cs.at(cIndex)[p];
                }
                modelPath << ".model";
                svm_save_model(modelPath.str().c_str(),mod);
            }
            for(unsigned j = 0; j < val.l; j++){
                svm_predict_values(mod, val.x[j], &(ris[j]));
            }
            prec = ERR_EVAL::accuracy(ris, val.y, val.l);
            {
                boost::mutex::scoped_lock l(m_mutex);
                if(rocBasePath != "" && saveAll){
                    std::stringstream rocName;
                    rocName << rocBasePath;
                    for(unsigned p = 0; p < cs[cIndex].size(); p++){
                        rocName <<  "_" << cs.at(cIndex)[p];
                    }
                    rocName << ".roc";
                    std::ofstream outroc(rocName.str().c_str());
                    saveRoc(outroc, val.y, ris, val.l);
                    outroc.close();
                }else if(rocBasePath != ""&& prec.first > bestPrecision.first){
                    std::ofstream outroc(rocBasePath.c_str());
                    saveRoc(outroc, val.y, ris, val.l);
                    outroc.close();
                }
                if(prec > bestPrecision){
                    if(bestModel != 0){
                        svm_free_and_destroy_model(&bestModel);
                    }
                    bestPrecision = prec;
                    bestIndex = cIndex;
                    bestModel = mod;
                }else{
                    svm_free_and_destroy_model(&mod);
                }

                dstream << "LibSVM classifier C = " << par.C;
                if(cs[cIndex].size() > 1){
                    dstream <<" kernel parameters " ;
                    for(unsigned p = 1; p < cs[cIndex].size(); p++){
                        dstream <<" "  << cs[cIndex][p];
                    }
                }
                dstream << " Accuracy: " << prec.first << "\n";
            }
        }else if(kcv > 1){
            svm_cross_validation_fuzzy(&prob, &par, kcv, ris);
            prec = ERR_EVAL::accuracy(ris, prob.y, prob.l);
            {
                boost::mutex::scoped_lock l(m_mutex);
                if(rocBasePath != "" && saveAll){
                    std::stringstream rocName;
                    rocName << rocBasePath;
                    for(unsigned p = 0; p < cs[cIndex].size(); p++){
                        rocName <<  "_" << cs.at(cIndex)[p];
                    }
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
                dstream << "LibSVM classifier C = " << par.C;
                if(cs[cIndex].size() > 1){
                    dstream <<" kernel parameters " ;
                    for(unsigned p = 1; p < cs[cIndex].size(); p++){
                        dstream <<" "  << cs[cIndex][p];
                    }
                }
                dstream<< " Accuracy: " << prec.first << "\n";
            }
        }else if(saveAll || cs.size() == 1){
            svm_model* mod = svm_train(&prob, &par);
            {
                boost::mutex::scoped_lock l(m_mutex);

                std::stringstream modelPath;
                if(saveAll){
                    modelPath << baseModelPath <<  "_";
                    for(unsigned p = 0; p < cs[cIndex].size(); p++){
                        modelPath <<  "_" << cs.at(cIndex)[p];
                    }
                    modelPath << ".model";
                }else{
                    modelPath << baseModelPath;
                }
                svm_save_model(modelPath.str().c_str(), mod);

                dstream << "LibSVM classifier C = " << par.C;
                if(cs[cIndex].size() > 1){
                    dstream <<" kernel parameters " ;
                    for(unsigned p = 1; p < cs[cIndex].size(); p++){
                        dstream <<" "  << cs[cIndex][p] << "\n";
                    }
                }
                if(cs.size() > 1){
                    svm_free_and_destroy_model(&mod);
                }else{
                    bestModel = mod;
                }
            }
        }else{
            throw std::runtime_error("Undefined");
        }

        delete[] ris;
        computed[cIndex] = true;
    }

    double getPrecision() const {
        return bestPrecision.first;
    }

    unsigned getBestIndex() const {
        return bestIndex;
    }

    svm_model* getBestModel() const {
        return bestModel;
    }

    LibSVMPool( std::vector<std::vector<double> > cs, struct svm_problem prob, struct svm_problem val, bool validation, unsigned kcv, double epsilon, std::string rocBasePath, bool saveAll, int kernelType, std::string baseModelPath):
        kernelType(kernelType), cs(cs), computed(cs.size(), false), prob(prob), val(val),validation(validation), kcv(kcv), epsilon(epsilon), rocBasePath(rocBasePath), saveAll(saveAll), baseModelPath(baseModelPath), bestModel(0), computedModels(cs.size(), 0){
        bestPrecision = std::pair<double, double>(0, 0);
        bestIndex = -1;
        bestModel = 0;
    }

private:
    std::string rocBasePath;
    std::pair<double, double> bestPrecision;
    int bestIndex;
    int kernelType;
    std::vector<std::vector<double> > cs;
    std::vector<bool> computed;
    struct svm_problem prob;
    struct svm_problem val;
    bool validation;
    int kcv;
    double epsilon;
    boost::mutex m_mutex;
    bool saveAll;
    std::string baseModelPath;
    svm_model* bestModel;
    std::vector<svm_model*> computedModels;
};



int main(int argc, char** argv){
    int kernelType = RBF;
    int ERR_TYPE =1;
    bool singlePrecision = false;
    bool binary = false;
    int kcv = 1;
    std::string rocBasePath = "", debugFile, trainFile, testFile, cRange, gammaRange, modelFile = "", degreeRange;
    struct svm_problem val;
    double epsilon = 0.01;
    double requiredTP = 0.995;
    bool saveAll = false;
    std::vector<double> regs, gammas, degrees;
    std::vector<std::vector<double> > cs;
    struct svm_problem prob;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("cachesize", po::value<int>(&cacheSize),"cache size in mb (-m libsvm option) default 1000")
            ("kernel", po::value<int>(&kernelType),"kernel type: \n 0 -- Linear \n 1 -- Polynomial \n 2 -- RBF \n 3 -- Sigmoid \n default RBF")
            ("save-all", "save all ROCs and models. If omitted saves only the roc selected with kcv or validation")
            ("binary", "use binary dataset instead of libsvm format")
            ("roc", po::value<std::string>(&rocBasePath), "roc base path and name")
            ("log", po::value<std::string>(&debugFile),"log file")
            ("train-set", po::value<std::string>(&trainFile),"train data file")
            ("validation-set", po::value<std::string>(&testFile), "validation data file used for model selection")
            ("c", po::value<std::string>(&cRange), "C parameter of the SVM. It can be a scalar or a range in the form first:N:end. In this second case a geometric serie of N elements will be used to sample in the range first-end")
            ("gamma", po::value<std::string>(&gammaRange), "rbf kernel parameter. It can be a scalar or a range in the form first:N:end. In this second case a geometric serie of N elements will be used to sample in the range first-end")
            ("degree", po::value<std::string>(&degreeRange), "degree of polynomial kernel parameter. It can be a scalar or a range in the form first:N:end. In this second case a linear span of N elements will be used to sample in the range first-end")
            ("epsilon", po::value<double>(&epsilon), " set tolerance of termination criterion\n -s 0 and 2\n |f'(w)|_2 <= eps*min(pos,neg)/l*|f'(w0)|_2,\n where f is the primal function and pos/neg are # of\n positive/negative data (default 0.01)\n -s 1, 3, 4 and 7\n Dual maximal violation <= eps; similar to libsvm (default 0.1)\n -s 5 and 6\n |f'(w)|_1 <= eps*min(pos,neg)/l*|f'(w0)|_1,\n	where f is the primal function (default 0.01)\n")
            ("err-type", po::value<int>(&ERR_TYPE), " Function used to evaluate the error: \n 1 -- error counting \n  2 -- equal error rate \n 3 -- fixed true positive rate ")
            ("required-tp", po::value<double>(&requiredTP), " required TP if is selected fixedTPerror ")
            ("model", po::value<std::string>(&modelFile), "output model file")
            ("folds", po::value<int>(&kcv), "kcv folds")
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

    saveAll = vm.count("save-all");

    check(vm.count("c"), "no C specified", desc);
    if(vm.count("roc")){
        check(vm.count("validation-set") || vm.count("folds"), "KCV or validation set is needed to compute a ROC", desc);
    }
    singlePrecision = vm.count("single-precision");

    binary = vm.count("binary");
    if (vm.count("mt")) {
        ThreadManager::getInstance()->setThreadNumber();
    }else{
        ThreadManager::getInstance()->setThreadNumber(1);
    }
    if(vm.count("log")){
        dstream.setStream(new std::ofstream(debugFile.c_str()));
        for(unsigned i = 0; i < argc; i++){
            dstream << argv[i] << " ";
        }
        dstream << "\n";
    }
    dstream << "Kernel " << kernelType << "\n" << "C range: ";
    regs = getRange(cRange);
    if(kernelType == RBF){
        check(vm.count("gamma"), "no gamma specified", desc);
    }else if(kernelType == POLY){
        check(vm.count("degree"), "no degree specified", desc);
    }
    if(vm.count("gamma")){
        check(kernelType == RBF || kernelType == SIGMOID, "gamma can be used only with rbf and sigmoid kernels", desc);
        dstream << "gamma range: ";
        gammas = getRange(gammaRange);
        for(unsigned i = 0; i < regs.size(); i++){
            for(unsigned j = 0; j < gammas.size(); j++){
                std::vector<double> parameter(2);
                parameter[0] = regs[i];
                parameter[1] = gammas[j];
                cs.push_back(parameter);
            }
        }
    }else if (vm.count("degree")){
        check(kernelType == POLY, "gamma can be used only with polynomial kernel", desc);
        degrees = getLinearRange(degreeRange);
        for(unsigned i = 0; i < regs.size(); i++){
            for(unsigned j = 0; j < degrees.size(); j++){
                std::vector<double> parameter(2);
                parameter[0] = regs[i];
                parameter[1] = degrees[j];
                cs.push_back(parameter);
            }
        }
    }else if(kernelType == LINEAR){
        for(unsigned i = 0; i < regs.size(); i++){
            std::vector<double> parameter(1, regs[i]);
            cs.push_back(parameter);
        }
    }
    if(cs.size() > 1){
        check(vm.count("save-all") || vm.count("validation-set") || vm.count("folds"), "Multiple parameter require --save-all or --fold or --validation-set", desc);
    }

    if(binary){
        if(singlePrecision){
            readProblemBinaryLibSVM<float>(prob, trainFile);
        }else{
            readProblemBinaryLibSVM<double>(prob, trainFile);
        }
    }else{
        readProblemASCIILibSVM(prob, trainFile);
    }

    if(vm.count("validation-set")){
        if(binary){
            if(singlePrecision){
                readProblemBinaryLibSVM<float>(val, testFile);
            }else{
                readProblemBinaryLibSVM<double>(val, testFile);
            }
        }else{
            readProblemASCIILibSVM(val, testFile);
        }
    }

    double bestPrecision = -1;
    int bestIndex = -1;
    svm_model* bestModel = 0;

    if(ERR_TYPE == 1){
        LibSVMPool<StandardClassificationAccuracy<double> > MT(cs, prob, val,vm.count("validation-set"),  kcv, epsilon, rocBasePath, saveAll, kernelType, modelFile);
        par_for_each(0, cs.size(), MT);
        bestPrecision = MT.getPrecision();
        bestIndex = MT.getBestIndex();
        bestModel = MT.getBestModel();
    }else if(ERR_TYPE == 2){
        LibSVMPool<EqualErrorRateAccuracy<double> > MT(cs, prob, val,vm.count("validation-set"),  kcv, epsilon, rocBasePath, saveAll, kernelType, modelFile);
        par_for_each(0, cs.size(), MT);
        bestPrecision = MT.getPrecision();
        bestIndex = MT.getBestIndex();
        bestModel = MT.getBestModel();
    }else if(ERR_TYPE == 3){
        check(vm.count("required-tp") , "required true positive rate not set", desc);
        LibSVMPool<FixedTPAccuracy<double> > MT(cs, prob, val,vm.count("validation-set"),  kcv, epsilon, rocBasePath, saveAll, kernelType, modelFile);
        MT.setRequiredTP(requiredTP);
        par_for_each(0, cs.size(), MT);
        bestPrecision = MT.getPrecision();
        bestIndex = MT.getBestIndex();
        bestModel = MT.getBestModel();
    }else{
        throw std::runtime_error("Undefined");
    }

    if(vm.count("validation-set") || kcv > 1){
        cout << "Precision: " << bestPrecision << std::endl << "C: " << cs.at(bestIndex)[0];
        if( cs.at(bestIndex).size() > 1){
            cout << " kernel par: " << cs.at(bestIndex)[1];
        }
        cout << std::endl;
    }
    if(bestIndex >= 0 && modelFile != ""){
        svm_model* mod = bestModel;
        if(mod == 0){
            assert(kcv > 0);
            std::cout << "Recomputing model with the selected parameters..." << std::endl;
            struct svm_parameter par = buildPar(kernelType, cs.at(bestIndex)[1], epsilon, cs.at(bestIndex)[0]) ;
            const char* err = svm_check_parameter(&prob, &par);
            if(err != 0){
                check(false, err, desc);
            }
            mod = svm_train(&prob, &par);
        }
        std::cout << "Saving final model..." << std::endl;
        svm_save_model(modelFile.c_str(), mod);
        svm_free_and_destroy_model(&mod);
    }
    free_svm_problem(prob);
    if(vm.count("validation-set")){
        free_svm_problem(val);
    }
    std::cout << "Done" << std::endl;
    return 0;
}

