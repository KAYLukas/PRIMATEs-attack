#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <engine.h>
#include <Eigen/Dense>
#include "RankGaussian.hpp"

#define BUFSIZE 1024*1024

namespace Matlab {
    
    void __rank_debug(const char * str){
        //printf(str);
    }

    void __rank_matlab_console(Engine* ep){
        char *buffer = new char[BUFSIZE + 1], *str = new char[BUFSIZE + 1];
        
        engOutputBuffer(ep, buffer, BUFSIZE);
        while(1){
            printf(">>>");
            fgets(str, BUFSIZE, stdin);

            /*
                * Evaluate input with engEvalString
                */
            engEvalString(ep, str);

            /*
                * Echo the output from the command.  
                */
            printf("%s", buffer);
        }
        engOutputBuffer(ep, NULL, 0);
        delete[] buffer;
        delete[] str;
    }

    #define matrix_type Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>
    int rankGaussian (Engine* ep,  matrix_type M) {
        mxArray *M_matlab = NULL, *b_matlab = NULL, *x_matlab;
        
        char* data_ptr;
        
        __rank_debug("Transfering M to MATLAB\n");
        M_matlab = mxCreateNumericMatrix(M.rows(), M.cols(), mxINT8_CLASS, mxREAL);
        __rank_debug("Preparing the data for transfer\n");
        data_ptr = (char*) mxGetData(M_matlab);
        
        for(int i = 0; i < M.rows(); i++){
            for(int j = 0; j < M.cols(); j++){
                data_ptr[j * M.rows() + i] = M(i,j);
            }
        }
        
        __rank_debug("Sending the data to MATLAB\n");
        engPutVariable(ep, "M", M_matlab);
        __rank_debug("Data send!\n");
        
        //__solve_matlab_console(ep);
        __rank_debug("gflineq... \n");
        engEvalString(ep, "x = int32(gfrank(double(M)))");
        
        
        if ((x_matlab = engGetVariable(ep,"x")) == NULL) {
            fprintf(stderr, "\n Couldn't retrieve the result from the MATLAB engine.\n");
        }
        
    
        data_ptr = (char* ) mxGetData(x_matlab);
        
        int result = ((int*) data_ptr)[0];
        
        //clean up.
        mxDestroyArray(x_matlab);
        mxDestroyArray(M_matlab);
        
        return result;
    }

}