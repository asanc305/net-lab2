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
#include <sys/stat.h>
#include <sys/stat.h>


void syserr ( char *msg )
{
  perror( msg ) ;
  exit( -1 ) ;
}

void printlist ( char *list )
{
  DIR *dp ;
  struct dirent *ep ;
  dp = opendir ( "./" ) ;
 
  strcat(list, "\n") ;
  if ( dp != NULL )
  {
	  while ( ep = readdir ( dp ) )
	  {
	    if ( ( &(ep->d_name)[0])[0] != '.' )
	    {
	      strcat(list, ep->d_name) ;
	      strcat(list, "\n") ;
	    }
	  }
	  ( void ) closedir ( dp ) ;
  }
  else syserr( "Couldn't open the dir\n" ) ;
}

int connection ( char * args[] )
{
  int sockfd, portno, n ;
  char list[255] ; 
  struct hostent* server ;
  struct sockaddr_in serv_addr ;

  server = gethostbyname( args[1] ) ;
  if(!server) syserr( "Error no such host\n" ) ; 
  
  if ( args[2] == NULL ) portno = 5000 ;
  else portno = atoi( args[2] ) ;

  sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
  if ( sockfd < 0 ) syserr( "can't open socket" ) ;
  printf( "create socket...\n" ) ;

  memset( &serv_addr, 0, sizeof( serv_addr ) ) ;
  serv_addr.sin_family = AF_INET ;
  serv_addr.sin_addr = *( ( struct in_addr* )server->h_addr ) ;
  serv_addr.sin_port = htons( portno ) ;

  if ( connect( sockfd, ( struct sockaddr* )&serv_addr, sizeof( serv_addr )) < 0 )
    syserr( "can't connect to server" ) ;
  printf( "connect...\n" ) ;
  
  list[0] = '\0' ;
  if ( args[3] == NULL ) strcat( list, "6000" ) ;
  else strcat( list, args[3] ) ;
  printlist( list ) ;
  
  send( sockfd, list, sizeof( list ), 0 ) ;
  
  return sockfd ;
}

int main ( int argc, char *argv[] )
{
  int sockfd ;
  struct hostent* server ;
  
  server = gethostbyname(argv[1]) ;
  
  sockfd = connection(argv) ;

}