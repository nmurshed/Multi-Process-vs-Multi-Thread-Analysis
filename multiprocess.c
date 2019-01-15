#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<sys/errno.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>
#include <math.h>

sem_t buff_full,buff_empty;
int pshared = 1;
int MSGSZ,n_p,n_c;
unsigned int value = 1;
pid_t pid;
typedef struct msgbuf {
    long    mtype;
    int     num;
   
} message_buf;
int    ct1=0,ct2=0;
int quit=0;
int create_producer();
int create_consumer();
int N=0;
int msqid;
struct msqid_ds buffer;
int status;
int B=0;
void produce();
void consume();
#if 0
int producer_block,total_producer_block;
int consumer_block,total_consumer_block;
int request_produced;
int request_processed;

#endif
struct timeval prod_block_start,prod_block_end,cons_block_start,cons_block_end,timer_prod_start, timer_prod_end, timer_cons_start,timer_cons_end;
message_buf buf,rbuf;
int delay_prod_rate=0;
int max_message_size=0;
double probability=0;
double producer_delay();
int message_size();
double consumer_delay();
double delay();
void sighandler(int signum)
{
    if (signum==SIGINT || signum==SIGALRM){
        quit=1;}
}

int main (int argc,char *argv[])
{
    if (signal(SIGALRM,sighandler)==SIG_ERR)
        printf("err");
    if (signal(SIGINT,sighandler)==SIG_ERR)
        printf("err");
   
    struct timeval start, init,end;

    int child_status;
    key_t key = 1234;
        int buffersize = atoi(argv[2]);
    B=atoi(argv[2])*sizeof(int);
    N=atoi(argv[1]);
    n_p = atoi(argv[3]);
    n_c = atoi(argv[4]);
    delay_prod_rate=atoi(argv[5]);
    max_message_size=atoi(argv[6]);
    ct1=atoi(argv[7]);
    ct2=atoi(argv[8]);
    probability=atoi(argv[9]);

    int msgflg = IPC_CREAT | 0666;
    msqid = msgget(key, msgflg);
  //  printf("msqid %d \n",msqid);
    msgctl(msqid,IPC_STAT,&buffer);
    buffer.msg_qbytes=B;
    int r;
    r=msgctl(msqid,IPC_SET,&buffer);
    if (r<0) {
        perror("msgctl");
        exit(1);
    }
    sem_init(&buff_full, 0, 1);
    sem_init(&buff_empty, 0, B);
   
    

  MSGSZ =sizeof(int);
    
      alarm(N);
      create_producer(n_p,n_c);

   
    while(!quit){}
    

    msgctl(msqid,IPC_RMID,0);
    
    
    printf("THE END \n");


    return 0;
}



int create_producer(int m, int n){
   

    
    if(m<=0 && n<=0){
       // printf("All processes created \n");
        return 0;
    }
    pid_t prod_pid;
    pid_t cons_pid;
   
    if(m>0){
        prod_pid= fork();
        if (prod_pid==0){
            produce();
              wait(&status);
            exit(0);
        }
        else if (prod_pid<0){
            printf("error in producer \n");
            exit(1);
        }
    }
    
    
    if(n>0){
        cons_pid= fork();
        if (cons_pid==0){
            consume();
              wait(&status);
            exit(0);
                    }
        else if (cons_pid<0){
            
            printf("error in consumer \n");
            exit(1);
        }
    }
    
    create_producer(m-1,n-1);

    }




