#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "IdAtomic.h"
#include "Mat.h"
#include "util.h"
using namespace std;

#define nThread    10
#define nUser      5760
#define UserNum    57581

double  *feat_norm;
int    **user_feat;
int    **user_like;

void getUserFeat()
{
	char filename[30];
	int  user, nItem, nRecom;
	int  count(0), i, j;

	for (int file_i = 1; file_i <= 175; ++file_i)
	{
		sprintf(filename, "user-feat-10/%03d", file_i);
		ifstream fin(filename);

		while (fin >> user_feat[count][33] >> nItem >> nRecom)
		{
			double norm = 0.0;
			for (i = 0; i < 33; ++i)
			{
				fin >> j;
				user_feat[count][i] = j;
				norm += j*j;
			}
			feat_norm[count] = sqrt(norm);
			++count;
		}
		fin.close();
	}
}

void* like(void *arg)
{
	int task  = ((IdAtomic*)arg)->get();
	int start = task * nUser;
	int end   = start + nUser - 1;
	if  (end >= UserNum)
		end = UserNum - 1;

	const int MaxRandomNum = UserNum/5;
	const int MaxDetectNum = 40;

	char *flag = new char[UserNum];
	int  sum_nLike(0), sum_calc(0), nLike;
	int  i, j, k, like_user;

	for (int user = start; user <= end; ++user)
	{
		memset(flag, 0, UserNum);
		nLike = 0;

		for (i = 0; nLike<10 && i<MaxRandomNum; ++i)
		{
			for (j = 0; j < MaxDetectNum; ++j)
			{
				like_user = rand() % UserNum;
				if (flag[like_user] == 0)
					break;
				for (k = 0; k < 10; ++k)
				{
					like_user = (like_user+17) % UserNum;
					if (flag[like_user] == 0)
						break;
				}
				if (k < 5)
					break;
			}
			if (j == MaxDetectNum)
				continue;

			double sim = 0;
			for (j = 0; j < 33; ++j)
				sim += user_feat[user][j] * user_feat[like_user][j];
			sim = sim / feat_norm[user] / feat_norm[like_user];
			if (0.85 < sim && sim < 0.95)
				user_like[user][++nLike] = like_user;
			++sum_calc;
		}

		user_like[user][0] = nLike;
		sum_nLike += nLike;
	}

	cout << sum_nLike/nUser << '\t' << sum_calc << endl;

	delete[] flag;
	return NULL;
}

void saveUserLike()
{
	ofstream fout("user-like");
	for (int i = 0; i < UserNum; ++i)
	{
		fout << user_feat[i][33] << ' '
			 << user_like[i][0];
		for(int j = user_like[i][0]; j > 0; --j)
			fout << ' ' << user_like[i][j];
		fout << '\n';
	}
	fout.close();
}

int main(int argc, char **argv)
{
	Mat<int> mat1(UserNum, 34);
	user_feat = mat1.data;
	feat_norm = new double[UserNum];
	getUserFeat();

	Mat<int> mat2(UserNum, 11);
	user_like = mat2.data;

	IdAtomic task_id(0);
	pthread_t tasks[nThread];
	for (int i = 0; i < nThread; ++i)
		pthread_create(&tasks[i], NULL, like, &task_id);
	for (int i = 0; i < nThread; ++i)
		pthread_join(tasks[i], NULL);

	saveUserLike();

	delete[] feat_norm;
	return 0;
}