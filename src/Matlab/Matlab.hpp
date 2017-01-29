#ifndef Matlab_Matlab_H_   /* Include guard */
#define Matlab_Matlab_H_

#include <Eigen/Dense>
#include <boost/thread.hpp> 

namespace Matlab {
    class Matlab {
        
        void* ep;
        
        boost::thread* matlabstarter;
        void waitOnEngine();
        
        public:
        
        Matlab();
        ~Matlab();
        
        Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> gflineq(Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> M, Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> b);
        int gfrank(Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> M);
    };
}
#endif