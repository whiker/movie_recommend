#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <pthread.h>
#include "IdAutomic.h"
using namespace std;

#define FileNum 20

void* map(void *arg)
{
	IdAutomic *taskid = (IdAutomic*) arg;
	int task_id = taskid->get();

	int start = FileNum * task_id + 1;
	int end = start + FileNum - 1;
	if (end > 158) end = 158;

	char inp_name[30], out_name[30];
	ushort user, item, nItem;
	vector<ushort> item_user[7100];

	for (int file_i = start; file_i <= end; ++file_i)
	{
		sprintf(inp_name, "train/%03d", file_i);
		ifstream fin(inp_name);
		while (fin >> user >> nItem)
		{
			for (int i = 0; i < nItem; ++i)
			{
				fin >> item;
				item_user[item].push_back(user);
			}
		}
		fin.close();
		cout << file_i << endl;
	}

	vector<ushort>::iterator it, it_end;
	int size;

	for (int out_i = 1; out_i <= 100; ++out_i)
	{
		sprintf(out_name, "map-ret/map-%03d/%03d", task_id, out_i);
		ofstream fout(out_name);
		for (int j = out_i; j < 7100; j+=100)
		{
			size = item_user[j].size();
			if (size > 0)
			{
				it = item_user[j].begin();
				it_end = item_user[j].end();
				fout << j << ' ' << size << '\n';
				fout << *it;
				for (++it; it != it_end; ++it)
					fout << ' ' << *it;
				fout << '\n';
			}
		}
		fout.close();
	}

	return NULL;
}

int main()
{
	int task_ids[2] = { 0, 4 };
	
	for (int i = 0; i < 1; ++i)
	{
		IdAutomic taskid(task_ids[i]);
		const int nThread = 4;
		pthread_t tasks[nThread];

		for (int i = 0; i < nThread; ++i)
			if ( pthread_create(&tasks[i], 0, map, &taskid) )
				cout << "error create thread : " << i << endl;

		for (int i = 0; i < nThread; ++i)
			if ( pthread_join(tasks[i], NULL) )
				cout << "error join thread : " << i << endl;
	}

	return 0;
}