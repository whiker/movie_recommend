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
#define ItemNum    964

int **item_feat;

void setupItemFeature()
{
	char filename[30], date[11];
	int  count(0);

	for (int file_i = 37; file_i <= 41; ++file_i)
	{
		sprintf(filename, "item-set/%03d", file_i);
		ifstream fin(filename);
		int item, nFeat, i;

		while (fin >> item >> date >> nFeat)
		{
			item_feat[count][0] = nFeat;
			for (int i = 1; i <= nFeat; ++i)
				fin >> item_feat[count][i];
			item_feat[count][8] = item;
			++count;
		}
		fin.close();
	}
}

void siftdown(int *ret, int *pos, int new_var, int new_pos, int heap_size)
{
	int p(0), q, left(1), right(2);

	while (left < heap_size)
	{
		if (ret[left] < new_var)
		{
			if (right < heap_size && ret[right] < ret[left])
				q = right;
			else
				q = left;
		}
		else if (right < heap_size && ret[right] < ret[left])
			q = right;
		else
			break;

		ret[p] = ret[q], pos[p] = pos[q];
		p = q;
		left = (p<<1) + 1;
		right = left + 1;
	}

	ret[p] = new_var, pos[p] = new_pos;
}

int topN(int *src, int len, int *ret, int *pos, int topn)
{
	if (topn <= 0) return 0;
	int fill, size, i, j;
	int var, new_pos, temp1, temp2;
	fill = 0;

	for (i = 0; i < len; ++i)
	{
		var = src[i];
		if (var <= 0) continue;

		if (fill < topn)  // setup min-heap
		{
			new_pos = i;
			for (j = 0; j < fill; ++j)
			{
				if (var < ret[j])
				{
					temp1  = ret[j], temp2   = pos[j];
					ret[j] = var,    pos[j]  = new_pos;
					var    = temp1,  new_pos = temp2;
				}
			}
			ret[fill] = var, pos[fill] = new_pos;
			++fill;
		}
		else if (var > ret[0])
			siftdown(ret, pos, var, i, topn);
	}

	if (fill == topn)
	{
		for (size = fill-1; size > 0; --size)
		{
			var       = ret[size], new_pos   = pos[size];
			ret[size] = ret[0],    pos[size] = pos[0];
			siftdown(ret, pos, var, new_pos, size);
		}
	}

	return fill;
}

void* recommend(void *arg)
{
	int task_id    = ((IdAtomic*)arg)->get();
	int start_file = task_id * nFile + 1;
	int end_file   = start_file + nFile - 1;
	if  (end_file > 175) end_file = 175;

	char filename[30];
	int  user, nItem, nRecom;
	int  user_feat[33];
	int  *recom_scor = new int[ItemNum];
	int  *recom_item = new int[ItemNum];
	int  *recom_rest = new int[ItemNum];
	int  *recom_posi = new int[ItemNum];

	for (int file_i = start_file; file_i <= end_file; ++file_i)
	{
		sprintf(filename, "user-feat-20/%03d", file_i);
		ifstream fin(filename);
		sprintf(filename, "ret-0/%03d", file_i);
		ofstream fout(filename);

		while (fin >> user >> nItem >> nRecom)
		{
			int topn(nRecom/2);
			int sum(0), nrecom(0), score;
			int i, j, k;

			for (i = 0; i < 33; ++i)
			{
				fin >> user_feat[i];
				sum += user_feat[i];
			}

			for (i = 0; i < ItemNum; ++i)
			{
				score = 0;
				for (j = item_feat[i][0]; j > 0; --j)
					score += user_feat[item_feat[i][j]];
				score -= item_feat[i][0] * sum / 33;
				if (score > 0)
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
		cout << file_i << endl;
	}

	delete[] recom_scor, recom_item;
	delete[] recom_rest, recom_posi;
	return NULL;
}

int main(int argc, char **argv)
{
	Mat<int> mat(ItemNum, 9);
	item_feat = mat.data;
	setupItemFeature();

	IdAtomic task_id(0);
	pthread_t tasks[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, recommend, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);

	return 0;
}