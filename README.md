*The toolbox provides a set of utilities to simplify model selection and assesment of LIBSVM and LIBLINEAR binary classifiers*

## Input format 
Each program accepts the LIBSVM/SVMLight input format or a binary format that is more suitable to store dense data.

* LIBSVM format: each data sample corresponds to one line which contains a label (+1/-1) followed by an ordered sequence of elements in the form idx:val, where idx is the index of the value in the feature vector (counting from 1) and val is the value of the feature at index idx. All the values that are not specified are assumed to be 0.

Example:
+1 1:-0.4 2:0.12 5:0.99
-1 2:-0.2 4:0.2 5:0.42

* BINARY format: the file starts with a 32 bit integer containing the number of samples (N) and another 32 bit integer with the number of features (D) in each sample vector. It follows the dataset stored sample by sample. Each sample is represented with a floating point number containing the label and a vector of floating point numbers of lenght D containing the values. The data can be stored with single precision float (parameter –float) or a double precision (default) number.

## Model evaluation 
It is possible to train and estimate the accuracy of a model both using a validation set or a K-folds Cross Validation (KCV).
The parameter --folds N enable N-folds cross validation. --validation-set specifies the dataset to be used to compute the accuracy.
Parameter --roc specifies the path of an output Receiver Operating Characteristic. The ROC can be computed both with KCV and a validation set. The output file contains a row for each classified sample. Each line has the following format:

threshold false_positive_rate true_positive_rate

the file can be easily plot with common utilities. For example with gnuplot

plot 'file_name' 2:3 using lines

It is possible to compute different accuracy measures:
	- standard accuracy (–err-type 1): evaluates errors using a 0-1 loss
	- equal error rate (--err-type 2): computes the equal error rate
	- false positive rate given a fixed true positive rate (--err-type 3): computes the best false positive rate fixed the minimum true positive rate specified by (–required-tp N)

## Model selection 
It is possible to specify a range of parameters to test using the syntax min_value:n_samples:max_value. If the algorithm accepts multiple parameters (e.g. regularization and kernel parameter) it will be trained with all the combinations of the given parameters. Depending on the parameter the range can be sampled with equally spaced values or with a geometric progression.

By default the algorithms save only the results of the best performing classifier. However it is possible to save all the models and ROCs computed using the flag –save-all. The best model is selected to maximize the error specified by --err-type  

## Multithreading and memory management 
It is possible to exploit multi core processors specifying the flag –mt. In this case it will be created a thread for each logical core, each running the algorithm with different parameters. The training set will be stored in memory once and it will be shared between the instances of the algorithm. This allows to use bigger datasets than running independent instances as it is the case of easy.py and grid.py of libsvm. If the algorithm uses some auxiliary memory during the training (e.g. --cachesize with the libsvm wrapper), this memory will be instantiated for each thread. Running an high number of parallel instances you may need to set a small cache size.

## LibSVM wrapper usage

```
#!verbatim
  --help                produce help message
  --cachesize arg       cache size in mb (-m libsvm option) default 1000
  --kernel arg          kernel type: 
                         0 -- Linear 
                         1 -- Polynomial 
                         2 -- RBF 
                         3 -- Sigmoid 
                         default RBF
  --save-all            save all ROCs and models. If omitted saves only the roc
                        selected with kcv or validation
  --binary              use binary dataset instead of libsvm format
  --roc arg             roc base path and name
  --log arg             log file
  --train-set arg       train data file
  --validation-set arg  validation data file used for model selection
  --c arg               C parameter of the SVM. It can be a scalar or a range 
                        in the form first:N:end. In this second case a 
                        geometric serie of N elements will be used to sample in
                        the range first-end
  --gamma arg           rbf kernel parameter. It can be a scalar or a range in 
                        the form first:N:end. In this second case a geometric 
                        serie of N elements will be used to sample in the range
                        first-end
  --degree arg          degree of polynomial kernel parameter. It can be a 
                        scalar or a range in the form first:N:end. In this 
                        second case a linear span of N elements will be used to
                        sample in the range first-end
  --epsilon arg          set tolerance of termination criterion
                         -s 0 and 2
                         |f'(w)|_2 <= eps*min(pos,neg)/l*|f'(w0)|_2,
                         where f is the primal function and pos/neg are # of
                         positive/negative data (default 0.01)
                         -s 1, 3, 4 and 7
                         Dual maximal violation <= eps; similar to libsvm 
                        (default 0.1)
                         -s 5 and 6
                         |f'(w)|_1 <= eps*min(pos,neg)/l*|f'(w0)|_1,
                        where f is the primal function (default 0.01)
                        
  --err-type arg         Function used to evaluate the error: 
                         1 -- error counting 
                         2 -- equal error rate 
                         3 -- fixed true positive rate 
  --required-tp arg      required TP if is selected fixedTPerror 
  --model arg           output model file
  --folds arg           kcv folds
  --single-precision    the algorithms use single precision floating point 
                        instead of double
  --mt                  it use multithread algorithm

```

