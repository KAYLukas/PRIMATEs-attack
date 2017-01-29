#include <stddef.h>
#include <string.h>
#include <algorithm>

#include <stdio.h>

#include "TermConstants.hpp"


namespace Block {
    
    void TermConstants::_initBlocks (int degree_in){
        block_count = 1;
        for(int i = 0; i < degree_in; i++){
            block_count *= 200;
        }
        blocks = new Block*[block_count];
        memset(blocks, 0, sizeof(Block*) * block_count);
    }

    void TermConstants::_delBlocks(int degree_in){
        block_count = 1;
        for(int i = 0; i < degree_in; i++){
            block_count *= 200;
        }
        for(int i = 0; i < block_count; i++){
            if(blocks[i] != NULL){
                delete blocks[i];
            }
        }
        delete[] blocks;
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

    void TermConstants::_setBlock(unsigned char* W_s, Block* block){
        blocks[_derivToIndex(W_s)] = block;
    }

    Block* TermConstants::_getBlock(unsigned char* W_s){
        return blocks[_derivToIndex(W_s)];
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
        Block* block = new Block(NULL);
        int U_len = V_len + degree;
        unsigned char* U = new unsigned char[U_len];
        for(int i = 0; i < V_len; i++){
            U[i] = V[i];
        }
        for(int i = V_len; i < U_len; i++){
            U[i] = W_s[i-V_len];
            for(int j = 0; j < i; j++){
                if(U[i] == V[j]){
                    _setBlock(W_s, block);
                    return;
                }
            }
        }
        int sum_length = 1 << U_len;
        Block* temp_block = new Block(NULL);
        for (int i = 0; i < sum_length; i++) {
            temp_block->zero();
            for(int j = 0; j < U_len; j++){
                temp_block->setBit(U[j], ((1 << j) & i) >> j);
            }
            //temp_block->print();
            temp_block->permute();
            //temp_block->print();
            block->exclusiveor(temp_block);
        }
        _setBlock(W_s, block);
        delete temp_block;
    }

    TermConstants::TermConstants(int degree_in, unsigned char* V_in, int V_len_in){
        V_len = V_len_in;
        degree = degree_in;
        V = new unsigned char[V_len];
        memcpy(V, V_in, sizeof(unsigned char) * V_len);
        _initBlocks(degree_in);
        
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
        delete[] V;
        _delBlocks(degree);
    }

    unsigned char TermConstants::getTermConstant(int poly_number, unsigned char* W_s) {
        Block* block = _getBlock(W_s);
        //block->print();
        return block->getBit(poly_number);
    }

    void TermConstants::print(){
        unsigned char* W_s = new unsigned char[degree];
        
        for (int i = 0; i < degree; i++){
            W_s[i] = i;
        }
        do{
            printf("W_s = (");
            for(int i = 0; i < degree; i++){
                if(i){
                    printf(", ");
                }
                printf("%i", W_s[i]);
            }
            printf(")\n");
            _getBlock(W_s)->print();
        }while(_nextDiffVector(W_s, degree));
        delete[] W_s;
    }
}