#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
int* buffer;

int full = 0;
int use = 0;
int count = 0;
int numBuffers = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
// CS537: Parse the new arguments too
void getargs(int *port, int* threads, int* buffers, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
}

void put(int val) {;
  buffer[full] = val;
  full = (full + 1) % numBuffers;
  count++;
 }

int get() {
  int tmp = buffer[use];
  use = (use + 1) % numBuffers;
  count --;
  return tmp;
}
 
void* consumer(void* arg) {
  while(1) {
    pthread_mutex_lock(&mutex);
    while(count == 0) {
      pthread_cond_wait(&fill, &mutex);
    }
    int connfd = get(); // this is where you use "use" for the FD to use;
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    requestHandle(connfd);
    close(connfd);
  }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, threads, buffers, clientlen;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads, &buffers, argc, argv);
    
    buffer = (int*) malloc(buffers*sizeof(int));
    if (buffer == NULL) {
      fprintf(stdout, "Malloc Failed");
      exit(1);
    }
   
    pthread_t cid[threads];
    numBuffers = buffers;
    int i;
   
    for(i = 0; i < threads; i++){
      int rc = pthread_create(&cid[i], NULL, consumer, NULL);
      if(rc != 0) {
        fprintf(stdout, "error creating thread\n");
        exit(0);
      }
    }
    // create a fixed sized pool of worker threads when the web server is first started. 
    // # is specified on the command line. 
    // master thread is then responsible for accepting new http connections over the network
    // and placing the descriptor for this connection into a fixed-size buffer (basically the file descriptor)
    // Master thread should not read from this connection.  
    // # elements in buffer also specified in command line. 
    // Current implementation has a single thread that accpetes a connection and then immediately handles the connection
    // In your web serve,r this thrad should place the conenction descriptor into a fixed sized pool.
    //  and return to accepting more connections. 
    // CS537: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {        
      pthread_mutex_lock(&mutex);
      while(count >= numBuffers) {
        pthread_cond_wait(&empty, &mutex);
      }

      clientlen = sizeof(clientaddr);
      pthread_mutex_unlock(&mutex);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
      pthread_mutex_lock(&mutex);
      put(connfd);
     
      pthread_cond_signal(&fill);
      pthread_mutex_unlock(&mutex);
	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work.
	// 
	//
    // requestHandle(connfd);
    // Close(connfd);
    }

    exit(0);
}


    



