set -o verbose
mkdir build
mkdir build/Block
mkdir build/Matlab
mkdir build/MultiDeriv
mkdir dist
mex -g -c src/Block/Block.cpp -outdir build/Block
mex -g -c src/Block/TermConstants.cpp -outdir build/Block
mex -g -c src/MultiDeriv/Calculator.cpp -outdir build/MultiDeriv
mex -g -c src/MultiDeriv/Function.cpp -outdir build/MultiDeriv
mex -g -c src/MultiDeriv/TermConstants.cpp -outdir build/MultiDeriv
mex -g -c src/primate.cpp -outdir build
mex -g -c src/Matlab/Matlab.cpp -outdir build/Matlab -I/usr/local/include/ -I/usr/local/include/eigen3/
mex -g -c src/Matlab/RankGaussian.cpp -outdir build/Matlab -I/usr/local/include/eigen3/
mex -g -c src/Matlab/SolveGaussianLinearEquation.cpp -outdir build/Matlab -I/usr/local/include/eigen3/
mex -g -c src/MultiRoundAttack.cpp -outdir build -I/usr/local/include -I/usr/local/include/eigen3/
mex -g -outdir dist build/MultiRoundAttack.o build/primate.o build/Block/Block.o build/Block/TermConstants.o build/MultiDeriv/Function.o build/MultiDeriv/Calculator.o build/MultiDeriv/TermConstants.o build/Matlab/Matlab.o build/Matlab/RankGaussian.o build/Matlab/SolveGaussianLinearEquation.o \
-leng -lmx -lboost_program_options -lboost_thread -lboost_system
#/Applications/Matlab/bin/glnxa64/libboost_program_options.so.1.56.0 -Wl,-rpath=/Applications/Matlab/sys/os/glnxa64 -L/Applications/Matlab/sys/os/glnxa64 -Wl,-rpath=/Applications/Matlab/bin/glnxa64 -L/Applications/Matlab/bin/glnxa64 -leng -lmx -lboost_thread -lboost_system
mex -g -o dist/testcase build/testcase.o build/primate.o build/Block/Block.o
rm build/*.o