#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <stdio.h>
using namespace std;

void f1Score()
{
	char filename[30];
	int  user_ret, user_dst;
	int  nRecom, nReal;
	int  sum_same(0), sum_recom(0), sum_real(1339765);

	set<int> set1, set2, set3;
	set<int>::iterator set_it;

	for (int file_i = 1; file_i <= 175; ++file_i)
	{
		sprintf(filename, "ret/%03d", file_i);
		ifstream fin_ret(filename);
		sprintf(filename, "dst/%03d", file_i);
		ifstream fin_dst(filename);
		int i, j;

		while (fin_ret >> user_ret >> nRecom)
		{
			sum_recom += nRecom;
			set1.clear();
			for (i = 0; i < nRecom; ++i)
			{
				fin_ret >> j;
				set1.insert(j);
			}

			while (fin_dst >> user_dst >> nReal)
			{
				if (user_dst == user_ret)
					break;
				for (i = 0; i < nReal; ++i)
					fin_dst >> j;
			}
			set2.clear();
			for (i = 0; i < nReal; ++i)
			{
				fin_dst >> j;
				set2.insert(j);
			}

			set3.clear();
			set_it = set3.begin();
			set_intersection(set1.begin(), set1.end(),
				set2.begin(), set2.end(), inserter(set3,set_it));
			sum_same += set3.size();
		}

		fin_ret.close();
		fin_dst.close();
	}

	cout << sum_same  << ' '
		 << sum_recom << ' '
		 << sum_real  << endl;

	double p1 = (double)sum_same / sum_recom;
	double p2 = (double)sum_same / sum_real;
	double p3 = 2*p1*p2 / (p1+p2);
	cout << p1 << ' ' << p2 << ' ' << p3 << endl;
}

int main(int argc, char **argv)
{
	f1Score();
	return 0;
}