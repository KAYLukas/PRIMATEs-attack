
#ifndef Block_Block_H_   /* Include guard */
#define Block_Block_H_

namespace Block {
#define BLOCK_ROWS 5
#define BLOCK_COLUMNS 8
    
    namespace Permutation {
        extern int rounds;
        extern bool skip_first_sub_elements;
        extern bool enable_add_constant;
        extern int add_constant_constant;
    }
    
    
    class Block {
        unsigned char data[BLOCK_ROWS][BLOCK_COLUMNS];
        
        
        void loadBytes(unsigned char* byte_array);
        
        public:
        Block(unsigned char* byte_array);
        
        void setBit(int i, unsigned char val);
        
        unsigned char getBit(int i);
        
        void permute();
        
        void exclusiveor(Block* block);
        void copyTo(Block* block);
        
        void print();
        
        void zero();
    };
}
#endif