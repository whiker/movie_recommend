#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "IdAtomic.h"
using namespace std;

#define nThread    16
#define nFile      11
#define nRecom     10

void* filte(void *arg)
{
	int task_id    = ((IdAtomic*)arg)->get();
	int start_file = task_id * nFile + 1;
	int end_file   = start_file + nFile - 1;
	if (end_file > 175) end_file = 175;

	char filename[30];
	int  user, nItem, file_i(start_file);
	int  *set = new int[7200];

	for (; file_i <= end_file; ++file_i)
	{
		sprintf(filename, "src/%03d", file_i);
		ifstream fin(filename);
		sprintf(filename, "src-10/%03d", file_i);
		ofstream fout(filename);

		while (fin >> user >> nItem)
		{
			int item, i, j(0), n(0);

			for (i = 0; i < nItem; ++i)
			{
				fin >> item;
				set[j++] = item;
				if (item > 140000)
					++n;
			}

			if (n > nRecom)  // src-10
			{
				fout << user << ' ' << nItem << ' ' << n << '\n';
				fout << set[0];
				for (i = 1; i < nItem; ++i)
					fout << ' ' << set[i];
				fout << '\n';
			}
		}

		fin.close();
		fout.close();
		cout << file_i << endl;
	}

	delete[] set;
	return NULL;
}

int main(int argc, char **argv)
{
	IdAtomic task_id(0);
	pthread_t tasks[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, filte, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);

	return 0;
}