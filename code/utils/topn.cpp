#include "util.h"

// TODO 优化
int topN(double *src, int len, double *ret, int *pos, int topn)
{
	if (topn <= 0) return 0;
	int fill, size, i, j;
	double var, temp1;
	int new_pos, temp2;
	fill = 0;

	for (i = 0; i < len; ++i)
	{
		var = src[i];

		if (fill < topn)  // setup min-heap
		{
			new_pos = i;
			for (j = 0; j < fill; ++j)
			{
				if (var < ret[j])
				{
					temp1  = ret[j], temp2   = pos[j];
					ret[j] = var,    pos[j]  = new_pos;
					var    = temp1,  new_pos = temp2;
				}
			}
			ret[fill] = var, pos[fill] = new_pos;
			++fill;
		}
		else if (var > ret[0])
			siftdown(ret, pos, var, i, topn);
	}

	if (fill == topn)
	{
		for (size = fill-1; size > 0; --size)
		{
			var       = ret[size], new_pos   = pos[size];
			ret[size] = ret[0],    pos[size] = pos[0];
			siftdown(ret, pos, var, new_pos, size);
		}
	}

	return fill;
}

void siftdown(double *ret, int *pos, double new_var, int new_pos, int heap_size)
{
	int p(0), q, left(1), right(2);

	while (left < heap_size)
	{
		if (ret[left] < new_var)
		{
			if (right < heap_size && ret[right] < ret[left])
				q = right;
			else
				q = left;
		}
		else if (right < heap_size && ret[right] < ret[left])
			q = right;
		else
			break;

		ret[p] = ret[q], pos[p] = pos[q];
		p = q;
		left = (p<<1) + 1;
		right = left + 1;
	}

	ret[p] = new_var, pos[p] = new_pos;
}
