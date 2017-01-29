#ifndef MultiDeriv_TermConstants_H_   /* Include guard */
#define MultiDeriv_TermConstants_H_

#include "Function.hpp"

namespace MultiDeriv {
    class TermConstants { 
        
        int degree;
        Function** functions;
        int amount_of_functions;
        unsigned char** bit_arrays;
        
        
        void _initBitArray (int degree_in);

        void _delBitArray(int degree_in);

        int _derivToIndex(unsigned char* W_s);

        void _setBitArray(unsigned char* W_s, unsigned char* bit_array);

        unsigned char* _getBitArray(unsigned char* W_s);
        bool _nextDiffVector (unsigned char* W_s, int degree);
        void _calculateD_s(unsigned char* W_s);
        public:
        
        TermConstants(int degree_in, Function** functions_in, int amount_of_functions_in);
        ~TermConstants();
        unsigned char getTermConstant(Function* function, unsigned char* W_s);
        
        unsigned char getTermConstantByIndex(int i, unsigned char* W_s);
    };
}
#endif