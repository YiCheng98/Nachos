#include "syscall.h"

#define N 2048
int a[N];

int main(){
	int i,j,tmp;
	for (i=0;i<N;i++)
		a[i]=N-i;
    Halt();	
}
