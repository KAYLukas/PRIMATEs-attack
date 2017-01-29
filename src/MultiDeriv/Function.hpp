#ifndef MultiDeriv_Function_H_   /* Include guard */
#define MultiDeriv_Function_H_

namespace MultiDeriv {
    class Function {
        unsigned char** V_list;
        unsigned char* bit_list;
        int amount;
        int* V_lengths;
        public:
        Function(unsigned char** V_list_in,  unsigned char* bit_list_in, int amount_of_polynomials_in, int* V_lengths_in);
        ~Function();
        
        int getAmountOfParts();
        
        int getDerivationDegreeOfPart(int i);
        
        unsigned char* getDerivationVectorOfPart(int i);
        
        int getBitOfPart(int i);
        
        void print();
    };
}

#endif