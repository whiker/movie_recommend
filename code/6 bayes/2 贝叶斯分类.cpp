#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "IdAtomic.h"
#include "Mat.h"
#include "util.h"
using namespace std;

#define nThread    16
#define nFile      11
#define ItemNum    964

int **item_feat;


void getItemPattern()
{
	char filename[30], date[11];
	long pattern, one(1);
	int  count(0);

	map<long, int> pattern_freq;
	long *item_pattern = new long[ItemNum];

	for (int file_i = 1; file_i <= 41; ++file_i)
	{
		sprintf(filename, "item-set/%03d", file_i);
		ifstream fin(filename);
		int item, nFeat, i, j;

		while (fin >> item >> date >> nFeat)
		{
			pattern = 0;
			for (i = 1; i <= nFeat; ++i)
			{
				fin >> j;
				pattern |= one<<j;
				if (file_i >= 37)
					item_feat[count][i] = j;
			}
			++pattern_freq[pattern];

			if (file_i >= 37)
			{
				item_feat[count][0] = nFeat;
				item_feat[count][8] = item;
				item_pattern[count] = pattern;
				++count;
			}
		}
		fin.close();
	}

	for (int i = 0; i < ItemNum; ++i)
	{
		item_feat[i][9] = pattern_freq[ item_pattern[i] ];
	}
	delete[] item_pattern;
}


void* recommend(void *arg)
{
	int task_id    = ((IdAtomic*)arg)->get();
	int start_file = task_id * nFile + 1;
	int end_file   = start_file + nFile - 1;
	if  (end_file > 175) end_file = 175;

	char filename[30];
	int  user, nItem, nRecom, user_feat[33];

	double *recom_scor = new double[ItemNum];
	double *recom_rest = new double[ItemNum];
	int    *recom_item = new int[ItemNum];
	int    *recom_posi = new int[ItemNum];

	for (int file_i = start_file; file_i <= end_file; ++file_i)
	{
		sprintf(filename, "user-feat-10/%03d", file_i);
		ifstream fin(filename);
		sprintf(filename, "ret/%03d", file_i);
		ofstream fout(filename);

		while (fin >> user >> nItem >> nRecom)
		{
			int i, j;
			int topn, nrecom(0);
			double score, k;

			if (nItem < 850)
				topn = nRecom*0.11;
			else
				topn = nRecom*1.40;

			for (i = 0; i < 33; ++i)
				fin >> user_feat[i];

			for (i = 0; i < ItemNum; ++i)
			{
				score = 0.0;
				for (j = item_feat[i][0]; j > 0; --j)
				{
					k = user_feat[ item_feat[i][j] ];
					if (k == 0) k = 0.01;
					score += log(k);
				}
				score -= log(item_feat[i][9]);

				if (9.5<score && score<9.82)
				{
					recom_scor[nrecom] = score;
					recom_item[nrecom] = item_feat[i][8];
					++nrecom;
				}
			}

			nrecom = topN(recom_scor, nrecom,
						  recom_rest, recom_posi, topn);
			if (nrecom <= 0) continue;
			fout << user << ' ' << nrecom << '\n';
			fout << recom_item[recom_posi[0]];
			for (i = 1; i < nrecom; ++i)
				fout << ' ' << recom_item[recom_posi[i]];
			fout << '\n';
		}

		fin.close();
		fout.close();
	}

	delete[] recom_scor, recom_item;
	delete[] recom_rest, recom_posi;
	return NULL;
}


int main(int argc, char **argv)
{
	Mat<int> mat(ItemNum, 10);
	item_feat = mat.data;
	getItemPattern();

	IdAtomic task_id(0);
	pthread_t tasks[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, recommend, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);
	cout << "ret" << endl;

	return 0;
}