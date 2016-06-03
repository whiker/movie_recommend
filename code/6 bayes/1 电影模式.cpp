#include <iostream>
#include <fstream>
#include <map>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "IdAtomic.h"
#include "Mat.h"
using namespace std;

#define nThread    16
#define nFile      11
#define ItemNum    964


void getItemPattern()
{
	map<long, int> pattern_freq;
	char filename[30], date[11];
	long pattern, one(1);

	for (int file_i = 1; file_i <= 41; ++file_i)
	{
		sprintf(filename, "item-set/%03d", file_i);
		ifstream fin(filename);
		int item, nFeat, i, j;

		while (fin >> item >> date >> nFeat)
		{
			pattern = 0;
			for (i = 0; i < nFeat; ++i)
			{
				fin >> j;
				pattern |= one<<j;
			}
			++pattern_freq[pattern];
		}
		fin.close();
	}

	ofstream fout("pattern");
	int check = 0;
	for (auto it = pattern_freq.begin(); it != pattern_freq.end(); ++it)
	{
		fout << it->first << '\t' << it->second << endl;
		check += it->second;
	}
	fout.close();
	cout << check << endl;
}

int main(int argc, char **argv)
{
	getItemPattern();
	return 0;
}