void produce(){
//message_buf buf;
    int i=1;
    long snt=0,flag=0,producer_block=0,total_producer_block=0, request_produced=0;
    double temp=0.0,temp1=0.0,total_producer_block_time=0.0,temp_p_block_time=0.0;
    int temp_p_block=0, temp_c_block=0,temp_produced=0;
 //   printf("producer start %d \n", (int)getpid());
    alarm(N);
     gettimeofday(&timer_prod_start,NULL);
    while (!quit){
          double t =producer_delay(delay_prod_rate);
sleep(t*100);
        buf.mtype = 1;
        buf.num =message_size(max_message_size);;
     //   MSGSZ =buf.num*sizeof(int);

      sem_wait(&buff_empty);

        msgctl(msqid,IPC_STAT,&buffer);
     //   printf("%d \n",buffer.msg_qnum);
        
        if(buffer.msg_qnum==(B/sizeof(int))){
            gettimeofday(&prod_block_start,NULL);
      //      printf("Queue is full for producer %d  \n",(int)getpid());
            flag =1;
            producer_block++;
            temp_p_block++;
        }
        else{
        //   printf("Message Sent %d by producer %d  \n", buf.num,(int)getpid());
            msgsnd(msqid, &buf, MSGSZ, 0);
            request_produced++;
            temp_produced++;
        
        if(flag==1){
            gettimeofday(&prod_block_end,NULL);
            temp= (prod_block_end.tv_sec - prod_block_start.tv_sec)+(prod_block_end.tv_usec - prod_block_start.tv_usec)/1000000.0;
            total_producer_block_time+=temp;
            temp_p_block_time=temp;
           
            flag=0;
            }
        }
        gettimeofday(&timer_prod_end,NULL);
        temp= (timer_prod_end.tv_sec - timer_prod_start.tv_sec)+(timer_cons_end.tv_usec - timer_cons_start.tv_usec)/1000000.0;
        if(temp >= 10.00)
        {
        //    printf("Data of producer Round  %d \n" ,i);

           printf("Produced,%d,Blocked,%lu,Blck_time,%f,producer,%d\n", temp_produced, temp_p_block,temp_p_block_time, (int)getpid());
            gettimeofday(&timer_prod_start,NULL);
            temp_p_block=0;
            temp_produced=0;
            i++;
            
        }

        sem_post(&buff_full);
      
    }
    
  //  printf("Blocked request:%lu at producer:%d \n", producer_block,(int)getpid());
    printf("Totalblockingtime,%f,producer, %d \n",total_producer_block_time,(int)getpid());
    printf("\n");


    exit(0);
}



void consume(){

    int rcv,flag=0,consumer_block=0;
    double temp=0.0,total_consumer_block_time=0.0,temp_c_block_time=0.0;
    int request_processed=0,temp_processed=0,temp_c_block=0;
     gettimeofday(&timer_cons_start,NULL);
  //  MSGSZ= 10*sizeof(int);
    alarm(N);
  //  printf("consumer start %d \n", (int)getpid());
    int i=1;
       while(!quit){
           double t =consumer_delay(probability);
           sleep(t*100);
         //  printf("%f  \n", t);
           
           
           sem_wait(&buff_full);
           msgctl(msqid,IPC_STAT,&buffer);
          // printf("%d \n",buffer.msg_qnum);
           
           if(buffer.msg_qnum==0){
               gettimeofday(&cons_block_start,NULL);
               consumer_block++;
               temp_c_block++;
               flag =1;
           }
           else{
               
              msgrcv(msqid, &rbuf, MSGSZ, 1, 0);
           //   printf("Message received %d by consumer %d \n", rbuf.num,(int)getpid());
               request_processed++;
               temp_processed++;
               
           
           if(flag==1){
               gettimeofday(&cons_block_end,NULL);
               temp= (cons_block_end.tv_sec - cons_block_start.tv_sec)+(cons_block_end.tv_usec - cons_block_start.tv_usec)/1000000.0;
               total_consumer_block_time+=temp;
               temp_c_block_time=temp;
               flag=0;
           }
           }
               gettimeofday(&timer_cons_end,NULL);
               temp= (timer_cons_end.tv_sec - timer_cons_start.tv_sec)+(timer_cons_end.tv_usec - timer_cons_start.tv_usec)/1000000.0;

           if(temp >= 10.00)
           {
               
            //   printf("Data of consumer Round  %d \n" ,i);
             printf("Processed,%d,Blcked,%d,Blck_time,%f,consumer, %d\n",temp_processed,temp_c_block,temp_c_block_time,(int)getpid());
               temp_c_block=0;
               temp_processed=0;
               i++;
               
                              gettimeofday(&timer_cons_start,NULL);
           }

     sem_post(&buff_empty);

       }
    
    //printf("Blocked process:%d at consumer:%d \n", consumer_block,(int)getpid());
   printf("Total blocking time,%f,consumer,%d \n",total_consumer_block_time,(int)getpid());
    printf("\n");
  
  exit(0);
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
     time_t t;
     srand((unsigned) time(&t));
    c=(double)rand()/(double)(RAND_MAX);
    if(c!=0)
    {
         r=rand()%msize;
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






