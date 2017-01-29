
#include <stdio.h>
#include <boost/thread.hpp> 
#include <engine.h>
#include <Eigen/Dense>
#include "Matlab.hpp"
#include "RankGaussian.hpp"
#include "SolveGaussianLinearEquation.hpp"
#include <stdlib.h> 
 
 namespace Matlab{
     void task(Engine** epp){
        if(!(epp[0] = engOpen(""))) {
            fprintf(stderr, "\n Can't start MATLAB engine\n");
            exit(EXIT_FAILURE);
        }
     }
     
     Matlab::Matlab(){
        matlabstarter = new boost::thread(task, (Engine **) &ep);
     }
     
     Matlab::~Matlab(){
         waitOnEngine();
         delete matlabstarter;
         engClose((Engine*)ep);
     }
     
     Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> Matlab::gflineq(Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> M, Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> b){
         waitOnEngine();
         return solveGaussianLinearEquation((Engine *)ep, M, b);
     }
     
     int Matlab::gfrank(Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> M){
         waitOnEngine();
         return rankGaussian((Engine *)ep, M);
     }
     
     void Matlab::waitOnEngine(){
         matlabstarter->join();
     }
 }