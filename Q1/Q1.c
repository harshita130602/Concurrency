#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>


long long int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    long long int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (long long int*)shmat(shm_id, NULL, 0);
}


void swap_func(long long int *arr,long long int index_1,long long int index_2)
{
    long long int temp;
    temp = arr[index_1];
    arr[index_1] = arr[index_2]; 
    arr[index_2] = temp;
}
  

long long int partition_func(long long int *arr, long long int start, long long int end) 
{ 
    srand(time(0)); 
    long long int random = start + (rand() % (end - start)); 
    swap_func(arr,random,end); 

    long long int pivot = arr[end]; // pivot 
    long long int p_index = (start - 1); // Index of smaller element 
    
    long long int j = start;
    while(j < end)
    { 
        if (arr[j] <= pivot)
        { 
            p_index = p_index + 1;
            swap_func(arr,p_index,j); 
        }
        j++; 
    } 
    swap_func(arr,p_index+1,end); 
    return (p_index + 1); 
} 
  

void concurrent_quicksort(long long int *arr, long long int start, long long int end)
{
    long long int pi;
    if(start > end)
    { 
        _exit(1);
    }    
    else if(start < end)
    {
        if(end-start+1<=5) //insertion sort
         {
            long long int a[5], mi=INT_MAX, mid=-1;
            for(long long int i=start;i<end;i++)
            {
                long long int j=i+1; 
                for(; j <= end; j++)
                    if(arr[j]<arr[i] && j<=end) 
                    {
                        swap_func(arr,i,j);
                    }
            }
            return;
        }
        else
        {
            pi = partition_func(arr, start, end); 
        }
    }
    int left_pid1;
    int right_pid2;
    left_pid1 = fork();
    if(left_pid1 == 0)
    {
        //sort left half array
        concurrent_quicksort(arr, start , pi - 1);
        _exit(1);
    }
    else
    {
        right_pid2 = fork();
        if(right_pid2 == 0)
        {
            //sort right half array
            concurrent_quicksort(arr, pi + 1 , end);
            _exit(1);
        }
        else
        {
            //wait for the right and the left half to get sorted
            int status;
            waitpid(left_pid1, &status, 0);
            waitpid(right_pid2, &status, 0);
        }
    }
    return;
}


struct arg
{
    long long int l;
    long long int r;
    long long int* arr;    
};
void *threaded_quicksort(void* a)
{
    struct arg *args = (struct arg*) a;

    long long int l = args->l;
    long long int r = args->r;
    long long int *arr = args->arr;
    if(l > r)
    { 
        return NULL;
    }    
    
    //insertion sort
    if(l < r)
    {
        if(r-l+1<=5)
        {
            long long int a[5], mi=INT_MAX, mid=-1;
            for(long long int i=l;i<r;i++)
            {
                long long int j=i+1; 
                for(;j<=r;j++)            
                    if(arr[j] < arr[i] && j <= r) 
                    {
                         swap_func(arr,i,j);
                    }
            }
            return NULL;
        }
        else
        {
            long long int pi;
            pi = partition_func(arr, l, r); 
    
        //sort left half array
            struct arg a1;
            a1.l = l;
            a1.r = pi - 1;
            a1.arr = arr;
            pthread_t tid1;
            pthread_create(&tid1, NULL, threaded_quicksort, &a1);
            
            //sort right half array
            struct arg a2;
            a2.l = pi + 1;
            a2.r = r;
            a2.arr = arr;
            pthread_t tid2;
            pthread_create(&tid2, NULL, threaded_quicksort, &a2);
            
            //wait for the two halves to get sorted
            pthread_join(tid1, NULL);
            pthread_join(tid2, NULL);
        }
    }
}


void normal_quicksort(long long int *arr, long long int start, long long int end)
{
    if (start < end)
    { 
        long long int pi = partition_func(arr, start, end); 
        normal_quicksort(arr, start, pi - 1); 
        normal_quicksort(arr, pi + 1, end); 
    } 
} 


void display_array(long long int *arr, long long int size) 
{
    for (long long int i = 0; i <= size -1; i++)
    { 
        printf("%lld ", arr[i]);
    } 
    printf("\n"); 
}


void runSorts(long long int n)
{
    struct timespec ts;

    //getting shared memory
    long long int *arr = shareMem(sizeof(long long int)*(n+1));

    for(long long int i=0;i<n;i++)
    {
        scanf("%lld", arr+i);
    }

    long long int brr[n+1],crr[n+1];
    for(long long int i=0;i<n;i++)
    {
        brr[i] = arr[i];
        crr[i] = arr[i];
    }

//concurrent quicksort part
    printf("Running concurrent_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;

    //multiprocess mergesort
    concurrent_quicksort(arr, 0, n-1);
    display_array(arr,n);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en-st;


//threaded quick sort part
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n-1;
    a.arr = brr;
    printf("Running threaded_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    //multithreaded quicksort
    pthread_create(&tid, NULL, threaded_quicksort, &a);
    pthread_join(tid, NULL); 
    display_array(brr,n);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;


//normal quicksort part
    printf("Running normal_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    // normal quicksort
    normal_quicksort(crr, 0, n-1); 
    display_array(crr,n);   
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;

    printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_quicksort\n\n\n", t1/t3, t2/t3);

    shmdt(arr);
    shmdt(brr);
    shmdt(crr);
    return;
}


int main(void)
{

    long long int n;
    scanf("%lld", &n);
    runSorts(n);
    return 0;
}