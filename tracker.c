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

void syserr ( char *msg )
{
  perror( msg ) ;
  exit( -1 ) ;
}

int main ( int argc, char *argv[] )
{
  int sockfd, newsockfd, portno, connected, n, i, id ;
  struct sockaddr_in serv_addr, clt_addr ;
  pid_t child ;
  socklen_t addrlen ;
  char buffer[255] ;
  char info[5][500] ;
  
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
    id ++ ;

    child = fork() ;
    if ( child < 0 ) syserr( "Fork error" ) ;
    else if ( child == 0 ) 
    {
      recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
     
      strcpy( info[id], buffer ) ;
      connected = 1 ;
      
      while( connected )
      {
        printf( "Waiting for command\n" ) ;
        n = recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
        if ( n < 0 ) syserr( "Error message not received\n" ) ;
        buffer[n] = '\0' ;

        if ( strcmp( buffer, "list" ) == 0 ) 
        {
          for ( i = 0; i <=5; i++ )
          {
            if ( info[i] == "a" ) break ;
            printf( "Files at %i\n%s", id, info[i] ) ;          
          }
        }
        else
        {
          connected = 0 ;
          info[i][0] = '\0' ;
        }
      }
    }  
     
  }  
}
  
