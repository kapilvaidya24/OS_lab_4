#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <stdlib.h> 
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

char* serverip;
char* port;
int duration;
int think_time;
int mode;
struct timeval tstart;
struct timeval end_prog_time;
struct timeval tcurr;
int trial_amt[1000];
double res_time[1000];


void *client(int tid){
    int id=tid;

    char buffer[1024];


    struct timeval tthread;

    //printf("thread with pid %d started\n",id);



    while(1){
						
	        gettimeofday(&tthread,NULL);

	        double elapsed_time,response_time;

	        elapsed_time=0.00-(double)(tstart.tv_sec-tthread.tv_sec+(double)(tstart.tv_usec-tthread.tv_usec)/1000000);

	        //printf("elapsed time of thread id %d -> %f\n",id,elapsed_time);

	        if(elapsed_time>(float)(duration))
	        {
	            break;
	        }

	       // printf("thread with pid %d is requesting a file\n",id);        


	        struct timeval start_res,end_res;

	        gettimeofday(&start_res,NULL);


	        int sockfd, portno, n;
	        struct sockaddr_in serv_addr;

	        /* create socket, get sockfd handle */

	        portno = atoi(port);
	        sockfd = socket(AF_INET, SOCK_STREAM, 0);
	        if (sockfd < 0) 
	            error("ERROR opening socket");

	        /* fill in server address in sockaddr_in datastructure */

	        bzero((char *) &serv_addr, sizeof(serv_addr));
	        serv_addr.sin_family = AF_INET;
	        serv_addr.sin_addr.s_addr=inet_addr(serverip);
	        serv_addr.sin_port = htons(portno);

	        /* connect to server */

	        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	            error("ERROR connecting");

	        //printf("connected to server thread %d\n",id);

	        /* choose file*/
	        int no;
	        if(mode==1){
	            no=0;
	        }
	        else{

	            no=rand()%10000;
	        }
	        char str[10];
	        sprintf( str, "%d", no );
	        bzero(buffer,1024);
	        strcpy(buffer,"get files/foo");
	        strcat(buffer,str);
	        int i;
	        
	        for(i=0;;i++){
	                       if(buffer[i]=='\0'){
				                buffer[i]='.';
				                buffer[i+1]='t';
				                buffer[i+2]='x';
				                buffer[i+3]='t';
				                buffer[i+4]='\0';
				                break;
				            					}
	        			}


	        ////////////////////////////////////hatao isko
	        //strcpy(buffer,"get temp.txt");
	        //////////////////////////////////////////////////////////////////////////////////////////////////////

	        /* send user message to server */
	        //printf("sending server file name thread %d : %s \n",id,buffer);

	        n = write(sockfd,buffer,strlen(buffer));
	        if (n < 0) 
	             error("ERROR writing to socket");
	        bzero(buffer,1024);

	        
	        /* read reply from server */
	        while(1){
		            bzero(buffer, 1024);
		            n = read(sockfd,buffer,1024);
		            if (n < 0) 
		                 error("ERROR reading from socket");
		            if(n==0) {
		            	//printf("thread with pid %d socket closed\n",id);
						break;} //file ended
	            	}
	        /* close the socket */
	        close(sockfd);
	        //printf("read from server and closing socket for thread %d\n",id);

	        gettimeofday(&end_res,NULL);

	        response_time=0-(double)(start_res.tv_sec-end_res.tv_sec+(double)(start_res.tv_usec-end_res.tv_usec)/1000000);
	        res_time[id]=res_time[id]+response_time;
	        trial_amt[id]++;

	        //printf("trial amount of thread %d increased to %d\n",id,trial_amt[id]);
	        //printf("response amount of thread %d increased to %f\n",id,res_time[id]);


	        //printf("thread with pid %d is sleeping\n",id);
	        sleep(think_time);
	      
    }
    //printf("thread with pid %d is exiting\n",id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{


	int jr=0;

	for(jr=0;jr<1000;jr++)
	{
		trial_amt[jr]=0;
		res_time[jr]=0;
	}
    srand (time(NULL));

    gettimeofday(&tstart,NULL);



    if (argc < 7) {
       fprintf(stderr,"usage %s serverip, port, no. of threads, duration, think_time, mode \n", argv[0]);
       exit(0);
    }
    serverip=argv[1];
    port=argv[2];
    duration=atoi(argv[4]);
    think_time=atoi(argv[5]);
    int NUM_THREADS=atoi(argv[3]);
    if(argv[6][0]=='r'){
        mode=0;
    }
    else{
        mode=1;
    }
    /////////////////////////////////////////////////////////////////////////
    
    //printf("%d\n",NUM_THREADS);
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    void* status;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int rc;
    int t;
    for(t=0; t<NUM_THREADS; t++){
       //printf("In main: creating thread %ld\n", t);
       rc = pthread_create(&threads[t], NULL, client, t);
       if (rc){
          //printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
       }

    }

    pthread_attr_destroy(&attr);
    for(t=0; t<NUM_THREADS; t++){
       rc = pthread_join(threads[t], &status);
       if (rc){
          //printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
       }
       //printf("Main has completed joining threads %d\n",t);
    }
    /////////////////////////////////////////////////////////////make threads

    double total_response_time=0,total_response=0;

    int counter=0;

    for(counter=0;counter<NUM_THREADS;counter++)
    {
    	total_response_time=total_response_time+res_time[counter];
    	total_response=total_response+trial_amt[counter];
    }

    gettimeofday(&end_prog_time,NULL);

    double running_time=0-(double)(tstart.tv_sec-end_prog_time.tv_sec+(double)(tstart.tv_usec-end_prog_time.tv_usec)/1000000);;

    double throughput=total_response/running_time*1.00;
    double average_response_time=total_response_time/total_response;


    printf("Num of threads is %d\n",NUM_THREADS);
    printf("Throughput is: %f\n",throughput);

    printf("average response time is: %f\n\n\n\n\n",average_response_time);

    pthread_exit(NULL);
    return 0;
}
