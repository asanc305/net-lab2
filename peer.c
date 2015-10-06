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
#include <pthread.h>

void syserr ( char *msg )
{
  perror( msg ) ;
  exit( -1 ) ;
}

void printlist ( char *port, int sockfd )
{
  DIR *dp ;
  struct dirent *ep ;
  dp = opendir ( "./" ) ;
  char list [255] ;
 
  list[0] = '\0' ;
  strcat( list, port ) ;
  strcat( list, " " ) ;
  
  if ( dp != NULL )
  {
	  while ( ep = readdir ( dp ) )
	  {
	    if ( ( &(ep->d_name)[0])[0] != '.' )
	    {
	      strcat(list, ep->d_name) ;
	      strcat(list, " ") ;
	    }
	  }
	  ( void ) closedir ( dp ) ;
  }
  else syserr( "Couldn't open the dir\n" ) ;
  
  list[strlen ( list ) - 1] = '\0' ;
  send( sockfd, list, sizeof( list ), 0 ) ;
}

int connectTo ( char * ip, int portno )
{
  int sockfd ;
  struct hostent* server ;
  struct sockaddr_in serv_addr ;

  server = gethostbyname( ip ) ;
  if(!server) syserr( "Error no such host\n" ) ; 

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
  
  return sockfd ;
}

void * server ( void *x )
{
  int sockfd, newsockfd, portno, n, out ;
  struct sockaddr_in serv_addr, clt_addr ;
  pid_t child ;
  struct stat f_stat ;
  socklen_t addrlen ;
  char buffer[255] ;
  
  portno = atoi( ( char* ) x ) ;
  
  sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ;
  if ( sockfd < 0 ) syserr( "Error can't open socket\n" ) ;
  
  memset( &serv_addr, 0, sizeof( serv_addr ) ) ;
  serv_addr.sin_family = AF_INET ;
  serv_addr.sin_addr.s_addr = INADDR_ANY ;
  serv_addr.sin_port = htons( portno ) ;
  
  if ( bind( sockfd, ( struct sockaddr* ) &serv_addr, sizeof( serv_addr ) ) < 0 ) 
    syserr( "Error can't bind\n" ) ;
  
  listen ( sockfd, 5 ) ;

  for(;;) 
  {
	  addrlen = sizeof(clt_addr); 
	  newsockfd = accept(sockfd, (struct sockaddr*)&clt_addr, &addrlen);
	  if(newsockfd < 0) syserr("can't accept"); 

	  child = fork() ;
	  if (child < 0) syserr("Fork error") ;
	  else if (child == 0)
	  {
	    n = recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
	    out = open(buffer, O_RDONLY) ;
	    
	    fstat(out, &f_stat) ; 
		  sprintf(buffer, "%jd", f_stat.st_size) ;
		  n = send(newsockfd, buffer, sizeof(buffer), 0) ;
      
		  n = sendfile(newsockfd, out, NULL, f_stat.st_size) ;
      close(out) ;
		}
  }
}

int main ( int argc, char *argv[] )
{
  int sockfd, cTracker, n, len, total, serverport, portno, newsockfd ;
  char buffer[255] ;
  char lbuffer[500] ;
  char lbuffer2[500] ;
  char port[6] ;
  char filenum[5] ;
  char filename[15] ;
  char addr[15] ;
  char *token ;  
  char *line ;
  FILE *in ;
  const char del[2] = " " ;
  pthread_t threads[0] ;
  
  if ( argv[2] == NULL ) serverport = 5000 ;
  else serverport = atoi( argv[2] ) ;
  sockfd = connectTo( argv[1], serverport ) ;

  if ( argc == 4 ) strcpy( port, argv[3] )  ;
  else strcpy( port, "6000" ); 
  
  printlist( port, sockfd ) ;
  
  if ( pthread_create( &threads[0], NULL, server, ( void* ) port ) != 0 )
      syserr( "Pthread\n" ) ; 

  cTracker = 1 ;
  while( cTracker )
  {
    printf("PLEASE ENTER MESSAGE: ") ;
    fgets( buffer, 255, stdin ) ;
    n = strlen( buffer ) ;
    if(n>0 && buffer[n-1] == '\n') buffer[n-1] = '\0' ;
   
    token = strtok(buffer, del) ;

    if ( strcmp( token, "list" ) == 0 )
    {
      strcpy( buffer, "list" ) ;
      send( sockfd, buffer, sizeof( buffer ), 0 ) ;
      
      recv( sockfd, lbuffer, sizeof(lbuffer), 0) ;
      printf( "%s\n", lbuffer ) ;
    }
    else if ( strcmp( token, "download" ) == 0 )
    {
      //download file
      strcpy( lbuffer2, lbuffer) ;
      
      token = strtok( NULL, del) ;
      sprintf( filenum, "[%s]", token ) ;
      
      line = strtok( lbuffer2, "\n" ) ;
      while ( line != NULL )
      {
        if ( strstr( line, filenum ) != NULL ) break ;
        line = strtok( NULL, "\n") ;
      }
      
      if ( line != NULL ) 
      {
        token = strtok( line, del ) ;
        
        token = strtok( NULL, del ) ;
        strcpy( filename, token ) ;
        
        token = strtok( NULL, ":") ;
        strcpy( addr, token ) ;
        
        token = strtok( NULL, del ) ;
        portno = atoi( token ) ;
        
        newsockfd = connectTo( addr, portno ) ;
        strcpy( buffer, filename) ;
        n = send( newsockfd, buffer, sizeof( buffer ), 0 ) ;
        
        n = recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
        len = atoi( buffer ) ;
        
        in = fopen(filename, "w") ;
        if ( in != NULL ) 
        {
          total = 0 ;
          while (len > total)
          {
            n = recv( newsockfd, buffer, sizeof( buffer ), 0 ) ;
            fwrite( buffer, sizeof(char), n, in ) ; 
            total += n ;
          }
          
          //check 
          if ( total == len ) 
          {
            printf( "Successful !\n" ) ;  
            strcpy( buffer, "update" ) ; 
            send( sockfd, buffer, sizeof( buffer ), 0 ) ;
            
            strcpy( buffer, filename) ;
            send( sockfd, buffer, sizeof( buffer ), 0 ) ;
          }
          else
          {
            printf( "Error receiving file. Please try again.\n" ) ;
            remove( filename ) ;
          }
          fclose(in) ;
        }
        else printf( "Error creating file\n" ) ;
        
        
      }
      else printf( "Bad file number\n" ) ; 
      
    }
    else if ( strcmp( token, "exit" ) == 0 )
    {
      send( sockfd, buffer, sizeof( buffer ), 0 ) ; 
      cTracker = 0 ; 
    }
    else printf( "Error command does not exist!\n" ) ;
  }
 
}
