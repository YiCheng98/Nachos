#include "syscall.h"

#define N 16
int a[N];

int main(){
	int i,j,tmp;
	for (i=0;i<N;i++)
		a[i]=N-i;
	for (i=0;i<N;i++)
		for (j=i+1;j<N-1;j++)
			if (a[i]>a[j]){
				tmp=a[i];
				a[i]=a[j];
				a[j]=tmp;
			}
    Halt();	
}