## LibLinear wrapper usage

```
#!verbatim

  --help                produce help message
  --save-all            save all ROCs. If omitted saves only the roc selected 
                        with kcv or validation
  --binary              use binary dataset instead of libsvm format
  --roc arg             roc base path and name
  --log arg             log file
  --train-set arg       train data file
  --validation-set arg  validation data file used for model selection
  --c arg               C parameter of the SVM. It can be a scalar or a range 
                        in the form first:N:end. In this second case a 
                        geometric serie of N elements will be used to sample in
                        the range first-end
  --epsilon arg          set tolerance of termination criterion
                         -s 0 and 2
                         |f'(w)|_2 <= eps*min(pos,neg)/l*|f'(w0)|_2,
                         where f is the primal function and pos/neg are # of
                         positive/negative data (default 0.01)
                         -s 1, 3, 4 and 7
                         Dual maximal violation <= eps; similar to libsvm 
                        (default 0.1)
                         -s 5 and 6
                         |f'(w)|_1 <= eps*min(pos,neg)/l*|f'(w0)|_1,
                        where f is the primal function (default 0.01)
                        
  --algorithm arg       set type of solver (default 1)
                         0 -- L2-regularized logistic regression (primal)
                         1 -- L2-regularized L2-loss support vector 
                        classification (dual)
                         2 -- L2-regularized L2-loss support vector 
                        classification (primal)
                         3 -- L2-regularized L1-loss support vector 
                        classification (dual)
                         4 -- multi-class support vector classification by 
                        Crammer and Singer
                         5 -- L1-regularized L2-loss support vector 
                        classification
                         6 -- L1-regularized logistic regression
                         7 -- L2-regularized logistic regression (dual)
                        
  --model arg           output model file
  --folds arg           kcv folds
  --err-type arg         Function used to evaluate the error: 
                         1 -- error counting 
                         2 -- equal error rate 
                         3 -- fixed true positive rate 
  --required-tp arg      required TP if is selected fixedTPerror 
  --single-precision    the algorithms use single precision floating point 
                        instead of double
  --mt                  it use multithread algorithm

```

## Usage examples

```
#!verbatim
$  ./liblinear-learn  --train-set heart_scale --c 1 --model model_name
Done

$  ./liblinear-learn  --train-set heart_scale --c 1e-3:20:1e3 --model model_name --folds 5
Precision: 0.855556
C: 54.5559
Recomputing model with the selected parameters...
Saving final model...
Done

$    ./libsvm-learn  --train-set heart_scale  --validation-set heart_scale --c 1e-3:5:1e3 --kernel 2 --gamma 1e-2:3:1e2 --model model_name --folds 5 --err-type 2
Precision: 1
C: 0.001 kernel par: 100
Saving final model...
Done
```

## Licence
Copyright 2014 by Luca Zini and Francesca Odone,
Department of Informatics, Bioengineering, Robotics, and Systems
Engineering University of Genova.

This is a free software: you can redistribute it and/or modify it
under the terms of the CC-BY Public License, Version 4.0, 25
November 2013 http://creativecommons.org/licenses/by/4.0/legalcode.
This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY, including but not limited to
the warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Author Luca Zini (luca.zini@gmail.com)

The toolbox includes the source code of Liblinear 
http://www.csie.ntu.edu.tw/~cjlin/liblinear/
and LibSVM
http://www.csie.ntu.edu.tw/~cjlin/libsvm/
both the packages have been slightly modified to integrate them in the toolbox.
 
### Liblinear

Copyright (c) 2007-2014 The LIBLINEAR Project.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither name of copyright holders nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### LibSVM

Copyright (c) 2000-2014 Chih-Chung Chang and Chih-Jen Lin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither name of copyright holders nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.