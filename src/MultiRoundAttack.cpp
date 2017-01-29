#include "api.h"
#include <stdio.h>
#include "primate.h"

#include <fstream>
#include <iterator>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <boost/program_options.hpp>
#include <Eigen/Dense>

#include "Block/TermConstants.hpp"
#include "Block/Block.hpp"
#include "Matlab/Matlab.hpp"

namespace po = boost::program_options;


std::vector<Block::Block*> onlineAttack(Block::Block* inputBlock, std::vector<std::vector<int> > Vs){
    std::vector<Block::Block*> attackBlocks;
    Block::Block* temp_block = new Block::Block(NULL);
    for(int idx = 0; idx < Vs.size(); idx++){
        Block::Block* outputBlock = new Block::Block(NULL);
        std::vector<int> V = Vs[idx];
        int shiftVsize = 1 << V.size();
        for (int i = 0; i < shiftVsize; i++) {
            inputBlock->copyTo(temp_block);
            for(int j = 0; j < V.size(); j++){
                temp_block->setBit(V[j], (i >> j) & 1);
            }
            temp_block->permute();
            outputBlock->exclusiveor(temp_block);
        }
        attackBlocks.push_back(outputBlock);
    }
    delete temp_block;
    return attackBlocks;
}

void assertValidEquation(Block::Block* block,
                    Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 160> > Mb,
                    Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> rightHandSide){
    for(int i = 0; i < Mb.rows(); i++){
        int adder = 0;
        for(int j = 0; j < Mb.cols(); j++){
            adder ^= Mb(i,j) & block->getBit(j + 40);
        }
        
        if(adder != rightHandSide(i,0)){
            printf("Error! righthandSide(%i,0) = %i != %i = (Mb*b)_%i\n",i, rightHandSide(i,0), adder, i);
        }
    }
}

void solveLinearSystem(Matlab::Matlab* matlab,
                        unsigned char* b_out, 
                        Block::Block* block, 
                        std::vector<std::vector<int> > Vs, 
                        std::vector<Block::Block*> outputBlocks, 
                        Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 40> > Ma, 
                        Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 160> > Mb, 
                        Eigen::Map<Eigen::Matrix<unsigned char, 1, Eigen::Dynamic> > c){
   /*
       Solve the given linear system.                          
       M =  (Ma Mb)      x = (a | b)^T
       
       
       f(x) =  M.x + c
            = Ma . a + Mb . b + c
         ==>                   
            Mb . b = f( (a | b)^T  ) + Ma.a + c    
   */
   
   Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> rightHandSide;
   rightHandSide.resize(40*Vs.size(), 1);
   //load f(x)
   for(int i = 0; i < Vs.size(); i++){
       for(int j = 0; j < 40; j++){
           rightHandSide(i*40 + j, 0) = outputBlocks[i]->getBit(j); 
       }
   }
   //add Ma. a
   for(int i = 0; i < Vs.size()*40; i++){
       unsigned char sum = 0;
       for(int j = 0; j < 40; j++){
           sum ^= Ma(i, j) & block->getBit(j);
       }
       rightHandSide(i, 0) = ((unsigned char) rightHandSide(i, 0)) ^ sum;
   }
   //add c
   for(int i = 0; i < Vs.size() * 40; i++){
       rightHandSide(i, 0) = ((unsigned char) rightHandSide(i, 0)) ^ c(0,i);
   }
   //Now solve Mb. b = rightHandSide for b.
   assertValidEquation(block, Mb, rightHandSide);
   Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> x = matlab->gflineq(Mb, rightHandSide);
   for(int i = 0; i < 160; i++){
       b_out[i] = x(i,0);
   }
}

