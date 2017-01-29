#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <engine.h>
#include <Eigen/Dense>
#include "SolveGaussianLinearEquation.hpp"

#define BUFSIZE 1024*1024

namespace Matlab {
    
    void __solve_debug(const char * str){
        //printf(str);
    }

    void __solve_matlab_console(Engine* ep){
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
    #define col_type  Eigen::Matrix<unsigned char, Eigen::Dynamic, 1>
    col_type solveGaussianLinearEquation (Engine* ep,  matrix_type M, col_type b) {
    
        mxArray *M_matlab = NULL, *b_matlab = NULL, *x_matlab;
        
        char* data_ptr;
        __solve_debug("Transfering M to MATLAB\n");
        M_matlab = mxCreateNumericMatrix(M.rows(), M.cols(), mxINT8_CLASS, mxREAL);
        __solve_debug("Preparing the data for transfer\n");
        data_ptr = (char*) mxGetData(M_matlab);
        
        for(int i = 0; i < M.rows(); i++){
            for(int j = 0; j < M.cols(); j++){
                data_ptr[j * M.rows() + i] = M(i,j);
            }
        }
        
        __solve_debug("Sending the data to MATLAB\n");
        engPutVariable(ep, "M", M_matlab);
        __solve_debug("Data send!\n");
        
        __solve_debug("Transfering b to MATLAB\n");
        b_matlab = mxCreateNumericMatrix(b.rows(), b.cols(), mxINT8_CLASS, mxREAL);
        __solve_debug("Preparing the data for transfer\n");
        data_ptr = (char *) mxGetData(b_matlab);
        
        for(int i = 0; i < b.rows(); i++){
            for(int j = 0; j < b.cols(); j++){
                data_ptr[j * b.rows() + i] = b(i,j);
            }
        }
        __solve_debug("Sending the data to MATLAB\n");
        engPutVariable(ep, "b", b_matlab);
        __solve_debug("Data send!\n");
            
        //__solve_matlab_console(ep);
        __solve_debug("gflineq... \n");
        engEvalString(ep, "x = int8(gflineq(double(M),double(b)))");
        
        //__solve_matlab_console(ep);
        if ((x_matlab = engGetVariable(ep,"x")) == NULL) {
            fprintf(stderr, "\n Couldn't retrieve the result from the MATLAB engine.\n");
        }
        
        //does not have to be freed.
        const mwSize* size = mxGetDimensions(x_matlab);
        if(size[0] == 0 && size[1] == 0){
            fprintf(stderr, "\n No solution could be found for the linear system.\n");
            exit(EXIT_FAILURE);
        }
        col_type x;
        //printf("%ix%i", size[0], size[1]);
        x.resize(size[0], size[1]);
        
        data_ptr = (char* ) mxGetData(x_matlab);
        
        for(int i = 0; i < size[0]; i++){
            for(int j = 0; j < size[1]; j++){
                x(i,j) = data_ptr[j*size[1] + i];
            }
        }
        
        //clean up.
        mxDestroyArray(x_matlab);
        mxDestroyArray(M_matlab);
        mxDestroyArray(b_matlab);
        
        return x;
    }

}