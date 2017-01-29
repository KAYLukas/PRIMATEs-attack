
#ifndef Matlab_Rank_Gaussian_H_   /* Include guard */
#define Matlab_Rank_Gaussian_H_

#include <engine.h>
#include <Eigen/Dense>

namespace Matlab {
    int rankGaussian (Engine* ep,  Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> M);
}

#endif