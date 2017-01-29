#include <string.h>
#include <stddef.h>


#include <iostream>
#include <string>

#include "Calculator.hpp"
#include "Function.hpp"

namespace MultiDeriv {
   
    Calculator::Calculator() {
        functions = NULL;
    }
    
    Calculator::~Calculator() {
        if(functions != NULL){
            delete[] functions;
        }
        
        for(std::map<std::string, Block::Block*>::iterator itr = derivVectorToBlock.begin();
                itr != derivVectorToBlock.end(); itr++) {
            delete[] itr->second;
        }
        derivVectorToBlock.clear();
    }
        
    Calculator* Calculator::load(Function** functions_in, int amount_in) {
        if(functions != NULL){
            delete[] functions;
        }
        for(std::map<std::string, Block::Block*>::iterator itr = derivVectorToBlock.begin();
                itr != derivVectorToBlock.end(); itr++) {
            delete[] itr->second;
        }
        derivVectorToBlock.clear();
        
        functions = new Function*[amount_in];
        //std::cout << "amount_in="<< amount_in << std::endl;
        memcpy(functions, functions_in, amount_in * sizeof(Function*));
        amount = amount_in;
        return this;
    }
    
    Block::Block* Calculator::_calculateDeriv(Block::Block* block, unsigned char* V, int degree){
        int sum_length = 1 << degree;
        Block::Block* temp_block = new Block::Block(NULL);
        Block::Block* outputBlock = new Block::Block(NULL);
        for(int i = 0; i < sum_length; i++){
            block->copyTo(temp_block);
            for(int j = 0; j < degree; j++){
                temp_block->setBit(V[j], ((1 << j) & i) >> j);
            }
            temp_block->permute();
            
            outputBlock->exclusiveor(temp_block);
        }
        
        delete temp_block;
        return outputBlock;
    }
        
        
    void Calculator::calculate(Block::Block* block) {
        loaded_block = block;
        //calculate all blocks necessary:
        for(int i = 0; i < amount; i++){
            Function* h = functions[i];
            int parts = h->getAmountOfParts();
            
            for(int i = 0; i < parts; i++){
                int degree = h->getDerivationDegreeOfPart(i);
                unsigned char* V = h->getDerivationVectorOfPart(i);
                unsigned char* c_key = new unsigned char[degree + 1];
                c_key[degree] = 0;
                for(int k = 0; k < degree; k++){
                    c_key[k] = V[k] + 1;
                }
                std::string key(reinterpret_cast<char*>(c_key));
                delete[] c_key;
                
                if(derivVectorToBlock.count(key) == 0){
                    Block::Block* cache_block = _calculateDeriv(block, V, degree);
                    derivVectorToBlock.insert(std::make_pair(key, cache_block));
                }
            }
        }
    }
    
    Block::Block* Calculator::_getBlock(unsigned char* V, int degree){
        unsigned char* c_key = new unsigned char[degree + 1];
        c_key[degree] = 0;
        for(int i = 0; i < degree; i++){
            c_key[i] = V[i] + 1;
        }
        std::string key(reinterpret_cast<char*>(c_key));
        Block::Block* result = derivVectorToBlock.find(key)->second;
        delete[] c_key;
        return result;
        
        //return _calculateDeriv(loaded_block, V, degree);
    }
        
    int Calculator::getBitOfFunction(int i) {
        Function* h = functions[i];
        int parts = h->getAmountOfParts();
        int result = 0;
        
        for(int i = 0; i < parts; i++){
            int degree = h->getDerivationDegreeOfPart(i);
            unsigned char* V = h->getDerivationVectorOfPart(i);
            int bit = h-> getBitOfPart(i);
            Block::Block* block = _getBlock(V, degree);
            result ^= block->getBit(bit);
            //delete block;
        }
        return result;
        
        /*std::map<unsigned char*, Block::Block*, derivVectorCompare>::iterator it;
        for(int i = 0; i < parts; i++){
            int degree = h->getDerivationDegreeOfPart(i);
            unsigned char* V = h->getDerivationVectorOfPart(i);
            
            unsigned char* key = new unsigned char[degree+1];
            key[0] = degree;
            for(int k = 0; k < degree; k++){
                key[k + 1] = V[k];
            }
            
            it = derivVectorToBlock.find(key);
            if(it == derivVectorToBlock.end()){
                std::cout << "ERRROR" << std::endl;
                result = 0/0;
            }
            Block::Block* block = it->second;
            
            int bit = h->getBitOfPart(i);
            
            result ^= block->getBit(bit);
            delete[] key;
        }
        return result;*/
    }
    
    int Calculator::getBitOfFunctionPrint(int i) {
        Function* h = functions[i];
        h->print();
        return getBitOfFunction(i);
    }
}