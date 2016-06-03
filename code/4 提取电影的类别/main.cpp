#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
using namespace std;

const char Mark_1[] = "发布于";
const char Mark_2[] = "基本信息";
const char Mark_3[] = "类型";
const char Mark_6[] = "<br />";
const char Mark_7[] = "豆瓣信息";
const char Mark_8[] = "电影类型：";
const char Mark_9[] = "产　　地：";

char itemdate[11];
char itemname[100];
char itemtype[100];
char national[100];

char temp_1[100], temp_2[100];

int parse(char *buffer, int size)
{
	char *p, *q, *r;
	int  len, i, j;

	p = strstr(buffer, Mark_1);
	if (p == NULL) return -1;
	p += sizeof(Mark_1) + 12;
	strncpy(itemdate, p, 10);
	itemdate[10] = 0;

	p = strstr(p, Mark_2);
	if (p == NULL) return -1;
	p += sizeof(Mark_2);
	p = strstr(p, Mark_3);
	if (p == NULL) return -1;
	p += sizeof(Mark_3) + 10;
	strncpy(temp_1, p, 6);
	temp_1[6] = 0;
	if (strcmp(temp_1, "电影") != 0)
		return 1;

	p = strstr(p, Mark_7);
	if (p == NULL) return 2;
	p += sizeof(Mark_7);

	p = strstr(p, Mark_8);
	if (p == NULL) return -1;
	p += sizeof(Mark_8) - 1;
	for (i = 0; *p != '<'; ++p, ++i)
		itemtype[i] = *p;
	itemtype[i] = 0;

	p = strstr(p, Mark_9);
	if (p == NULL) return -1;
	p += sizeof(Mark_9) - 1;
	for (i = 0; *p != '<'; ++p, ++i)
		national[i] = *p;
	national[i] = 0;

	return 0;
}

void task()
{
	const int BufSize = 1024 * 1024;
	char *buffer = new char[BufSize];

	char filename[30];
	int  fd, nRead, ret;
	int  count=52, out_id=26;

	ofstream fout, fout_error("error"), fout_error_2("error_2");
	sprintf(filename, "item/%03d", out_id);
	fout.open(filename, ofstream::out|ofstream::app);

	for (int item_id = 136757; item_id <= 145058; ++item_id)
	{
		sprintf(filename, "item-1/%06d.html", item_id);
		if (access(filename, F_OK) != 0)
			continue;

		fd = open(filename, O_RDONLY);
		nRead = read(fd, buffer, BufSize);
		close(fd);

		ret = parse(buffer, nRead);
		if (ret == -1)
			fout_error << item_id << '\n';
		else if (ret == 2)
			fout_error_2 << item_id << '\n';
		else if (ret == 0)
		{
			if (++count > 200)
			{
				count = 1;
				fout.close();
				++out_id;
				sprintf(filename, "item/%03d", out_id);
				fout.open(filename, ofstream::out);
			}
			if (strlen(itemtype) > 0)
				fout<< item_id  << '\t'
				 	<< itemdate << '\t'
				 	<< itemtype << '\n';
			// national
		}

		cout << item_id << endl;
	}

	fout.close();
	fout_error.close();
	fout_error_2.close();
	delete[] buffer;
}

int main()
{
	task();
	return 0;
}