rm -rf ret;
mkdir ret;
g++ -std=c++11 main.cpp topn.cpp -lpthread;
./a.out;
g++ fs.cpp;
./a.out;