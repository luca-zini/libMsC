#include "roc.h"


void saveRoc(std::ostream &out, const double *yVal, const double *classresult, int N){
    std::vector<float> y;
    for(unsigned i = 0;i < N; i++){
        y.push_back((classresult)[i]);
    }

    std::sort(y.begin(), y.end());
    for(int o = 0; o < y.size(); o++){
        double th = y[o];
        int negN = 0, posN = 0, tp = 0, fp = 0;
        for(unsigned i = 0; i < N; i++){
            if(yVal[i] < 0){
                negN++;
                if(classresult[i] >= th ){
                    fp++;
                }
            }
            if(yVal[i] >= 0){
                posN++;
                if(classresult[i] >= th){
                    tp++;
                }
            }
        }
        out<< th << " " << fp/(float)negN << " " << tp /(float)posN << std::endl;
    }
}
