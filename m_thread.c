    //
//  m_thread.c
//  
//
//  Created by Niyaz Murshed on 2016-10-18.
//
//


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <semaphore.h>

#define TRUE 1
#define FALSE 0
int N=0;
int B=0;
int prod=0;
int cons=0;
int delay_prod_rate=0;
double probability=0;
int    ct1=0,ct2=0;
int max_message_size=0;
pthread_mutex_t lock;
int bindex = 0;
int* number;
int processed_request=0;
int produced_request=0;
int temp_produced=0,temp_processed=0;
int producer_block=0;
int consumer_block=0;
int temp_pblock=0,temp_cblock=0;
int quit = 0;
sem_t buff_full;
sem_t buff_empty;
double producer_delay();
void produce_data();
void consume_data();
int message_size();
double consumer_delay();
double delay();
void sighandler(int);

pthread_attr_t attr;
char BLOCKFLAG,BLOCKFLAG_CONS,START_PROD,DISP_PROD,START_CONS,DISP_CONS;
struct timeval block_start1,block_end1,block_start2,block_end2,timer_prod_start,timer_prod_end,timer_cons_start,timer_cons_end;
float temp,prod_block_time, cons_block_time,temp_p_b_time=0.0,temp_c_b_time=0.0;

void sighandler(int signum)
{
    if (signum==SIGINT || signum==SIGALRM)
    {
     //   printf("handler %d\n",signum);
        quit=1;
    }
}

void* producer()
{
 //   printf("producer start\n");
 //   alarm(N);
    while (1)
    {
        if(quit==0)
        {
        double t =producer_delay(delay_prod_rate);
       // printf("producer delay %f \n", t);
        sleep(t*1000);
        int temp=message_size(max_message_size);
        sem_wait (&buff_empty);
        pthread_mutex_lock(&lock);
        produce_data(temp);
        pthread_mutex_unlock(&lock);
        sem_post(&buff_full);
        }
        else
            pthread_exit(NULL);
    }
}


void* consumer (){
    
 //   printf("consumer start\n");
 //   alarm(N);
    
    while (1) {
        
        if(quit==0){
        sem_wait(&buff_full);
        pthread_mutex_lock(&lock);
        consume_data();
        pthread_mutex_unlock(&lock);
        sem_post(&buff_empty);
        double t =consumer_delay(probability);
       // printf("consumer delay %f \n", t);
        sleep(t*1000);
    }
    else
    pthread_exit(NULL);
}
}


double producer_delay(int lamda)
{
    double a,r;
    a=(double)rand()/(double)(RAND_MAX);
    if(a!=0)
    {
        r=-log(a)/lamda;
        return r;
    }
    else
    {
        producer_delay(lamda);
    }
}

int message_size(int msize)
{
    double c;
    int r;
    c=(double)rand()/(double)(RAND_MAX);
    if(c!=0)
    {
        double r=-msize*log(c);
        return r;
    }
    else
    {
        message_size(msize);
    }
}

double consumer_delay(double prob)
{
    double c,r;
    c=(double)rand()/(double)(RAND_MAX);
    if(c<=prob)
    {
        return delay(ct1);
    }
    else
    {
        return delay(ct2);
    }
}


double delay(int lamda)
{
    double a,r;
    a=(double)rand()/(double)(RAND_MAX);
    if(a!=0)
    {
        r=-log(a)/lamda;
        return r;
    }
    else
    {
        delay(lamda);
    }
}




