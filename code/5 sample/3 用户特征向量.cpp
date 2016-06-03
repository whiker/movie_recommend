#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "IdAtomic.h"
#include "Mat.h"
using namespace std;

#define nThread    16
#define nFile      11

int **item_feat;

void setupItemFeature()
{
	int  file_i, item, nFeat, max(0);
	char filename[30], date[11];

	for (file_i = 1; file_i <= 41; ++file_i)
	{
		sprintf(filename, "item-set/%03d", file_i);
		ifstream fin(filename);

		while (fin >> item >> date >> nFeat)
		{
			item_feat[item][0] = nFeat;
			for (int i = 1; i <= nFeat; ++i)
				fin >> item_feat[item][i];
		}
		fin.close();
	}
}

void* userFeat(void *arg)
{
	int task_id    = ((IdAtomic*)arg)->get();
	int start_file = task_id * nFile + 1;
	int end_file   = start_file + nFile - 1;
	if  (end_file > 175) end_file = 175;

	char filename[30];
	int  user, nItem, nRecom, file_i(start_file);
	int  user_feat[34];

	for (; file_i <= end_file; ++file_i)
	{
		sprintf(filename, "src-10/%03d", file_i);
		ifstream fin(filename);
		sprintf(filename, "user-feat-10/%03d", file_i);
		ofstream fout(filename);
		int item, i, j;

		while (fin >> user >> nItem >> nRecom)
		{
			memset(user_feat, 0, 34*sizeof(int));
			for (i = 0; i < nItem; ++i)
			{
				fin >> item;
				for (j = item_feat[item][0]; j > 0; --j)
					++user_feat[item_feat[item][j]];
			}

			fout << user << ' ' << nItem << ' ' << nRecom;
			for (i = 1; i <= 33; ++i)
				fout << ' ' << user_feat[i];
			fout << '\n';
		}

		fin.close();
		fout.close();
		cout << file_i << endl;
	}

	return NULL;
}

int main(int argc, char **argv)
{
	Mat<int> mat(160000, 8);
	item_feat = mat.data;
	setupItemFeature();

	IdAtomic task_id(0);
	pthread_t tasks[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, userFeat, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);

	return 0;
}