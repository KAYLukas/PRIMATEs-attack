#ifndef Block_TermConstants_H_   /* Include guard */
#define Block_TermConstants_H_

#include "Block.hpp"

namespace Block {
    class TermConstants { 
        int degree;
        int block_count;
        Block** blocks;
        unsigned char* V;
        int V_len;
        
        void _initBlocks (int degree_in);
        void _delBlocks(int degree_in);
        int _derivToIndex(unsigned char* W_s);
        void _setBlock(unsigned char* W_s, Block* block);
        Block* _getBlock(unsigned char* W_s);
        bool _nextDiffVector (unsigned char* W_s, int degree);
        void _calculateD_s(unsigned char* W_s);
        
        public:
        
        TermConstants(int degree_in, unsigned char* V_in, int V_len_in);
        ~TermConstants();
        unsigned char getTermConstant(int poly_number, unsigned char* W_s);
        void print();
    };
}
#endif