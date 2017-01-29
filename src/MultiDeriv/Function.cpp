#include <string.h>
#include <stddef.h>
#include <iostream>

#include "Function.hpp"

namespace MultiDeriv {

    Function::Function(unsigned char** V_list_in,  unsigned char* bit_list_in, int amount_of_polynomials_in, int* V_lengths_in){
        V_list = new unsigned char*[amount_of_polynomials_in];
        for(int i = 0; i < amount_of_polynomials_in; i++){
            V_list[i] = new unsigned char[V_lengths_in[i]];
            memcpy(V_list[i], V_list_in[i], sizeof(unsigned char) * V_lengths_in[i]);
        }
        
        bit_list = new unsigned char[amount_of_polynomials_in];
        memcpy(bit_list, bit_list_in, sizeof(unsigned char)*amount_of_polynomials_in);
        
        V_lengths = new int[amount_of_polynomials_in];
        memcpy(V_lengths, V_lengths_in, sizeof(int) * amount_of_polynomials_in);
        
        amount = amount_of_polynomials_in;
    }
    
    Function::~Function(){
        delete[] V_list;
        delete[] bit_list;
        delete[] V_lengths;
    }
        
    int Function::getAmountOfParts(){
        return amount;
    }
        
    int Function::getDerivationDegreeOfPart(int i){
        return V_lengths[i];
    }
        
    unsigned char* Function::getDerivationVectorOfPart(int i){
        return V_list[i];
    }
        
    int Function::getBitOfPart(int i){
        return bit_list[i];
    }
    
    void Function::print(){
        for(int term_idx = 0; term_idx < amount; term_idx++){
            std::cout << "d_{";
            for(int i = 0; i < V_lengths[term_idx]; i++){
                std::cout << (int) V_list[term_idx][i];
                if(i + 1 < V_lengths[term_idx]){
                    std::cout << ", ";
                }
            }
            std::cout << "}";
            std::cout << "^(f_" << (int) bit_list[term_idx] << ")(x)";
            if(term_idx + 1 < amount){
                std::cout << " + ";
            }
        }
        std::cout << std::endl;
    }
        
}