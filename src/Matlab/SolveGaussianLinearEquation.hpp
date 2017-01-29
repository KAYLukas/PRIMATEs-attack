#ifndef Matlab_Solve_Gaussian_Linear_Equation_H_   /* Include guard */
#define Matlab_Solve_Gaussian_Linear_Equation_H_


#include <engine.h> 
#include <Eigen/Dense>
 
namespace Matlab {
    Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> solveGaussianLinearEquation(Engine* ep,
       Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> M,
       Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> b);   
}
#endif