void Attack(Matlab::Matlab* matlab, Block::Block* block, std::string dataFile){
    std::cout << "Reading the precomputed data." << std::endl;
    std::ifstream inFile;
    inFile.open(dataFile.c_str(), std::ios::binary);
    
    int Vs_len = 0;
    int rounds = 0;
    int rc = 0;
    inFile.read((char*) &rounds, sizeof(int));
    Block::Permutation::rounds = rounds;
    inFile.read((char*) &rc, sizeof(int));
    if(rc == -1){
        Block::Permutation::enable_add_constant = false;
        std::cout << "Attack on the " << rounds << " round PRIMATE permutation, with no round constants." << std::endl;
    } else {
        Block::Permutation::enable_add_constant = true;
        Block::Permutation::add_constant_constant = rc;
        std::cout << "Attack on the " << rounds << " round PRIMATE permutation, with round constant " << rc << "." << std::endl;
    }
    inFile.read((char*) &Vs_len, sizeof(int));
    std::vector<std::vector<int> > Vs;
    for(int i = 0; i < Vs_len; i++){
        int V_len = 0;
        inFile.read((char*) &V_len, sizeof(int));
        int* buffer = new int[V_len];
        inFile.read((char*) buffer, sizeof(int)*V_len);
        std::vector<int> V(buffer, buffer+V_len);
        Vs.push_back(V);
        delete[] buffer;
    }
    std::cout << "Read " << Vs_len <<  " differentiation vectors. " << std::endl;
    
    std::cout << "The differentiation vectors are:";
    for(int i = 0; i < Vs.size(); i++){
        std::cout << " [";
        for(int j = 0; j < Vs[i].size(); j++){
            std::cout << Vs[i][j];
            if(j + 1 != Vs[i].size()){
                std::cout << ",";
            }
        }
        std::cout << "]";
    }
    std::cout << "." << std::endl;
    
    unsigned char* Ma_buffer = new unsigned char[40*Vs.size() * 40];
    unsigned char* Mb_buffer = new unsigned char[40*Vs.size() * 160];
    unsigned char* c_buffer = new unsigned char[40*Vs.size()];
    
    inFile.read((char*) Ma_buffer, sizeof(unsigned char)* 40*Vs.size() *40);
    inFile.read((char*) Mb_buffer, sizeof(unsigned char)* 40*Vs.size() *160);
    inFile.read((char*) c_buffer, sizeof(unsigned char)* 40*Vs.size());
    
    Eigen::Map<Eigen::Matrix<unsigned char, 1, Eigen::Dynamic> > c(c_buffer, 1, 40*Vs.size());
    Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 40> > Ma(Ma_buffer, 40*Vs.size(), 40);
    Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 160> > Mb(Mb_buffer, 40*Vs.size(), 160);
    
    std::cout << "Linear system loaded from the precomputed data." << std::endl;
    
    std::cout << "Performing online attack:" << std::endl;
    
    std::vector<Block::Block*> outputBlocks = onlineAttack(block, Vs);
    std::cout << "Online attack performed. Solving the linear system..." << std::endl;
    
    unsigned char* b = new unsigned char[160];
    solveLinearSystem(matlab, b, block, Vs, outputBlocks, Ma, Mb, c);
    bool error = false;
    for(int i = 0; i < 160; i++){
       if(b[i] != block->getBit(40 + i)){
           printf("Error! bit %i was not correct! (%i != %i)\n", i, b[i], block->getBit(40+i));
           error = true;
       }
    }
    printf("We recovered the following internal state: \n");   
    for(int i = 0; i < 4; i++){
        std::cout << "    " << i;
        for(int j = 0; j < 5;j++){
            int byte = 0;
            for(int k = 0; k < 8; k++){
                byte ^= b[(i*5 + j)*8 + k] << k;
            }
            if(byte > 0x80 || byte < 0x20){
                printf(" (0x%02X) ", byte);
            } else {
                std::cout <<  "  |" << (char) byte << "|   ";
            }
        }
        std::cout << std::endl;
    }
    
    if(!error){
        printf("This result is correct.\n");
    } else {
        printf("This result is incorrect. \n");
    }
    
}

void viewPolynomial(int bit){
    unsigned char V[] = {};
    Block::TermConstants* constantTerms = new Block::TermConstants(0, V, 0);
    Block::TermConstants* linearTerms = new Block::TermConstants(1, V, 0);
    Block::TermConstants* quadraticTerms = new Block::TermConstants(2, V, 0);
    
    for(unsigned char i1 = 0; i1 < 200; i1++){
        for(unsigned char i2 = i1 + 1; i2 < 200; i2++){
            unsigned char W_s[] = {i1, i2};
            int constant = quadraticTerms->getTermConstant(bit, W_s);
            if(constant != 0){
                if(i1 > 39){
                    printf("b%i", i1 - 40);
                    
                } else {
                    printf("a%i", i1);
                }
                printf("*");
                if(i2 > 39){
                    printf("b%i", i2 - 40);
                    
                } else {
                    printf("a%i", i2);
                }
                printf(" + ");
            }
        }
    }
    
    
    for(unsigned char i1 = 0; i1 < 200; i1++){
        unsigned char W_s[] = {i1};
        int constant = linearTerms->getTermConstant(bit, W_s);
        if(constant != 0){
            if(i1 > 39){
                printf("b%i", i1 - 40);
                
            } else {
                printf("a%i", i1);
            }
            printf(" + ");
        }
    }
    int constant_constant = constantTerms->getTermConstant(bit, NULL);
    
    printf("%i\n", constant_constant);
    
    printf("\n");
}

