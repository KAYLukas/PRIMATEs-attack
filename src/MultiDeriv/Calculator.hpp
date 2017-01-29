#ifndef MultiDeriv_Calculator_H_   /* Include guard */
#define MultiDeriv_Calculator_H_

#include <map>

#include "../Block/Block.hpp"
#include "Function.hpp"
#include <string>


namespace MultiDeriv {
    
    class Calculator {
        
        Function** functions;
        Block::Block* loaded_block;
        int amount;
        std::map<std::string, Block::Block*> derivVectorToBlock; 
        Block::Block* _getBlock(unsigned char* V, int degree);
        Block::Block* _calculateDeriv(Block::Block* block, unsigned char* V, int degree);
        
        public:
        Calculator();
        ~Calculator();
        
        Calculator* load(Function** functions_in, int amount_in);
        
        void calculate(Block::Block* block);
        
        int getBitOfFunction(int i);
        
        int getBitOfFunctionPrint(int i);
    };
}

#endif