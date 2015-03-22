cmake_minimum_required(VERSION 2.4)
project (libMsC)
include_directories (.)

set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}" )

link_libraries(boost_thread boost_system boost_program_options blas)

add_executable(liblinear-learn liblinear-learn.cpp cmd-utils.h dataset_management.h thread_manager.h extern/Liblinear/tron.cpp roc.h roc.cpp parfor.h debug_stream.h extern/Liblinear/linear.cpp)
add_executable(libsvm-learn libsvm-learn.cpp errors_evaluation.h cmd-utils.h dataset_management.h thread_manager.h roc.h roc.cpp parfor.h debug_stream.h extern/Libsvm/svm.h extern/Libsvm/svm.cpp)