void printQuadraticPolynomial(int j, Block::TermConstants* tc0, Block::TermConstants* tc1, Block::TermConstants* tc2){
    for(int i = 0; i < 160; i++){
        for(int j = i + 1; j < 160; j++){
            unsigned char W_s[] = {i,j};
            if(tc2->getTermConstant(j, W_s)){
                if(i > 39){
                    printf("b%i", i - 40);
                } else {
                    printf("a%i", i);
                }
                if(j > 39){
                    printf("b%i", j - 40);
                } else {
                    printf("a%i", i);
                }
                printf(" + ");
            }
        }
    }
    for(int i = 0; i < 160; i++){
        unsigned char W_s[] = {i};
        if(tc1->getTermConstant(j, W_s)){
            if(i > 39){
                printf("b%i", i - 40);
            } else {
                printf("a%i", i);
            }
            printf(" + ");
        }
    }
    printf("%i\n", tc0->getTermConstant(j, NULL));
}

void checkDiffVectors(Matlab::Matlab* matlab, std::vector<std::vector<int> > Vs, bool output_lin_system, bool linear_check){
    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> linearSystem;
    linearSystem.resize(40 * Vs.size(), 160);
    for(int i = 0; i < Vs.size(); i++){
        unsigned char* V = new unsigned char[Vs[i].size()];
        for(int j = 0; j < Vs[i].size(); j++){
            V[j] = (unsigned char) Vs[i][j];
        }
        
        Block::TermConstants* tc1 = new Block::TermConstants(1, V, Vs[i].size());
        
        Block::TermConstants* tc2;
        if(linear_check){
            tc2 = new Block::TermConstants(2, V, Vs[i].size());
        }
        
        for(int j = 0; j < 40; j++){
            for(unsigned char k = 40; k < 200; k++){
                linearSystem(40*i + j, k - 40) = tc1->getTermConstant(j, &k);
            }
        }
        if(linear_check){
            //check for linearity
            for(int j = 0; j < 40; j++){
                for(unsigned char k = 0; k < 200; k++){
                    for(unsigned char k2 = k + 1; k2 < 200; k2++){
                        unsigned char W_s[] = {k, k2};
                        if(tc2->getTermConstant(j, W_s)){
                            std::cout << "Error! The polynomials generated by the set Vs = {";
                            for(int l = 0; l < Vs.size(); l++){
                                std::cout << "[";
                                for(int m = 0; m < Vs[l].size(); m++){
                                    std::cout << Vs[l][m];
                                    if( m + 1 != Vs[l].size()){
                                        std::cout << ",";
                                    }
                                }
                                std::cout << "]";
                                if(l + 1 != Vs.size()){
                                    std::cout << ", ";
                                }
                            }
                            std::cout << "} are not all linear! " << std::endl;
                            std::cout << "Polynomial of bit " << j << " with V = ";
                            for(int l = 0; l < Vs[i].size(); l++){
                                std::cout << (int) V[l];
                                if( l + 1 != Vs[i].size()){
                                    std::cout << ",";
                                }
                            }
                            std::cout << " is not linear." << std::endl;
                            Block::TermConstants* tc0 = new Block::TermConstants(0, V, Vs[i].size());
                            printQuadraticPolynomial(j, tc0, tc1, tc2);
                            delete tc0;
                            delete tc2;
                            delete tc1;
                            delete[] V;
                            return;
                        }
                    }
                }
            }
            delete tc2;
        }
        
        delete tc1;
        delete[] V;
    }
    
    std::cout << "The set Vs = {";
    for(int i = 0; i < Vs.size(); i++){
        std::cout << "[";
        for(int j = 0; j < Vs[i].size(); j++){
            std::cout << Vs[i][j];
            if( j + 1 != Vs[i].size()){
                std::cout << ",";
            }
        }
        std::cout << "]";
        if(i + 1 != Vs.size()){
            std::cout << ", ";
        }
    }
    if(output_lin_system){
        std::cout << "} generates the following linear system:" << std::endl;
        
        for(int i = 0; i < linearSystem.rows(); i++){
            for(int j = 0; j < linearSystem.cols(); j++){
                std::cout << (int) linearSystem(i,j);
                if ( j != linearSystem.cols() - 1){
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
        }
        
        std::cout << "The rank of this matrix is: ";
    } else {
        std::cout << "} generates a linear system with a matrix of rank: ";
    }
    
    
    int rank = matlab->gfrank(linearSystem);
    
    std::cout << rank << ". " << std::endl << std::endl;
    
    std::cout << "    This makes the linear system ";
    if(rank < 160){
        std::cout << "UNDERdetermined :(." << std::endl;
    } else {
        std::cout << "determined :)." << std::endl;
    }
    
}

void precomputeAttack(std::vector<std::vector<int> > Vs, std::string output_file){
    /*
                                    
       M =  (Ma Mb)      x = (a | b)^T
       
       
       f(x) =  M.x + c
            = Ma . a + Mb . b + c
         ==>                   
            Mb . b = f( (a | b)^T  ) + Ma.a + c    
         
         In the precomputation a, b and f(x) are unknown.
         f(x) is determined by the online attack. A is given in the online attack and b should be calculated.
         So we need to store Mb, Ma and c
    */
    Eigen::Matrix<unsigned char, Eigen::Dynamic, 200> M;
    M.resize(40 * Vs.size(), 200);
        
    unsigned char* c_buffer = new unsigned char[40*Vs.size()];
    Eigen::Map<Eigen::Matrix<unsigned char, 1, Eigen::Dynamic> > c(c_buffer, 1, 40*Vs.size());
    for(int i = 0; i < Vs.size(); i++){
        unsigned char* V = new unsigned char[Vs[i].size()];
        for(int j = 0; j < Vs[i].size(); j++){
            V[j] = (unsigned char) Vs[i][j];
        }
        
        Block::TermConstants* tc0 = new Block::TermConstants(0, V, Vs[i].size());
        Block::TermConstants* tc1 = new Block::TermConstants(1, V, Vs[i].size());
        
        for(int j = 0; j < 40; j++){
            for(unsigned char k = 0; k < 200; k++){
                M(40*i + j, k) = tc1->getTermConstant(j, &k);
            }
        }
        
        for(int j = 0; j < 40; j++){
            c(0, 40*i + j) = tc0->getTermConstant(j, NULL);
        }
        delete tc1;
        delete tc0;
        delete[] V;
    }
   
    unsigned char* Ma_buffer = new unsigned char[40*Vs.size() * 40];
    unsigned char* Mb_buffer = new unsigned char[40*Vs.size() * 160];
    
    Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 40> > Ma(Ma_buffer, 40*Vs.size(), 40);
    Eigen::Map<Eigen::Matrix<unsigned char, Eigen::Dynamic, 160> > Mb(Mb_buffer, 40*Vs.size(), 160);
        
    Ma.block(0, 0, 40*Vs.size(), 40) = M.block(0, 0, 40*Vs.size(), 40);
    Mb.block(0, 0, 40*Vs.size(), 160) = M.block(0, 40, 40*Vs.size(), 160);
    
    std::ofstream outFile (output_file.c_str(), std::ios::out | std::ios::binary);
    int v_len = Vs.size();
    outFile.write((char*) &(Block::Permutation::rounds), sizeof(int));
    int rc = Block::Permutation::add_constant_constant;
    if(!Block::Permutation::enable_add_constant){
        rc = -1;
    }
    outFile.write((char*) &rc, sizeof(int));
    outFile.write((char*) &v_len, sizeof(int));
    for(int i = 0; i < Vs.size(); i++){
        v_len = Vs[i].size();
        outFile.write((char*) &v_len, sizeof(int));
        outFile.write((char*) &Vs[i][0], sizeof(int)* v_len);
    }
    
    outFile.write((char*) Ma_buffer, sizeof(unsigned char) * 40*Vs.size() *40);
    outFile.write((char*) Mb_buffer, sizeof(unsigned char) * 40*Vs.size() *160);
    outFile.write((char*) c_buffer, sizeof(unsigned char) * 40*Vs.size());
    outFile.close();
    delete[] Ma_buffer;
    delete[] Mb_buffer;
    delete[] c_buffer;
}

std::vector<int> stringToBitList(std::string str) {
    std::vector<int> result;
    int start = 0;
    for(int i = 0; i < str.size(); i++){
        if(str[i] == ','){
            result.push_back(atoi(str.substr(start, (i - start)).c_str()));
            start = i + 1;
        }
    }
    if(start != str.size()){
        result.push_back(atoi(str.substr(start, (str.size() - start)).c_str()));
    }
    return result;
}

int main(int argc, char** argv){
    //Init:
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("rounds", po::value<int>(), "The amount of rounds the attacker should use REQUIRED for all except attack.")
    ("rc", po::value<int>(), "The starting round constant. Do not pass, if the permutation should not use round constants.")
    ("view_polynomial", po::value<int>(), "View the polynomial of the given bit")
    ("check_diff_vectors", po::value<std::vector<std::string> >()->multitoken(), "Check if the given differentiation vectors yield a fully determined system of linear equations")
    ("precompute_attack", po::value<std::vector<std::string> >()->multitoken(), "Perform the precomputation for the attack given the differentiation vectors.")
    ("file", po::value<std::string>(), "The file from/to which the attack data should be read/written.")
    ("attack", po::value<std::string>(), "Try to attack the given state and find the inner state.")
    ("default_attack", "Try to attack the default state and find the inner state.")
    ("no_matrix", "Don't output the matrix of the whole linear system.")
    ("no_linear_check", "Don't check if the differentiation vectors yield a LINEAR system.");
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help") || (!vm.count("rounds") && !vm.count("attack") && !vm.count("default_attack"))) {
        std::cout << desc << "\n";
        return 1;
    }
    
    if(vm.count("rc")){
        int rc = vm["rc"].as<int>();
        Block::Permutation::enable_add_constant = true;
        Block::Permutation::add_constant_constant = rc;
    } else {
        Block::Permutation::enable_add_constant = false;
        Block::Permutation::add_constant_constant = 0;
    }
    
    if(vm.count("rounds")){
        int rounds = vm["rounds"].as<int>();
    
        Block::Permutation::rounds = rounds;
    }
    
    Block::Permutation::skip_first_sub_elements = true;
    
    if(vm.count("view_polynomial")){
        int bit = vm["view_polynomial"].as<int>();
        viewPolynomial(bit);
        return 0;
    }
    
    if(vm.count("check_diff_vectors")){
        bool output_linear_system = true;
        if(vm.count("no_matrix")){
            output_linear_system = false;
        }
        
        bool linear_check = true;
        if(vm.count("no_linear_check")){
            linear_check = false;
        }
        
        Matlab::Matlab* matlab = new Matlab::Matlab();
        std::vector<std::vector<int> > Vs;
        std::vector<std::string> raw_Vs = vm["check_diff_vectors"].as<std::vector<std::string> >();
        for(int i = 0; i < raw_Vs.size(); i++){
            Vs.push_back(stringToBitList(raw_Vs[i]));
        }
        checkDiffVectors(matlab, Vs, output_linear_system, linear_check);
        
        
        delete matlab;
        return 0;
    }
    
    //An complete attack would be: 0 5 10 15 20 25 30 35
    if(vm.count("precompute_attack") && vm.count("file")){
        
        std::vector<std::vector<int> > Vs;
        std::vector<std::string> raw_Vs = vm["precompute_attack"].as<std::vector<std::string> >();
        for(int i = 0; i < raw_Vs.size(); i++){
            Vs.push_back(stringToBitList(raw_Vs[i]));
        }         
        
        precomputeAttack(Vs, vm["file"].as<std::string>());
        return 0;
    }
    
    if((vm.count("attack") || vm.count("default_attack") )&& vm.count("file")){
        
        Matlab::Matlab* matlab = new Matlab::Matlab();
        unsigned char* byte_array = new unsigned char[25];
        if(vm.count("attack")){
            std::string inputStr = vm["attack"].as<std::string>();
            for(int i = 0; i < 25; i++){
                if(i > inputStr.size()){
                    byte_array[i] = 0x88;
                } else {
                    byte_array[i] = inputStr[i];
                }
            }
        } else {
            strncpy((char*) byte_array, "Okay, what are you doing?", sizeof(unsigned char)* 25);
        }
        
        Block::Block* block = new Block::Block(byte_array);
        
        std::cout << "The starting state is as follows:" << std::endl;
        std::cout << "     ";
        for(int i = 0; i < 5; i++){
           std::cout <<  "   " << i << "    ";
        }
        std::cout << std::endl;
        for(int i = 0; i < 5; i++){
            std::cout << "    " << i;
            for(int j = 0; j < 5;j++){
                if(byte_array[i*5 + j] > 0x80 || byte_array[i*5 + j] < 0x20){
                    printf(" (0x%02X) ", byte_array[i*5 + j]);
                    //std::cout << " (0x88) ";
                } else {
                    std::cout <<  "  |" << byte_array[i*5 + j] << "|   ";
                }
            }
            std::cout << std::endl;
        }
        
        std::cout << "In binary form this translates to:" << std::endl;
        block->print();
        std::cout << "Initiating attack" << std::endl;
        Attack(matlab, block, vm["file"].as<std::string>());
        delete matlab;
        return 0;
    }
}