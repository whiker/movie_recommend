#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "IdAtomic.h"
using namespace std;

#define MaxItemNum 160000
#define nThread    16
#define nFile      11

char *SrcSet, *DstSet;

void getItemSet()
{
	int  item;
	char date[15], type[100];
	char inp_name[30];

	memset(SrcSet, 0, MaxItemNum);
	memset(DstSet, 0, MaxItemNum);

	for (int i = 1; i <= 41; ++i)
	{
		sprintf(inp_name, "item/%03d", i);
		ifstream fin(inp_name);
		while (fin >> item >> date >> type)
		{
			if (i < 37)
				SrcSet[item] = 1;
			else
				DstSet[item] = 1;
		}
		fin.close();
	}
}

void* filte(void *arg)
{
	int task_id    = ((IdAtomic*)arg)->get();
	int start_file = task_id * nFile + 1;
	int end_file   = start_file + nFile - 1;
	if (end_file > 175) end_file = 175;

	int file_i = start_file;
	char filename[30];
	int user, nItem;

	int *src_set = new int[7200];
	int *dst_set = new int[1000];

	for (; file_i <= end_file; ++file_i)
	{
		sprintf(filename, "record/%03d", file_i);
		ifstream fin(filename);
		sprintf(filename, "src/%03d", file_i);
		ofstream fout1(filename);
		sprintf(filename, "dst/%03d", file_i);
		ofstream fout2(filename);

		while (fin >> user >> nItem)
		{
			int p1(0), p2(0), item, i;

			for (i = 0; i < nItem; ++i)
			{
				fin >> item;
				if (SrcSet[item])
					src_set[p1++] = item;
				else if (DstSet[item])
					dst_set[p2++] = item;
			}

			if (p1 > 0)
			{
				fout1 << user << ' ' << p1 << '\n';
				fout1 << src_set[0];
				for (i = 1; i < p1; ++i)
					fout1 << ' ' << src_set[i];
				fout1 << '\n';
			}

			if (p2 > 0)
			{
				fout2 << user << ' ' << p2 << '\n';
				fout2 << dst_set[0];
				for (i = 1; i < p2; ++i)
					fout2 << ' ' << dst_set[i];
				fout2 << '\n';
			}
		}

		fin.close();
		fout1.close();
		fout2.close();
		cout << file_i << endl;
	}

	delete[] src_set;
	delete[] dst_set;
	return NULL;
}

int main(int argc, char **argv)
{
	SrcSet = new char[MaxItemNum];
	DstSet = new char[MaxItemNum];
	getItemSet();

	IdAtomic task_id(0);
	pthread_t tasks[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, filte, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);

	delete[] SrcSet;
	delete[] DstSet;
	return 0;
}