int main (int argc , char *argv[]) {
    if (signal(SIGALRM,sighandler)==SIG_ERR)
       printf("err");
   if (signal(SIGINT,sighandler)==SIG_ERR)
        printf("err");
    N=atoi(argv[1]);
    B=atoi(argv[2]);
    prod =atoi(argv[3]);
    cons= atoi(argv[4]);
    delay_prod_rate=atoi(argv[5]);
    max_message_size=atoi(argv[6]);
    ct1=atoi(argv[7]);
    ct2=atoi(argv[8]);
    probability=atoi(argv[9]);
    bindex=0;
    pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t)*(prod+cons));
    sem_init(&buff_full, 0, 0);
    sem_init(&buff_empty, 0, B);
    number = (int *)malloc(sizeof(int)*B);
    pthread_mutex_init(&lock, NULL);
    alarm(N);
    double temp1;
    
    for (int k=0; k<prod; k++)
    {
        pthread_create(&thread[k],NULL,producer,NULL);
        pthread_join(&thread[k],NULL);
    }
    
    for (int k=0; k<cons; k++)
    {
        int t = prod+k;
        pthread_create(&thread[t],NULL,consumer,NULL);
        pthread_join(&thread[t],NULL);
    }
    
    
    
    gettimeofday(&timer_prod_start,NULL);
    
       while (!quit) {
        
        if(START_PROD == TRUE)
        {
            gettimeofday(&block_start1, NULL);
            START_PROD = FALSE;
        }
        
        if(START_CONS == TRUE)
        {
            gettimeofday(&block_start2, NULL);
            START_CONS = FALSE;
        }
           
        
        if(BLOCKFLAG_CONS == TRUE)
        {
            if(bindex != 0)
		          {
                      BLOCKFLAG_CONS = FALSE;
                      gettimeofday(&block_end2, NULL);
                      cons_block_time = cons_block_time + (block_end2.tv_sec - block_start2.tv_sec)+(block_end2.tv_usec - block_start2.tv_usec)/1000000.0;
                      temp_c_b_time= cons_block_time + (block_end2.tv_sec - block_start2.tv_sec)+(block_end2.tv_usec - block_start2.tv_usec)/1000000.0;
                      
                  }
            
        }
        
        if(BLOCKFLAG == TRUE)
        {
            if(bindex != B)
            {
                BLOCKFLAG = FALSE;
                gettimeofday(&block_end1, NULL);
                prod_block_time = prod_block_time + (block_end1.tv_sec - block_start1.tv_sec)+(block_end1.tv_usec - block_start1.tv_usec)/1000000.0;
                 temp_p_b_time= prod_block_time + (block_end1.tv_sec - block_start1.tv_sec)+(block_end1.tv_usec - block_start1.tv_usec)/1000000.0;
                
            }
            
        }
           gettimeofday(&timer_prod_end, NULL);
           temp1 = (timer_prod_end.tv_sec - timer_prod_start.tv_sec)+(timer_prod_end.tv_usec - timer_prod_start.tv_usec)/1000000.0;
           
           if(temp1 >= 10.0)
           {
              printf("Request Produced =%d \n",temp_produced);
               printf("Processed Request =%d \n",temp_processed);
               temp_produced=0;
               temp_processed=0;
               if(DISP_PROD == TRUE)
               {
                   
                   gettimeofday(&block_end1, NULL);
                   temp1 = prod_block_time + (block_end1.tv_sec - block_start1.tv_sec)+(block_end1.tv_usec - block_start1.tv_usec)/1000000.0;
                   //temp = temp+ prod_block_time;
                   
                
                  printf("Producer blocked count =%d \n",temp_pblock);
                   temp_pblock=0;
                 printf("Prod.BlockTime: %f seconds\n",temp_p_b_time);
                   temp_p_b_time=0.0;
                   
               }
               if(DISP_CONS == TRUE)
               {
                   gettimeofday(&block_end2, NULL);
                   temp1 = cons_block_time + (block_end2.tv_sec - block_start2.tv_sec)+(block_end2.tv_usec - block_start2.tv_usec)/1000000.0;
                   //                temp = temp + cons_block_time;
                   
                 printf("Consumer blocked count =%d \n",temp_cblock);
                   temp_cblock=0;
                  printf("Cons.BlockTime: %f seconds\n",temp_c_b_time);
                   temp_c_b_time=0.0;
                  printf("\n");
               }
               gettimeofday(&timer_prod_start, NULL);
               
           }
    }
    
    
    if(DISP_PROD == TRUE)
    {
        
        gettimeofday(&block_end1, NULL);
        temp = prod_block_time + (block_end1.tv_sec - block_start1.tv_sec)+(block_end1.tv_usec - block_start1.tv_usec)/1000000.0;
        
        printf("Total Prod.BlockTime: %f seconds\n",temp);
        DISP_PROD = FALSE;
        
    }
    
    if(DISP_CONS == TRUE)
    {
        
        gettimeofday(&block_end2, NULL);
        temp = cons_block_time + (block_end2.tv_sec - block_start2.tv_sec)+(block_end2.tv_usec - block_start2.tv_usec)/1000000.0;
 
        
        printf("Total Cons.BlockTime: %f seconds\n",temp);
        DISP_CONS = FALSE;
        
    }
    
    pthread_mutex_destroy(&lock);
    sem_destroy(&buff_empty);
    sem_destroy(&buff_full);
    
    
  //  printf("Production Complete \n");
   // printf("Request Produced %d \n",produced_request);
   // printf("Request Completed %d \n",processed_request);
  //  printf("Producer blocked %d \n",producer_block);
  //  printf("Consumer blocked %d \n",consumer_block);
    return 0;
}




void produce_data(int temp)
{
            if (bindex <= B){
                bindex+=temp;
                number[bindex++] = temp;
               
             //   printf("Producer %d thread sent %d\n",(int)pthread_self(),temp);
                produced_request++;
                temp_produced++;
                if(bindex==B){
                    BLOCKFLAG=TRUE;
                    DISP_PROD = TRUE;
                    START_PROD = TRUE;
                }
            }
            else if (BLOCKFLAG==TRUE)
            {
               
            //    printf("Buffer overflow by producer %d \n",(int)pthread_self());
                producer_block++;
                temp_pblock++;
            }
}


void consume_data(){
        int temp;
        if (bindex > 0)
        {
           
            temp=number[--bindex];
            bindex-=temp;
         //    printf("Consumer %d thread received %d\n",(int)pthread_self(),temp);
             processed_request++;
            temp_processed++;
            if(bindex==0){
                BLOCKFLAG_CONS=TRUE;
                DISP_CONS=TRUE;
                START_CONS=TRUE;
            }
        }
        else if (BLOCKFLAG_CONS==TRUE)
        {
        //    printf("Buffer underflow by consumer %d \n",(int)pthread_self());
             consumer_block++;
            temp_cblock++;
        }
}





















