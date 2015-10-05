#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>

char info[5][500] ;

void syserr ( char *msg )
{
  perror( msg ) ;
  exit( -1 ) ;
}

void * Connected ( void *x )
{
  int newsockfd, n, i, id ;
  char buffer[255] ;
  char lbuffer[500] ;
  char *token ;
  const char del[2] = " " ;

  token = strtok( ( char* ) x, del ) ;
  id = atoi( token ) ;
  token = strtok( NULL, del ) ;
  newsockfd = atoi( token ) ;

  while( id >= 0 )
  {
    printf( "Waiting for command\n" ) ;
    n = recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
    if ( n < 0 ) syserr( "Error message not received\n" ) ;
    buffer[n] = '\0' ;

    if ( strcmp( buffer, "list" ) == 0 ) 
    {
      for ( i = 0; i <5; i++ )
      {
        if ( strcmp( info[i], "a" ) != 0 ) 
        {
          strcat( lbuffer, info[i] ) ;
        }
      }        
      send( newsockfd, lbuffer, sizeof( lbuffer ), 0 ) ;
    }
    else if ( strcmp( buffer, "update" ) == 0 )
    {
      //update record
      recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
      strcpy( info[id], buffer ) ;
    }
    else 
    {
      //delete record 
      strcpy( info[id], "a" ) ;
      id = -1 ;
    } 
  }
  pthread_exit( NULL ) ;
}

int main ( int argc, char *argv[] )
{
  int sockfd, newsockfd, portno, n, i, id ;
  struct sockaddr_in serv_addr, clt_addr ;
  pid_t child ;
  socklen_t addrlen ;
  char buffer[255] ;
  char sockets[5][10] ;
  pthread_t threads[5] ;
  
  if ( argc != 2 ) portno = 5000 ;
  else portno = atoi ( argv[1] ) ;
  
  sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ;
  if ( sockfd < 0 ) syserr( "Error can't open socket\n" ) ;
  
  memset( &serv_addr, 0, sizeof( serv_addr ) ) ;
  serv_addr.sin_family = AF_INET ;
  serv_addr.sin_addr.s_addr = INADDR_ANY ;
  serv_addr.sin_port = htons( portno ) ;
  
  if ( bind( sockfd, ( struct sockaddr* ) &serv_addr, sizeof( serv_addr ) ) < 0 ) 
    syserr( "Error can't bind\n" ) ;
  
  listen ( sockfd, 5 ) ;

  id = 0 ; 

  for (i = 0; i <= 5; i++ ) strcpy ( info[i], "a" ) ;
  
  for( ;; )
  {
    printf( "Wait on port %i \n", portno ) ;
    
    addrlen = sizeof( clt_addr ) ;
    newsockfd = accept( sockfd, ( struct sockaddr* ) &clt_addr, &addrlen ) ;
    if ( newsockfd < 0 ) syserr( "Error can't accept\n" ) ;

    recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;     
    strcpy( info[id], buffer ) ;
    
    sprintf( sockets[id], "%i %i", id, newsockfd ) ;
    if ( pthread_create( &threads[id], NULL, Connected, ( void* ) sockets[id] ) != 0 )
      syserr( "Pthread\n" ) ; 

    id++ ;
  }  
}
  
