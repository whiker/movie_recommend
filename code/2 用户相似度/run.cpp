#include <iostream>
#include <stdio.h>
#include <fstream>
using namespace std;

int main(int argc, char **argv)
{
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);

	ofstream fout("run.sh");
	for (int i = a; i <= b; ++i)
		fout << "./c.out " << i << endl;
	fout.close();
}