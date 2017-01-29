#include <string.h>
#include <algorithm>

#include "TermConstants.hpp"
#include "Calculator.hpp"
#include "../Block/Block.hpp"

namespace MultiDeriv {
    
    void TermConstants::_initBitArray (int degree_in){
        int count = 1;
        for(int i = 0; i < degree_in; i++){
            count *= 200;
        }
        bit_arrays = new unsigned char*[count];
        memset(bit_arrays, 0, sizeof(unsigned char*) * count);
    }

    void TermConstants::_delBitArray(int degree_in){
        int count = 1;
        for(int i = 0; i < degree_in; i++) {
            count *= 200;
        }
        for(int i = 0; i < count; i++){
            if(bit_arrays[i] != NULL){
                delete[] bit_arrays[i];
            }
        }
        delete[] bit_arrays;
    }

    int TermConstants::_derivToIndex(unsigned char* W_s){
        std::sort(W_s, W_s + degree);
        int index = 0;
        int increment = 1;
        for(int i = 0; i < degree; i++){
            index += increment*W_s[i];
            increment *= 200;
        }
        return index;
    }

    void TermConstants::_setBitArray(unsigned char* W_s, unsigned char* bit_array){
        bit_arrays[_derivToIndex(W_s)] = bit_array;
    }

    unsigned char* TermConstants::_getBitArray(unsigned char* W_s){
        return bit_arrays[_derivToIndex(W_s)];
    }

    
    
    
    
    bool TermConstants::_nextDiffVector (unsigned char* W_s, int degree) {
        for(int i = degree - 1; i >= 0 ; i--){
            W_s[i] = W_s[i] + 1;
            if(i == 0){
                //printf("Increase in the largest counter: %i, degree: %i\n", W_s[i], degree);
            }
            for(int j = i + 1; j < degree; j++){
                W_s[j] = W_s[j-1] + 1;
            }
            bool okay = true;
            for(int j = i; j < degree; j++){
                if(W_s[j] >= 200){
                    okay = false;
                }
            }
            if(okay){
                return true;
            }
        }
        return false;
    }
    
    void TermConstants::_calculateD_s(unsigned char* W_s){
        Block::Block* block = new Block::Block(NULL);
        
        unsigned char * bits = new unsigned char[amount_of_functions];
        memset(bits, 0, sizeof(unsigned char)* (amount_of_functions));
        
        int sum_length = 1 << degree;
        Block::Block* temp_block = new Block::Block(NULL);
        for (int i = 0; i < sum_length; i++) {
            temp_block->zero();
            for(int j = 0; j < degree; j++){
                temp_block->setBit(W_s[j], ((1 << j) & i) >> j);
            }
            Calculator* calculator = new Calculator();
            calculator->load(functions, amount_of_functions);
            calculator->calculate(temp_block);
            for(int j = 0; j < amount_of_functions; j++){
                bits[j] ^= calculator->getBitOfFunction(j);
            }
            delete calculator;
        }
        delete temp_block;
        _setBitArray(W_s, bits);
        
    }
    
    TermConstants::TermConstants(int degree_in, Function** functions_in, int amount_of_functions_in){
        degree = degree_in;
        functions = functions_in;
        amount_of_functions = amount_of_functions_in;
        _initBitArray(degree);
        
        
        unsigned char* W_s = new unsigned char[degree_in];
        
        for (int i = 0; i < degree_in; i++){
            W_s[i] = i;
        }
        
        do{
            _calculateD_s(W_s);
        }while(_nextDiffVector(W_s, degree_in));
        
        delete[] W_s;
    }
    
    TermConstants::~TermConstants(){
        _delBitArray(degree);
    }
    
    unsigned char TermConstants::getTermConstant(Function* function, unsigned char* W_s) {
        for (int i = 0; i < amount_of_functions; i++){
            if(functions[i] == function){
                return getTermConstantByIndex(i, W_s);
            }
        }
        return -1;
    }
    
    unsigned char TermConstants::getTermConstantByIndex(int i, unsigned char* W_s) {
        unsigned char* bits = _getBitArray(W_s);
        //block->print();
        return bits[i];
    }
}