#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "IdAutomic.h"
using namespace std;

const int nThread = 8;

ushort user_id[400];
vector<ushort> item_user_index[7100];
ushort sim_mat[nThread][400][65536];

IdAutomic TaskId(0);
pthread_t Tasks[nThread];

void* task(void *arg)
{
	int task_id = TaskId.get();
	char inp_name[30];
	ushort user, item, nUser;
	int size, i, j, k, task_id_1;

	ushort right_user[62886];
	vector<ushort>::iterator it, it_end;

	// map
	for (i = 1; i <= 100; ++i)
	{
		sprintf(inp_name, "map-ret/map-%03d/%03d", task_id, i);
		ifstream fin(inp_name);
		while (fin >> item >> nUser)
		{
			for (j = 0; j < nUser; ++j)
				fin >> right_user[j];
			size = item_user_index[item].size();
			if (size > 0)
			{
				it = item_user_index[item].begin();
				it_end = item_user_index[item].end();
				for (; it != it_end; ++it)
				{
					k = *it;
					for (j = 0; j < nUser; ++j)
						++sim_mat[task_id][k][right_user[j]];
				}
			}
		}
		fin.close();
	}
	sprintf(inp_name, "map     : %d\n", task_id);
	cout << inp_name;

	// merge-0
	if (task_id % 2 == 0)
	{
		task_id_1 = task_id + 1;
		pthread_join(Tasks[task_id_1], NULL);
		for (i = 0; i < 400; ++i)
			for (j = 0; j < 62886; ++j)
				sim_mat[task_id][i][j] += sim_mat[task_id_1][i][j];
		sprintf(inp_name, "merge-0 : %d\n", task_id);
		cout << inp_name;
	}

	// merge-1
	if (task_id % 4 == 0)
	{
		task_id_1 = task_id + 2;
		pthread_join(Tasks[task_id_1], NULL);
		for (i = 0; i < 400; ++i)
			for (j = 0; j < 62886; ++j)
				sim_mat[task_id][i][j] += sim_mat[task_id_1][i][j];
		sprintf(inp_name, "merge-1 : %d\n", task_id);
		cout << inp_name;
	}

	// merge-2
	if (task_id == 0)
	{
		pthread_join(Tasks[4], NULL);
		for (i = 0; i < 400; ++i)
			for (j = 0; j < 62886; ++j)
				sim_mat[0][i][j] += sim_mat[4][i][j];
		sprintf(inp_name, "merge-2 : %d\n", task_id);
		cout << inp_name;
	}

	return NULL;
}

void setupItemUserIndex(int train_id)
{
	ushort user, item, nItem;
	char inp_name[30];
	sprintf(inp_name, "train/%03d", train_id);
	for (int i = 0; i < 7100; ++i)
		item_user_index[i].clear();

	ifstream fin(inp_name);
	for (int i = 0; fin >> user >> nItem; ++i)
	{
		user_id[i] = user;
		for (int j = 0; j < nItem; ++j)
		{
			fin >> item;
			item_user_index[item].push_back(i);
		}
	}
	fin.close();
}

void mapReduce()
{
	for (int i = 0; i < nThread; ++i)
		if ( pthread_create(&Tasks[i], 0, task, NULL) )
			cout << "error create thread : " << i << endl;
	pthread_join(Tasks[0], NULL);
}

void output(int train_id)
{
	ushort user, nItem, file_id;
	ushort user_nitem[62886];

	ifstream fin("query.txt");
	while (fin >> user >> nItem >> file_id)
		user_nitem[user] = nItem * 0.9;
	fin.close();

	char out_name[30];
	sprintf(out_name, "sim/%03d", train_id);
	int i, j, n;

	ofstream fout(out_name);
	for (i = 0; i < 400; ++i)
	{
		nItem = user_nitem[user_id[i]];
		n = 0;
		sim_mat[0][i][user_id[i]] = 0;
		for (j = 1; j < 62886; ++j)
		{
			if (sim_mat[0][i][j] < nItem || sim_mat[0][i][j] < user_nitem[j])
				sim_mat[0][i][j] = 0;
			else
				++n;
		}
		if (n > 0)
		{
			fout << user_id[i] << ' ' << n << '\n';
			for (j = 1; j < 62886; ++j)
				if (sim_mat[0][i][j] > 0)
					fout << j << ' ';
			fout << '\n';
		}
	}
	fout.close();
}

void zeroSimMat()
{
	for (int i = 0; i < nThread; ++i)
		memset(sim_mat[i], 0, 800*65536);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		cout << "main arg error\n";
		return 0;
	}

	int train_id = atoi(argv[1]);
	zeroSimMat();

	setupItemUserIndex(train_id);
	mapReduce();
	output(train_id);

	return 0;
}