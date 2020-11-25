#include <sys/types.h>
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


typedef struct table
{
    long long int id,ready,slots,index,portion_size;
    long long int arr_student_id_slot[30];
}table;
typedef struct robot
{
    long long int id,ready,vessels,time,portion_size;
}robot;
typedef struct student
{
    long long int id,arrival,slot_status;
}student;

void* robot_init(void* x);
void biryani_ready(void* x);
void* student_init(void*x);

pthread_t** robot_threads;
pthread_t** student_threads;
pthread_t** table_threads;
pthread_mutex_t** mutex_robots;
pthread_mutex_t** mutex_tables;

robot** robots;
student** students;
table** tables;

void wait_for_slot(void* x);
void* student_in_slot(void* l,void* m);
void* table_init(void* x);
void ready_to_serve_table(void* x);

long long int num_of_tables,num_of_robots,num_of_students,students_to_be_serviced,students_arrived;

void* robot_init(void* x)
{
    robot* y=(robot*)x;
    //if students remain again start makiung
    long long int i=0,robot_id;
    robot_id = y->id;
    while(students_to_be_serviced > 0)
    {
        long long int ran1,ran2,ran3;
        ran1 = (rand()%4)+2;
        ran2 = (rand() %9)+1;
        ran3 = (rand() %26)+25;
        y->time=ran1;
        y->ready=0;
        y->vessels=ran2;
        y->portion_size=ran3;
        int z;
        z=ran1;
        if(i==0)
        {
            printf("robot chef %lld is preparig %lld vessels of biryani each vessel can serve %lld students\n",robot_id,y->vessels,y->portion_size);
        }
        else
        {
            z++;
        }
        sleep(ran1);
        printf("robot chef %lld has prepared %lld vessels of biryani. Waiting for allthe vessels to be emptied to resume cooking.\n",y->id,y->vessels);
        int xyz=1;
        y->ready=xyz;
        biryani_ready(y);
        z++;
        i++;
    }
    return NULL;
}
void biryani_ready(void* x)
{
    long long int val = 0;
    robot* y=(robot*)x;
    long long int robo_id = y -> id;
    while(y->vessels);
    y->ready=val;
    printf("all vessels prepared by robot %lld are emptied\n",robo_id);
}
void* student_init(void*x)
{
    student* y=(student*)x;
    long long int ran1;
    ran1 = rand()%15;
    y->arrival=ran1;
    y->slot_status=0;
    sleep(ran1);
    long long int myid=y->id;
    printf("student %lld has arrived\n",myid);
    int xyz=1;
    y->slot_status=xyz;
    xyz++;
    students_arrived++;
    wait_for_slot(y);
    return NULL;
}
void wait_for_slot(void*x)
{
    student* y=(student*)x;
    long long int p=0,i,flag;
    printf("student %lld is waiting to be allocated a slot on the serving table\n",y->id);
    for(i = 0,flag=0; i < num_of_tables; )
    {
        long long int ready = tables[i] -> ready;
        long long int slot_n = tables[i]->slots;
        pthread_mutex_lock(mutex_tables[i]);
        if(ready == 1 && slot_n > 0)
        {
            flag = 1;
            student_in_slot(y,tables[i]);
        }
        pthread_mutex_unlock(mutex_tables[i]);
        if(flag == 1)
        {
            p=1;
            break;
        }
        else
        {
            p=0;
        }
    }
}

void* student_in_slot(void* l,void* m)
{
    student* y=(student*) l;
    table* x=(table*)m;

    int rem=2;
    x->slots--;
    y->slot_status=rem;
    printf("student %lld assigned a slot on serving table %lld and waiting to be served\n",y->id,x->id);
   long long int idnum=y->id;
    x->arr_student_id_slot[x->index]=idnum;
    x->index++;
    students_to_be_serviced--;
}
void* table_init(void* x)
{
    table* y= (table*)x;
    y->ready=0;
    long long int j=0;
    while(students_to_be_serviced > 0)
    {
        int kag=0;
        if((y->ready) >= 1)
        {
            for(long long int i = 0,flag=0; i <num_of_robots; )
            {
                long long int idname = robots[i]->id;
                pthread_mutex_lock(mutex_robots[i]);
                if(robots[i]->ready >= 1)
                {
                    printf("robot chef %lld is refilling servicing container of serving table %lld\n",idname,y->id);
                    robots[i]->vessels--;
                    y->portion_size=robots[i]->portion_size;

                    long long int value_j = j;
                    if(value_j!=0)
                    {
                        printf("serving container of table %lld is refilled by robot chef %lld;table %lld resuming service now.\n",y->id,robots[i]->id,y->id);
                    }
                    else
                    {
                        kag++;
                    }
                    while(y->portion_size >= 1)
                    {
                        long long int random;
                        if(y->portion_size>10)
                        {
                            random = (rand()%10)+1;
                            y->slots=random;
                        }
                        else
                        {
                            random = (rand()%y->portion_size)+1;
                            y->slots=random;
                        }
                        y->portion_size-=y->slots;
                        printf("serving table %lld is ready to serve with %lld slots\n",y->id,y->slots);
                        long long int value=1;
                        j++;
                        y->ready=value;
                        ready_to_serve_table(y);
                    }
                    printf("servicing container of table %lld is empty waiting for refil\n",y->id);
                    flag=1;
                }
                pthread_mutex_unlock(mutex_robots[i]);
                if(flag == 1)
                {
                    break;
                }
                else
                {
                     kag++;
                }
                i++;
            }
        }  
    }
    return NULL;
}
void ready_to_serve_table(void* x)
{
    long long int val = 0;
    table* y= (table*)x;
    while((y->slots > 0) && (students_to_be_serviced > 0));
    y->ready=val;
    long long int idnum=y->id;
    long long int idindex=y->index;
    printf("serving table %lld entering serving phase\n",idnum);
    for(long long int i=0;i<idindex;)
    {
        printf("student %lld on serving table %lld has been served\n",y->arr_student_id_slot[i],y->id);
        i++;
    }
    y->index=0;
    
}

