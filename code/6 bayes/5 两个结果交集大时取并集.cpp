#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
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

void* combine(void *arg)
{
	int task_id    = ((IdAtomic*)arg)->get();
	int start_file = task_id * nFile + 1;
	int end_file   = start_file + nFile - 1;
	if  (end_file > 175) end_file = 175;

	char filename[30];
	int  user0, nRecom0, user1, nRecom1;
	int  i, j;

	set<int> set0, set1, set2;
	set<int>::iterator set_it;

	for (int file_i = start_file; file_i <= end_file; ++file_i)
	{
		sprintf(filename, "ret-0/%03d", file_i);
		ifstream fin0(filename);
		sprintf(filename, "ret-1/%03d", file_i);
		ifstream fin1(filename);
		sprintf(filename, "ret/%03d", file_i);
		ofstream fout(filename);

		user1 = -1;

		while (fin0 >> user0 >> nRecom0)
		{
			if (user1 == -1)
			{
				while (fin1 >> user1 >> nRecom1)
				{
					if (user1 < user0)
					{
						fout << user1 << ' ' << nRecom1 << '\n';
						fin1 >> j; fout << j;
						for (i = 1; i < nRecom1; ++i)
						{
							fin1 >> j;
							fout << ' ' << j;
						}
						fout << '\n';
						user1 = -1;
					}
					else if (user1 >= user0)
						break;
				}
			}

			if (user1 == user0)
			{
				set0.clear();
				set1.clear();
				for (i = 0; i < nRecom0; ++i)
				{
					fin0 >> j;
					set0.insert(j);
				}
				for (i = 0; i < nRecom1; ++i)
				{
					fin1 >> j;
					set1.insert(j);
				}

				set2.clear();
				set_intersection(set0.begin(), set0.end(),
								set1.begin(), set1.end(),
								inserter(set2, set2.begin())
				);
				if (set2.size() < 0.97*set1.size())
				{
					fout << user0 << ' ' << set0.size() << '\n';
					set_it = set0.begin();
					fout << *set_it;
					for (++set_it; set_it != set0.end(); ++set_it)
						fout << ' ' << *set_it;
					fout << '\n';
				}
				else
				{
					set2.clear();
					set_union(set0.begin(), set0.end(),
							  set1.begin(), set1.end(),
							  inserter(set2, set2.begin())
					);

					fout << user0 << ' ' << set2.size() << '\n';
					set_it = set2.begin();
					fout << *set_it;
					for (++set_it; set_it != set2.end(); ++set_it)
						fout << ' ' << *set_it;
					fout << '\n';
				}
				user1 = -1;
			}
			else
			{
				fout << user0 << ' ' << nRecom0 << '\n';
				fin0 >> j; fout << j;
				for (i = 1; i < nRecom0; ++i)
				{
					fin0 >> j;
					fout << ' ' << j;
				}
				fout << '\n';
			}
		}

		if (user1 != -1)
		{
			fout << user1 << ' ' << nRecom1 << '\n';
			fin1 >> j; fout << j;
			for (i = 1; i < nRecom1; ++i)
			{
				fin1 >> j;
				fout << ' ' << j;
			}
			fout << '\n';
		}

		while (fin1 >> user1 >> nRecom1)
		{
			fout << user1 << ' ' << nRecom1 << '\n';
			fin1 >> j; fout << j;
			for (i = 1; i < nRecom1; ++i)
			{
				fin1 >> j;
				fout << ' ' << j;
			}
			fout << '\n';
		}

		fin0.close();
		fin1.close();
		fout.close();
	}

	return NULL;
}


int main(int argc, char **argv)
{
	IdAtomic task_id(0);
	pthread_t tasks[nThread];

	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, combine, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);
	cout << "ret" << endl;

	return 0;
}