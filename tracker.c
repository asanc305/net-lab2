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

void addList ( char *files, char *ip, int id )
{
  char *token ;
  char buffer[200] ;
  const char del[2] = " " ;
  char port[10] ;
  
  token = strtok( files, del ) ;
  strcpy( port, token )  ;
  
  token = strtok( NULL, del ) ;
  while ( token != NULL )
  {
    sprintf( buffer, "%s %s:%s\n", token, ip, port) ;
    strcat( info[id], buffer) ;
    token = strtok( NULL, del ) ; 
  }
}
void * Connected ( void *x )
{
  int newsockfd, n, i, id, ct ;
  char buffer[255] ;
  char lbuffer[500] ;
  char filename[15] ;
  char *token ;
  char *ip ;
  char port[6] ;
  char info2[5][500] ;
  const char del[2] = "\n" ;

  token = strtok( ( char* ) x, " " ) ;
  id = atoi( token ) ;
  token = strtok( NULL, " " ) ;
  
  strcpy( port, token ) ;
  
  newsockfd = atoi( port ) ;
  token = strtok( NULL, " " ) ;
  ip = token ;

  while( id >= 0 )
  {
    printf( "Waiting for command\n" ) ;
    n = recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
    if ( n < 0 ) syserr( "Error message not received\n" ) ;
    buffer[n] = '\0' ;

    if ( strcmp( buffer, "list" ) == 0 ) 
    {
      ct = 0 ;
      lbuffer[0] = '\0' ;
      for ( i = 0; i <5; i++ )
      {
        if ( strlen( info[i] ) != 0 ) 
        {
          strcpy( info2[i], info[i] ) ;
          token = strtok( info2[i] , del) ;
          while ( token != NULL ) 
          {
            sprintf( buffer, "[%i] %s\n", ct, token ) ;
            strcat( lbuffer, buffer ) ;
            ct++ ;
            token = strtok( NULL, del) ;
          }
        }
      }         
      send( newsockfd, lbuffer, sizeof( lbuffer ), 0 ) ;  
    }
    else if ( strcmp( buffer, "update" ) == 0 )
    {
      //update record
      recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
      token = strtok( buffer, " " ) ;
      strcpy( filename, token ) ;
      
      token = strtok( NULL, " " ) ;
      sprintf( lbuffer, "%s %s:%s\n", filename, ip, token ) ;
      printf("len %zd\n", strlen(lbuffer) ) ;
      strcat( info[id], lbuffer ) ;
    }
    else 
    {
      //delete record 
      info[id][0] = '\0' ;
      id = -1 ;
    } 
  }
  pthread_exit( NULL ) ;
}

int main ( int argc, char *argv[] )
{
  int sockfd, newsockfd, portno, n, i, id ;
  char * ip ;
  struct sockaddr_in serv_addr, clt_addr ;
  socklen_t addrlen ;
  char buffer[255] ;
  char sockets[5][20] ;
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

  for (i = 0; i <= 5; i++ ) info[i][0] = '\0' ;
  
  for( ;; )
  {
    printf( "Wait on port %i \n", portno ) ;
    
    addrlen = sizeof( clt_addr ) ;
    newsockfd = accept( sockfd, ( struct sockaddr* ) &clt_addr, &addrlen ) ;
    if ( newsockfd < 0 ) syserr( "Error can't accept\n" ) ;

    ip = inet_ntoa( clt_addr.sin_addr ) ;
    
    recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
    addList( buffer, ip, id ) ;  
    
    sprintf( sockets[id], "%i %i %s", id, newsockfd, ip ) ;
    if ( pthread_create( &threads[id], NULL, Connected, ( void* ) sockets[id] ) != 0 )
      syserr( "Pthread\n" ) ; 

    id++ ;
  }  
}
  
