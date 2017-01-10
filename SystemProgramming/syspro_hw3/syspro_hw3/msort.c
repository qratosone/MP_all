/** @file msort.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dat[MAXN];

#define MAXN 130000000

typedef struct 
{
    int *dat;
    size_t cnt;
} sort_struct;

typedef struct 
{
    int *d1;
    int *d2;
    int l1;
    int l2;
} merge_struct;

int cmp(const void* a, const void* b) 
{
    return *(int*)a - *(int*)b;
}
void *myqsort(void* _args) {
    sort_struct *args = (sort_struct*)_args;
    fprintf(stderr, "Sorted %zu elements.\n", args->cnt);
    qsort(args->dat, args->cnt, sizeof(int), cmp);
    return 0;
}
void msort(int *dat,int input_ct,int segment_count)
{
	int values_per_seg;
	if(input_ct%segment_count==0)
	{
		values_per_seg=input_ct/segment_count;
	}
	else
	{
		values_per_seg=input_ct/segment_count+1;
	}
	pthread_t p[segment_count];
	sort_struct sort_args[segment_count];
	int i;
	for(i=0;i<segment_count;i++)
	{
		sort_args[i].dat=&dat[i*values_per_seg];
		if(i!=segment_count-1||input_ct%segment_count==0)
		{
			count=values_per_seg;
		}
		else
		{
			count=input_ct%segment_count;
		}
		pthread_create(&p[i],NULL,(void*)myqsort,&sort_argsp[i]);
	}
	for(i=0;i<segment_count;i++)
	{
		pthread_join(p[i],NULL);
	}
	for(i=values_per_seg;i<input_ct;i*=2)
	{
		int count=input_ct/(i*2);
		if(input_ct%(2*i))!=0)
		{
			count++;
		}
		pthread_t p[count];
		merge_struct args[count];
		for(j=0;j<count;j++)
		{
			
		}
	}
}

void output(int* dat,int input_ct)
{
	int i=0;
	for(i=0;i<input_ct;i++)
	{
		printf("%d",dat[i]);
	}
}
int main(int argc, char **argv)
{
    int input_ct=0;
	int segment_count=atoi(argv[1]);
	while(scanf("%d",&dat[input_ct])==1){
		input_ct++;
	}
	msort(dat,input_ct,segment_count);
	output(dat,input_ct);
	return 0;
}
