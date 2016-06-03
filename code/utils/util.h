#ifndef Util
#define Util

int topN(double *src, int len, double *ret, int *pos, int topn);
void siftdown(double *ret, int *pos, double new_var, int new_pos, int heap_size);

#endif