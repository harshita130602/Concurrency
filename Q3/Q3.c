#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

int payment_array[10000];
int index1,index2;
int n_cabs,m_riders,k_payments;

sem_t  counter;
sem_t total_num_of_cabs;

pthread_mutex_t mutex;
pthread_mutex_t mutex1;
pthread_t pay;

int payment[10000];

typedef struct cabs
{
    int waiting_status,premier,pool_1,pool_2;
}cabs;
struct cabs cabs_array[10000];

typedef struct book_cab_str
{
    int id;
    int cab_type;
}book_cab_str;


void* payment_func(void* inputs)
{
    book_cab_str *info = (book_cab_str *)inputs;

    int rider_id_num = payment_array[index2];
    index2 ++;

    pthread_mutex_lock(&mutex1);
    int paid;
    for(int i = 1; i <= k_payments; i++)
    {
        int flag = 0;
        if(payment[i] == 0)
        {
            flag = 1;
            paid = i;
            payment[i] = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex1);
    sleep(2);

    pthread_mutex_lock(&mutex1);
    payment[paid] = 0;
    pthread_mutex_unlock(&mutex1);

    printf("Rider %d payment done by counter %d\n",rider_id_num,paid);
}



void* BookCab(void* inp)
{
    book_cab_str* new = (book_cab_str*)inp;
    int cab_type = new->cab_type;
    int id = new->id;

    int exist = 0,cab_num;
    int wait_time = (rand() % 10);
    int ride_time = (rand() % 10);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    if(cab_type == 0)
    {
        int rc;
        ts.tv_sec+=(time_t)wait_time;


        rc = sem_timedwait(&total_num_of_cabs,&ts);
        if(rc==-1)
        {
            if(errno == ETIMEDOUT)
            {
                printf("Rider %d Cannot wait any longer for the premier cab\n",id);
            }
            pthread_exit(NULL);
        }
        else
        {
            pthread_mutex_lock(&mutex);
            int cab_num;
            for(int i = 1; i <= n_cabs; i++)
            {
                if(cabs_array[i].waiting_status == 1)
                {
                    cabs_array[i].premier = 1;

                    cab_num = i;

                    cabs_array[i].waiting_status = 0;
                    cabs_array[i].pool_1 = 0;
                    cabs_array[i].pool_2 = 0;
                    break;
                }
            }
            printf("Rider %d gets in the available premier cab %d\n", id,cab_num);
            pthread_mutex_unlock(&mutex);

            sleep(ride_time);

            pthread_mutex_lock(&mutex);
            cabs_array[cab_num].waiting_status = 1;
            cabs_array[cab_num].premier = 0;
            cabs_array[cab_num].pool_1 = 0;
            cabs_array[cab_num].pool_2 = 0;
            pthread_mutex_unlock(&mutex);
            printf("Rider %d reaches the destination\n",id);
        }
    }
    else if(cab_type == 1)
    {
        int exist = 0;
        pthread_mutex_lock(&mutex);
        int cab_number;
        for(int i = 1; i <= n_cabs; i++)
        {
            if(cabs_array[i].pool_1 == 1)
            {
                exist = 1;
                cabs_array[i].pool_2 = 1;

                cab_number = i;

                cabs_array[i].waiting_status = 0;
                cabs_array[cab_num].premier = 0;
                cabs_array[i].pool_1 = 0;
                printf("Rider %d gets in the pool cab %d with 2 passengers \n", id,i);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        if(exist == 0)
        {
            int rc;
            ts.tv_sec+=(time_t)wait_time;
            rc=sem_timedwait(&total_num_of_cabs,&ts);
            pthread_mutex_lock(&mutex);
            ts.tv_sec-=(time_t)wait_time;
            if(rc == -1)
            {
                if(errno == ETIMEDOUT)
                {
                        printf("Rider %d Cannot wait any longer for the pool cab\n",id);
                }
                pthread_mutex_unlock(&mutex); 
                pthread_exit(NULL);
                
            }
            else
            {
                for(int i = 1; i<=n_cabs; i++)
                {
                    if(cabs_array[i].waiting_status == 1)
                    {
                        cabs_array[i].pool_1 = 1;

                        cab_number = i;

                        cabs_array[i].waiting_status = 0;
                        cabs_array[cab_num].premier = 0;
                        cabs_array[i].pool_2 = 0;
                        printf("Rider %d gets in the available pool cab %d\n", id,i);
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&mutex);

            sleep(ride_time);

            pthread_mutex_lock(&mutex);
            if(cabs_array[cab_number].pool_1 == 1)
            {
                cabs_array[cab_number].waiting_status = 1;
                cabs_array[cab_number].premier = 0;
                cabs_array[cab_number].pool_1 = 0;
                cabs_array[cab_number].pool_2 = 0;
                sem_post(&total_num_of_cabs);    
            }
            if(cabs_array[cab_number].pool_2 == 1)
            {
                cabs_array[cab_number].waiting_status = 0;
                cabs_array[cab_number].premier = 0;
                cabs_array[cab_number].pool_2 = 0;
                cabs_array[cab_number].pool_1 = 1;
            }
            printf("Rider %d reaches the destination\n",id);
            pthread_mutex_unlock(&mutex);
        }
        if(exist == 1)
        {
            sleep(ride_time);
            pthread_mutex_lock(&mutex);
            if(cabs_array[cab_number].pool_1 == 1)
            {
                cabs_array[cab_number].waiting_status = 1;
                cabs_array[cab_number].premier = 0;
                cabs_array[cab_number].pool_1 = 0;
                cabs_array[cab_number].pool_2 = 0;
                sem_post(&total_num_of_cabs);
            }
            if(cabs_array[cab_number].pool_2 == 1)
            {
                cabs_array[cab_number].waiting_status = 0;
                cabs_array[cab_number].premier = 0;
                cabs_array[cab_number].pool_2 = 0;
                cabs_array[cab_number].pool_1 = 1;
            }
            printf("Rider %d reaches the destination\n",id);
            pthread_mutex_unlock(&mutex);
        }
    }
    payment_array[index1] = id;
    index1++;
    sem_wait(&counter);
    pthread_create(&pay,NULL,payment_func,new);
    pthread_join(pay,NULL);
    sem_post(&counter);
}


int main(void)
{
    index1 = 1;
    index2 = 1;
    for(int i=1; i<10000;i++)
    {
        payment[i] = 0;
    }
    printf("Enter number of cabs\n");
    scanf("%d",&n_cabs);
    sem_init(&total_num_of_cabs,0,n_cabs);
    printf("Enter number of riders\n");
    scanf("%d",&m_riders);
    printf("Enter number of payment servers\n");
    scanf("%d",&k_payments);
    sem_init(&counter,0,k_payments);


    for(int i = 1; i <= m_riders ; i++)
    { 
        cabs_array[i].waiting_status = 1;
        cabs_array[i].premier = 0;
        cabs_array[i].pool_1 = 0;
        cabs_array[i].pool_2 = 0;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex1, NULL);

    pthread_t riders_thread[m_riders+1];

    struct book_cab_str *type = (struct book_cab_str *)malloc(sizeof(struct book_cab_str));

    for(int i = 1; i <= m_riders; i++)
    {
    //  cab_type range = 0,1---------> 0 = premier, 1 = pool

        type->id = i;
        type->cab_type = (rand() % 2);

        pthread_create(&riders_thread[i],NULL,BookCab,(void *)type);
        sleep(1);

    }
    for(int i = 1; i <= m_riders; i++)
    {
        pthread_join(riders_thread[i], NULL); 
    }
    return 0;
}