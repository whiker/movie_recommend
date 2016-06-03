#include <iostream>
#include <fstream>
#include <map>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "IdAtomic.h"
#include "Mat.h"
using namespace std;

#define TopN	50
#define nThread	16
#define nFile	10

ushort topN(ushort *array, ushort length,
			ushort *ret,   ushort *pos,   ushort topn)
{
	if (topn <= 0) return 0;
	ushort fill, left, right, size, i, j;
	ushort var, p, q, temp1, temp2;
	fill = 0;

	for (i = 0; i < length; ++i)
	{
		var = array[i];
		if (var == 0) continue;

		if (fill < topn)
		{
			p = i;
			for (j = 0; j < fill; ++j)
			{
				if (var < ret[j])
				{
					temp1 = ret[j];
					temp2 = pos[j];
					ret[j] = var;
					pos[j] = p;
					var = temp1;
					p = temp2;
				}
			}
			ret[fill] = var;
			pos[fill] = p;
			++fill;
		}
		else if (var > ret[0])
		{
			p = 0;
			left = p*2 + 1;

			while (left < topn)
			{
				right = left + 1;
				if (ret[left]<var)
				{
					if (right<topn && ret[right]<ret[left])
						q = right;
					else
						q = left;
				}
				else if (right<topn && ret[right]<ret[left])
						q = right;
				else
					break;

				ret[p] = ret[q];
				pos[p] = pos[q];
				p = q;
				left = p*2 + 1;
			}

			ret[p] = var;
			pos[p] = i;
		}
	}

	if (fill == topn)
	{
		for (size = fill-1; size > 0; --size)
		{
			var = ret[size];
			i   = pos[size];
			ret[size] = ret[0];
			pos[size] = pos[0];
			p = 0;
			left = p*2 + 1;

			while (left < size)
			{
				right = left + 1;
				if (ret[left]<var)
				{
					if (right<size && ret[right]<ret[left])
						q = right;
					else
						q = left;
				}
				else if (right<size && ret[right]<ret[left])
						q = right;
				else
					break;

				ret[p] = ret[q];
				pos[p] = pos[q];
				p = q;
				left = p*2 + 1;
			}

			ret[p] = var;
			pos[p] = i;
		}
	}

	return fill;
}

void* task(void *arg)
{
	int task_id 	= ((IdAtomic*)arg)->get();
	int start_file	= task_id * nFile + 1;
	int end_file 	= start_file + nFile - 1;
	if (end_file>158) end_file=158;

	char filename[30];
	ushort user, nUser, sim_user, item, nItem;

	Mat<ushort> mat(401, 1500);
	ushort **item_set = mat.data;
	std::map<ushort, ushort> user_id;
	
	ushort *item_count = new ushort[7100];
	ushort *topn_ret = new ushort[TopN];
	ushort *topn_pos = new ushort[TopN];
	int i, j, n;

	for (int file_i = start_file; file_i <= end_file; ++file_i)
	{
		user_id.clear();
		sprintf(filename, "train/%03d", file_i);
		ifstream fin1(filename);
		for (n = 1; fin1 >> user >> nItem; ++n)
		{
			user_id[user] = n;
			item_set[n][0] = nItem;
			for (i = 1; i <= nItem; ++i)
				fin1 >> item_set[n][i];
		}
		fin1.close();

		sprintf(filename, "sim/%03d", file_i);
		ifstream fin2(filename);
		sprintf(filename, "ret/%03d", file_i);
		ofstream fout(filename);
		while (fin2 >> user >> nUser)
		{
			memset(item_count, 0, 7100*sizeof(ushort));
			for (i = 0; i < nUser; ++i)
			{
				fin2 >> sim_user;
				n = user_id[sim_user];
				if (n > 0)
					for (j = item_set[n][0]; j > 0; --j)
						++item_count[item_set[n][j]];
			}
			n = user_id[user];
			for (j = item_set[n][0]; j > 0; --j)
				item_count[item_set[n][j]] = 0;

			n = topN(item_count, 7100, topn_ret, topn_pos, 50);
			if (n > 0)
			{
				fout << user << ' ' << n << '\n';
				if (n < 50)
				{
					fout << topn_pos[--n];
					for (--n; n >= 0; --n)
						fout << ' ' << topn_pos[n];
					fout << '\n';
				}
				else
				{
					fout << topn_pos[0];
					for (j = 1; j < n; ++j)
						fout << ' ' << topn_pos[j];
					fout << '\n';
				}
			}
		}
		fin2.close();
		fout.close();
		cout << file_i << endl;
	}

	delete[] item_count;
	delete[] topn_ret;
	delete[] topn_pos;
	return NULL;
}

int main()
{
	IdAtomic map_id(0);
	pthread_t map[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&map[i], NULL, task, &map_id);

	for (int i = 0; i < nThread; ++i)
		pthread_join(map[i], NULL);

	return 0;
}