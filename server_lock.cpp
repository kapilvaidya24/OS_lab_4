/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <strings.h>

using namespace std;

pthread_mutex_t lock;
int NUM_THREADS=100;
pthread_cond_t buffer_has_space,buffer_has_data;
int queue_count=0,max_count=10;


queue<int> fd_q;


void error(char* msg)
{
    perror(msg);
    exit(1);
}

void *client(void * t)
{   
    int* temp=(int *) t;
    int tid=*temp;
    int newsockfd;
    char buffer[2048];
    int n;

    printf("Thread with tid %d started\n",tid);


    for(;;)
    {

        pthread_mutex_lock(&lock);
        while(queue_count==0)
        {
            pthread_cond_wait(&buffer_has_data,&lock);
        }
        newsockfd=fd_q.front();
        queue_count--;
        fd_q.pop();
        pthread_cond_signal(&buffer_has_space);
        pthread_mutex_unlock(&lock);


        bzero(buffer,1024);
                 n = read(newsockfd,buffer,1023);
                 if (n < 0) error("ERROR reading from socket");
                // printf("Here is the input file name: %s\n",buffer);

                 /* open file */
                 char fil[1024];
                 //strcpy(fil,buffer+4);
                 int i;
                 for(i=0;;i++)
                    {
                        fil[i]=buffer[i+4];
                        if(buffer[i+4]=='\0') break;

                    }
                         for(i=0;;i++){
                            if(fil[i]=='\n') fil[i]='\0';
                            if(fil[i]=='\0'){
                                break;
                            }
                         }
                // printf("file to retrieve has name: %s\n",fil);
                 int fd = open(fil,O_RDONLY);
                 if(fd<0){
                    error("Error in opening file\n");
                 }

                // printf("sending message\n");
                 
                 int total_bytes=0;

                 while(1){
                            bzero(buffer,1024);
                            n = read(fd,buffer,1024);
                            if (n < 0){
                                error("ERROR reading file");
                            }
                            char *p=buffer;
                            int temp_n=n,temp_write;

                            total_bytes=total_bytes+n;

                            if(n==0) {break;}

                            while(temp_n>0){

                                        temp_write=write(newsockfd,p,temp_n);

                                        if ( temp_write< 0) error("ERROR writing to socket");
                                        temp_n=temp_n-temp_write;
                                        p=p+temp_write;


                                        }
                           

                        }

                // printf("total bytes sent are: %d\n",total_bytes);       

                // printf("message sent successfully\n");

                 /* close socket */
                 close(newsockfd);
                 close(fd);

    }

    void * cgh;
    return cgh;

}


int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, t;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 4) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     /* create socket */

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

    int *suraj;
    int temp_2=1;
    suraj=&temp_2;
     if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, suraj, sizeof(int)) < 0)
         error("setsockopt(SO_REUSEADDR) failed");

     /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     NUM_THREADS=atoi(argv[2]);
     if(atoi(argv[1])==0)
     {
     	max_count=1000000000;
     }
     else
     {
     	max_count=atoi(argv[3]);
     }

     /* bind socket to this port number on this machine */

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
//-----------------------------------------------------------------------------   

     /* listen for incoming connection requests */
//-----------------------------------------------------------------------------
     listen(sockfd,10000);
     clilen = sizeof(cli_addr);

     /* accept a new request, create a newsockfd */

     pthread_t threads[NUM_THREADS];
     int rc;

     int arr[NUM_THREADS];

     for(int k=0;k<NUM_THREADS;k++)
     {
     	arr[k]=k+1;
     }

     for(t=0; t<NUM_THREADS; t++){
       //printf("In main: creating thread %ld\n", t);
        int *temp1;
        temp1=&arr[t];
        void *temp;
        temp=(void *) temp1;
       rc = pthread_create(&threads[t], NULL, client, temp);
       if (rc){
          //printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
       }

    }





     

     

     pthread_mutex_init(&lock, NULL);
      pthread_cond_init (&buffer_has_data, NULL);
      pthread_cond_init(&buffer_has_space,NULL);

     while(true)
     {
        int newsockfd;

        pthread_mutex_lock (&lock);
        while(queue_count>=max_count)
        {
            pthread_cond_wait(&buffer_has_space,&lock);
        }
        pthread_mutex_unlock (&lock);



        cout<<"parent"<<endl;
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) 
            {
                perror("Error accept");
            }
        else{

                pthread_mutex_lock (&lock);
                while(queue_count>=max_count)
                {
                    pthread_cond_wait(&buffer_has_space,&lock);
                }
                fd_q.push(newsockfd);
                queue_count++;
                pthread_cond_signal(&buffer_has_data);
                pthread_mutex_unlock (&lock);

            }
     }
     

     return 0; 
}