void takeinput()
{
    printf("Enter number of robots\n");
    scanf("%lld",&num_of_robots);
    printf("Enter number of students\n");
    scanf("%lld",&num_of_students);
    printf("Enter number of tables\n");
    scanf("%lld",&num_of_tables);
}

int main(void)
{
    long long int randd1,randd2,randd3;
    randd1 = (rand()%4)+2;
    randd2 = (rand() %9)+1;
    randd3 = (rand() %26)+25;
    long long int robot_time=randd1,robot_vessels=randd2,robot_students=randd3; 
    takeinput();

    students_to_be_serviced=num_of_students;
    int pol=0;
    students_arrived=pol;
    long long int i = 0;

    robots = (robot**) malloc(num_of_robots * sizeof(robot*));
    mutex_robots=(pthread_mutex_t**)malloc(num_of_robots * sizeof(pthread_mutex_t*));
    robot_threads=(pthread_t**)malloc(num_of_robots*sizeof(pthread_t*));
    while(i<num_of_robots)
    {
        robots[i] = (robot*) malloc(sizeof(robot));
        robots[i]->id=(++i);
        i--;
        mutex_robots[i]=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mutex_robots[i], NULL);

        robot_threads[i]=(pthread_t*)malloc(num_of_robots);
        i++;
    }

    students = (student**) malloc(num_of_students * sizeof(student*));
    student_threads=(pthread_t**)malloc(num_of_students *sizeof(pthread_t*));
    i = 0;
    while(i < num_of_students)
    {
        students[i] = (student*) malloc(sizeof(student));
        students[i]->id=(++i);
        i--;
        student_threads[i]=(pthread_t*)malloc(sizeof(pthread_t));
        i++;
    }

    tables = (table**) malloc(num_of_tables * sizeof(table*));
    mutex_tables=(pthread_mutex_t**)malloc(num_of_tables * sizeof(pthread_mutex_t*));
    table_threads=(pthread_t**)malloc(num_of_tables*sizeof(pthread_t*));
    i = 0;
    while(i < num_of_tables)
    {
        tables[i] = (table*) malloc(sizeof(table));
        long long int val = 0;
        tables[i]->id=(++i);
        i--;
        tables[i]->index=val;

        mutex_tables[i]=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mutex_tables[i], NULL);
        table_threads[i]=(pthread_t*)malloc(sizeof(pthread_t));

        i++;
    }

    i = 0;
    while(i<num_of_robots)
    {
        pthread_create(robot_threads[i],NULL,robot_init,(void*)robots[i]);
        sleep(1);
        i++;
    }
    i = 0;
    while(i< num_of_tables)
    {
        pthread_create(table_threads[i],NULL,table_init,(void*)tables[i]);
        sleep(1);
        i++;
    }
    i = 0;
    while(i<num_of_students)
    {
        pthread_create(student_threads[i],NULL,student_init,(void*)students[i]);
        sleep(1);
        i++;
    }
    i = 0;
    while(i<num_of_students)
    {
        pthread_join(*(student_threads[i]),NULL);
        sleep(1);
        i++;
    }
    i = 0;
    while(i<num_of_robots)
    {
        pthread_join(*(robot_threads[i]),NULL);
        sleep(1);
        i++;
    }
    i = 0;
    while(i<num_of_tables)
    {
        pthread_join(*(table_threads[i]),NULL);
        sleep(1);
        i++;
    }
    sleep(1);
    printf("simulatin over\n");
    i = 0;
    while(i<num_of_robots)
    {
        pthread_mutex_destroy(mutex_robots[i]);
        i++;
    }
    i = 0;
    while(i<num_of_tables)
    {
        pthread_mutex_destroy(mutex_tables[i]);
        i++;
    }
}