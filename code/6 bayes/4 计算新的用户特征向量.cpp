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

int **user_feat;
int **user_like;
int  *user_nitem;
int  *user_nrecom;
int  *break_point;

void getUserFeat()
{
	char filename[30];
	int  user, nItem, nRecom;
	int  count(0);

	for (int file_i = 1; file_i <= 175; ++file_i)
	{
		sprintf(filename, "user-feat-10/%03d", file_i);
		ifstream fin(filename);
		int i;

		while (fin >> user_feat[count][33]
				   >> user_nitem[count]
				   >> user_nrecom[count])
		{
			for (i = 0; i < 33; ++i)
				fin >> user_feat[count][i];
			++count;
		}

		break_point[file_i] = count;
		fin.close();
	}
	break_point[0] = 0;
}

void getUserLike()
{
	int user, nLike, user_id(0), i;
	ifstream fin("user-like");
	while (fin >> user >> nLike)
	{
		user_like[user_id][0] = nLike;
		for (i = 1; i <= nLike; ++i)
			fin >> user_like[user_id][i];
		++user_id;
	}
	fin.close();
}

void newUserFeat()
{
	char filename[30];
	int  nLike, feat[33];
	int  user, like, end;
	int  i, j, k;

	for (int file_i = 1; file_i <= 175; ++file_i)
	{
		sprintf(filename, "user-feat-10-new/%03d", file_i);
		ofstream fout(filename);

		user = break_point[file_i-1];
		end  = break_point[file_i];

		for (; user < end; ++user)
		{
			nLike = user_like[user][0];
			for (i = 1; i <= nLike; ++i)
			{
				like = user_like[user][i];
				for (j = 0; j < 33; ++j)
					user_feat[user][j] += user_feat[like][j];
			}

			fout << user_feat[user][33] << ' '
				 << user_nitem[user]    << ' '
				 << user_nrecom[user];

			nLike += 1;
			for (j = 0; j < 33; ++j)
			{
				k = user_feat[user][j] / nLike;
				fout << ' ' << k;
			}
			fout << '\n';
		}
		fout.close();
	}
}

int main(int argc, char **argv)
{
	Mat<int> mat1(UserNum, 34);
	user_feat = mat1.data;

	Mat<int> mat2(UserNum, 11);
	user_like = mat2.data;

	user_nitem  = new int[UserNum];
	user_nrecom = new int[UserNum];
	break_point = new int[175];
	
	getUserFeat();
	getUserLike();
	newUserFeat();

	delete[] user_nitem;
	delete[] user_nrecom;
	delete[] break_point;
	return 0;
}