#ifndef CLASS_Mat
#define CLASS_Mat

#include <string.h>

template <typename T>
class Mat
{
public:
	// row-num    : m
	// column-num : n
	int m = 0, n = 0;
	T **data = 0;

public:
	Mat(int m, int n)
	{
		if (m <= 0 || n <= 0)
			return;
		data = new T*[m];
		for (int i = 0; i < m; ++i)
		{
			data[i] = new T[n];
			memset(data[i], 0, n*sizeof(T));
		}
		this->m = m;
		this->n = n;
	}

	~Mat()
	{
		if (!data) return;
		for (int i = 0; i < m; ++i)
			delete[] data[i];
		delete[] data;
	}
};

#endif