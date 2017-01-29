# PRIMATEs-attack
Proof-of-concept for a chosen plaintext attack on the PRIMATE permutation `PRIMATE-80`, using a cube attack

This repository gives the proof-of-concept discussed in my master's thesis. For information about the attack, you are advised to read: http://repository.tue.nl/858562

## Dependencies
- MATLAB
- C++ Boost library
- Eigen library

**Note**:
ensure the path variables correctly point to matlab.
```
export PATH='/Applications/MATLAB_R2014a.app/bin':$PATH
export DYLD_LIBRARY_PATH='/Applications/MATLAB_R2014a.app/bin/maci64:/Applications/MATLAB_R2014a.app/sys/os/maci64':$DYLD_LIBRARY_PATH
```

## Usage

`MultiRoundAttack.cpp` can be compiled into an executable.
This executable can attack the `PRIMATE-80` permutation family and reduced-round variants of the `PRIMATE-80` permutation family.

To document the usage of the executable, we will show how to perform an attack on a reduced round version of the `p3` of the `PRIMATE-80` permutation. For more information about the executable usages, you can use
```
./MultiRoundAttack --help
```
We will try to attack a reduced round variant of `p3` where it only uses 3 rounds of the `PRIMATE` round function.

To attack this permutation, we first need to determine which bits the cube attack can vary. This means that we need to select multiple sets of integers pointing to a set of input bits. As the outer state of our permutation has 40 bits, we can only select numbers between 0 and 39. What these sets of bit locations actually does is documented in my thesis. 

For instance, using `--check_diff_vectors` we found that the sets of input bits can be used to find a determined linear system.
```
./MultiRoundAttack --rounds 3 --rc 30 --check_diff_vectors 0,1,2, 1,3,4, 2,3,4, 0,1,3 5,6,7 6,8,9 7,8,9 5,6,8 10,11,12, 11,13,14, 12,13,14, 10,11,13 15,16,17 16,18,19 17,18,19 15,16,18 20,21,22, 21,23,24, 22,23,24, 20,21,23 25,26,27 26,28,29 27,28,29 25,26,28 30,31,32 31,33,34, 32,33,34, 30,31,33 35,36,37 36,38,39 37,38,39 35,36,38
```
Note that we use `--rounds 3` to indicate we attack the reduced round variant with 3 `PRIMATE` rounds and  `--rc 30` initializes the round constant to 30, corresponding to the `p3` variant of `PRIMATE-80`.

Once we found a set of input bits that produce a determined linear system, the equations in this system need to be calculated. To this end, we do:
```
./MultiRoundAttack --rounds 3 -rc 30 --precompute_attack 0,1,2, 1,3,4, 2,3,4, 0,1,3 5,6,7 6,8,9 7,8,9 5,6,8 10,11,12, 11,13,14, 12,13,14, 10,11,13 15,16,17 16,18,19 17,18,19 15,16,18 20,21,22, 21,23,24, 22,23,24, 20,21,23 25,26,27 26,28,29 27,28,29 25,26,28 30,31,32 31,33,34, 32,33,34, 30,31,33 35,36,37 36,38,39 37,38,39 35,36,38 --file tripleroundattack.bin
```
Which stores the equation in `tripleroundattack.bin`. Note that we do not need information about the message we are attacking, so this part can be computed offline.

Next we specify that state of the primate construction and try to derive this state using only the outer state. Note that the first row of the state "Hello" is the outer state, and the remaining part is the inner state. We execute:
```
./MultiRoundAttack --rounds 3 --file tripleroundattack.bin --attack "Hello, world"
```
This outputs:
```
The starting state is as follows:
        0       1       2       3       4    
    0  |H|     |e|     |l|     |l|     |o|   
    1  |,|     | |     |w|     |o|     |r|   
    2  |l|     |d|    (0x00)  (0x88)  (0x88) 
    3 (0x88)  (0x88)  (0x88)  (0x88)  (0x88) 
    4 (0x88)  (0x88)  (0x88)  (0x88)  (0x88) 
In binary form this translates to:
 8 10 25 24 6 22 29 13
 12 1 8 14 23 23 9 14
 12 3 25 0 0 4 2 17
 8 4 2 17 8 4 2 17
 8 4 2 17 8 4 2 17

Initiating attack
Reading the precomputed data.
Read 32 differentiation vectors. 
The differentiation vectors are: [0,1,2] [1,3,4] [2,3,4] [0,1,3] [5,6,7] [6,8,9] [7,8,9] [5,6,8] [10,11,12] [11,13,14] [12,13,14] [10,11,13] [15,16,17] [16,18,19] [17,18,19] [15,16,18] [20,21,22] [21,23,24] [22,23,24] [20,21,23] [25,26,27] [26,28,29] [27,28,29] [25,26,28] [30,31,32] [31,33,34] [32,33,34] [30,31,33] [35,36,37] [36,38,39] [37,38,39] [35,36,38].
Linear system loaded from the precomputed data.
Performing online attack:
Online attack performed. Solving the linear system...
We recovered the following internal state: 
    0  |,|     | |     |w|     |o|     |r|   
    1  |l|     |d|    (0x00)  (0x88)  (0x88) 
    2 (0x88)  (0x88)  (0x88)  (0x88)  (0x88) 
    3 (0x88)  (0x88)  (0x88)  (0x88)  (0x88) 
This result is correct.
```
We see that the attack has correctly calculated the secret inner state.