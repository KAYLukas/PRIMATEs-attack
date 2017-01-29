/*
 * applyPermutation is a polynomial f(a | b) of degree d = 2*PERMUTATION_COUNT
 * we reduce the degree by differentiation the polynomial.
 * Differentiating 2*PERMUTATION_COUNT - 1 makes it a linear polynomial f(d - 1)(a | b),
 * HOWEVER f(d - 1)(a | b) is independent of b, the terms are only non-zero for the a.
 * a is known, and our attack needs to find information about b.
 * as a is known, we construct a polynomial g(b) = f(d - 2)(a | b), which is linear in b.
 *
 * So we need to calculate our terms d_S of f(d-2), INCLUDING quadratic terms.
 * then we check if the quadratic terms for (S \subseteq b) are zero.
 * next we reduce the quadratic terms to linear terms by filling the value of a in or 
 * to constant terms if (S \union b) = {}
 
 x^4 x^3 x^2 x^1 x^0
 x^2 x^1 x^0
 
 quadratic approach: numbers close to each other have influence
 other parts not
 For almost any deriviation if i is not close to any other element in 
 
 
 */
#include "api.h"
#include <stdio.h>
#include "primate.h"

#include <fstream>
#include <iterator>
#include <string.h>
#include <vector>
#include <iostream>

#include "Block/Block.hpp"

void loadBytes(unsigned char* byte_array, unsigned char* state){
    int idx = 0;
    int offset = 0;
    for (int i = 0; i < BLOCK_ROWS; i++){
        for (int j = 0; j < BLOCK_COLUMNS; j++) {
            short twobytes = byte_array[idx];
            if(idx < 24){
                twobytes |= byte_array[idx+1] << 8;
            }
            state[i * 8 + j] = (twobytes >> offset) & 0x1F;
            offset += 5;
            idx += offset >> 3;
            offset &= 0x7;
        }
    }
}

int main(int argc, char** argv){
    
    //Init:
    //Init the permutations.
    GenerateRoundConstants();
    Block::Permutation::skip_first_sub_elements = false;
    Block::Permutation::enable_add_constant = true;
    
    unsigned char* byte_array = new unsigned char[25];
    unsigned char* state = new unsigned char[40];
    strncpy(reinterpret_cast <char*>(byte_array), "OKay, what are you doing?", sizeof(unsigned char)* 25);
    //p1
    Block::Permutation::rounds = 12;
    Block::Permutation::add_constant_constant = 1;
    
    
    Block::Block* block = new Block::Block(byte_array);
    block->permute();
    
    loadBytes(byte_array, state);
    p_1(state);
    
    for(int i = 0; i < 200; i++){
        int bit = state[i / 5] >> (i % 5) & 1;
        if(bit != block->getBit(i)){
            std::cout << "Error bit " << i << " of the reference implementation (p_1) does not match the custom implementation." << std::endl;
            std::cout << "Reference implementation:" << std::endl;
            PrintElementsStateDec((unsigned char*)state);
            std::cout << "Custom implementation:" << std::endl;
            block->print();
            return 1;
        }
    }
    
    std::cout << "p_1 is correct!" << std::endl;
    std::cout << "Reference implementation:" << std::endl;
    PrintElementsStateDec((unsigned char*)state);
    std::cout << "Custom implementation:" << std::endl;
    block->print();
    delete block;
    
    
    //p2
    Block::Permutation::rounds = 6;
    Block::Permutation::add_constant_constant = 24;
    
    block = new Block::Block(byte_array);
    block->permute();
    
    loadBytes(byte_array, state);
    p_2(state);
    
    for(int i = 0; i < 200; i++){
        int bit = state[i / 5] >> (i % 5) & 1;
        if(bit != block->getBit(i)){
            std::cout << "Error bit " << i << " of the reference implementation (p_2) does not match the custom implementation." << std::endl;
            std::cout << "Reference implementation:" << std::endl;
            PrintElementsStateDec((unsigned char*)state);
            std::cout << "Custom implementation:" << std::endl;
            block->print();
            return 1;
        }
    }
    std::cout << "p_2 is correct!" << std::endl;
    std::cout << "Reference implementation:" << std::endl;
    PrintElementsStateDec((unsigned char*)state);
    std::cout << "Custom implementation:" << std::endl;
    block->print();
    delete block;
    
    //p3
    Block::Permutation::rounds = 6;
    Block::Permutation::add_constant_constant = 30;
    block = new Block::Block(byte_array);
    block->permute();
    
    loadBytes(byte_array, state);
    p_3(state);
    
    for(int i = 0; i < 200; i++){
        int bit = state[i / 5] >> (i % 5) & 1;
        if(bit != block->getBit(i)){
            std::cout << "Error bit " << i << " of the reference implementation (p_3) does not match the custom implementation." << std::endl;
            std::cout << "Reference implementation:" << std::endl;
            PrintElementsStateDec((unsigned char*)state);
            std::cout << "Custom implementation:" << std::endl;
            block->print();
            return 1;
        }
    }
    
    std::cout << "p_3 is correct!" << std::endl;
    std::cout << "Reference implementation:" << std::endl;
    PrintElementsStateDec((unsigned char*)state);
    std::cout << "Custom implementation:" << std::endl;
    block->print();
    delete block;
    //p4
    Block::Permutation::rounds = 12;
    Block::Permutation::add_constant_constant = 24;
    
    block = new Block::Block(byte_array);
    block->permute();
    
    loadBytes(byte_array, state);
    p_4(state);
    
    for(int i = 0; i < 200; i++){
        int bit = state[i / 5] >> (i % 5) & 1;
        if(bit != block->getBit(i)){
            std::cout << "Error bit " << i << " of the reference implementation (p_4) does not match the custom implementation." << std::endl;
            std::cout << "Reference implementation:" << std::endl;
            PrintElementsStateDec((unsigned char*)state);
            std::cout << "Custom implementation:" << std::endl;
            block->print();
            return 1;
        }
    }
    std::cout << "p_4 is correct!" << std::endl;
    std::cout << "Reference implementation:" << std::endl;
    PrintElementsStateDec((unsigned char*)state);
    std::cout << "Custom implementation:" << std::endl;
    block->print();
    delete block;
}