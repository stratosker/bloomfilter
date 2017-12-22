#include <stdio.h>
#include "sort.h"

void quick_sort (int *a, int size) {
    q_sort(a, 0, size);
}


void q_sort(int *a, int left, int right){
  int pivot, l, r, loops=0;

  l = left;
  r = right;
  pivot = a[left];
  while (left < right)
  {
    while ((a[right] >= pivot) && (left < right)){
      right--;
      loops++;
    }
    if (left != right)
    {
      a[left] = a[right];
      left++;
    }
    while ((a[left] <= pivot) && (left < right)){
        left++;
        loops++;
    }
    if (left != right)
    {
      a[right] = a[left];
      right--;
    }
  }
  a[left] = pivot;
  pivot = left;
  left = l;
  right = r;
  if (left < pivot)
    q_sort(a, left, pivot-1);
  if (right > pivot)
    q_sort(a, pivot+1, right);
}



int removeDuplicates(int* A, int size) {
int j = 0;
	int i = 1;
 
	while (i < size) {
		if (A[i] == A[j]) {
			i++;
		} else {
			j++;
			A[j] = A[i];
			i++;
		}
	}
 
	return j + 1;
}


int write_all ( int fd , void * buff , size_t size ) {
int sent , n ;
for ( sent = 0; sent < size ; sent += n ) {
if (( n = write ( fd , buff + sent , size - sent ) ) == -1)
return -1; /* error */
}
return sent ;
}