#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "Block.hpp"
#include "../primate.h"

namespace Block {
    unsigned char sbox[32]= {1, 0, 25, 26, 17, 29, 21, 27, 20,  5,  4, 23, 14, 18,  2, 28, 15,  8,  6,  3, 13,  7, 24, 16, 30,  9, 31,  10,  22,  12,  11,  19};
    
    namespace Permutation{
        int rounds = 1;
        bool skip_first_sub_elements = false;
        bool enable_add_constant = false;
        int add_constant_constant = 0;
    }

    void Block::loadBytes(unsigned char* byte_array){
        int idx = 0;
        int offset = 0;
        for (int i = 0; i < BLOCK_ROWS; i++){
            for (int j = 0; j < BLOCK_COLUMNS; j++) {
                short twobytes = byte_array[idx];
                if(idx < 24){
                    twobytes |= byte_array[idx+1] << 8;
                }
                data[i][j] = (twobytes >> offset) & 0x1F;
                offset += 5;
                idx += offset >> 3;
                offset &= 0x7;
            }
        }
    }

    Block::Block(unsigned char* byte_array) {
        if(byte_array == NULL){
            zero();
        } else {
            loadBytes(byte_array);
        }
        //PrintElementsStateDec(byte_array);
        
        //print();
    }

    void Block::setBit(int i, unsigned char val){
        int offset = i % 5;
        int bitnr = i /5;
        int column = bitnr & 0x7;
        int row = bitnr >> 3;
        data[row][column] &= (~(1 << (offset)));
        data[row][column] |= (val << (offset));
    }

    unsigned char Block::getBit(int i) {
        int offset = i % 5;
        int bitnr = i / 5;
        int column = bitnr & 0x7;
        int row = bitnr >> 3;
        return (data[row][column] >> offset) & 1;
    }

    void Block::permute(){
        //for(int i = 0; i < COLUMNS; i++){
        //    data[0][i] = sbox[data[0][i]];
        //}
        if(Permutation::skip_first_sub_elements){
            InvSubElements((unsigned char*) data);
        }
        int rc = Permutation::add_constant_constant;
        for(int i = 0; i < Permutation::rounds; i++){
            primate((unsigned char*) data);
            if(Permutation::enable_add_constant){
                data[1][1] ^= rc;
                //printf("%i\n", rc);
                int bit = ((rc >> 4) ^ (rc >> 1)) & 1;
                rc = bit | ((rc << 1) & 0x1F);
            }
        }
    }

    void Block::exclusiveor(Block* block){
        for (int i = 0; i < BLOCK_ROWS; i++){
            for (int j = 0 ; j < BLOCK_COLUMNS; j++){
                data[i][j] ^= block->data[i][j];
            }
        }
    }

    void Block::copyTo(Block* block){
        memcpy(block->data, data, sizeof(data));
    }

    void Block::print(){
        PrintElementsStateDec((unsigned char*)data);
    }

    void Block::zero(){
        memset(data, 0, sizeof(data));
    }
}