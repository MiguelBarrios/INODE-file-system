/*
    Miguel A Barrios Davila  
    Project 2
*/

#include "storage.h"
#include "comm.h"
#include "storage_common.h"
#include <string.h>

#define BUFSIZE 200

int main(int argc, char** argv)
{
  unsigned char buffer[BUFSIZE];
  HEADER header;
  HEADER header_out;
  int ret;
  STORAGE *storage;

  int fd_out;
  int fd_in;  

  memset(buffer, 0, BUFSIZE);

  // Loop forever (break out with SHUTDOWN)
  while(1)
  {
  		printf("Waiting for connection with client...\n");

  		//Opens the named pipes
    	fd_in = open(PIPE_NAME_TO_STORAGE, O_RDONLY);     fprintf(stderr, "read from client open\n");
      fd_out = open(PIPE_NAME_FROM_STORAGE, O_WRONLY);   fprintf(stderr, "write to pipe opened\n");

   		//Waits for an Init_Connection Message
   		if(read(fd_in,&header,sizeof(header)) != sizeof(header)){
        fprintf(stderr, "Read Error\n");
      }
   		
    	if(header.type == INIT_CONNECTION)
    	{
          //Initialize's Local Storage
          if(header.len_message == 0)
          {
             storage = init_storage("storage.bin");
          }
          else
          {
              char storageName[128];         //String buffer
              memset(storageName, 0, 128);   //Fill with zeros 

              read(fd_in, storageName, header.len_message);  
              storage = init_storage(storageName);
          }
      		
      		///Acknoledgment
      		header_out.type = ACKNOWLEDGE;
      		header_out.len_message = 0;
      		header_out.location = -1;     
      		header_out.len_buffer = -1;    
			
          if(write(fd_out, &header_out, sizeof(header_out)) != sizeof(header_out)){
            fprintf(stderr, "Write Error\n");
          }

          fprintf(stderr, "Whating for Communication message\n");

      		//Loops untill SHUTDOWN IS RECIEVED
      		while(read(fd_in,&header,sizeof(header)) != -1  && header.type != SHUTDOWN)
      		{
      			   if(header.type == WRITE_REQUEST)
    			     {
        		       int location = header.location;
        			     int length = header.len_buffer;

        			     unsigned char *ptr = &buffer[0];
        			     if(read(fd_in, ptr, length) != length){
                      fprintf(stderr, "Read Error\n");
                   }

        			     int bytesWritten = put_bytes(storage, buffer, location, length);

        			     header_out.type = ACKNOWLEDGE;
        			     header_out.len_message = 0;
        			     header_out.location = -1;      
        			     header_out.len_buffer = -1;
                   
                   if(write(fd_out, &header_out, sizeof(header_out)) != sizeof(header_out)){
                      fprintf(stderr, "Write Error\n");
                   }

                   memset(buffer, 0 , BUFSIZE);
    			     }
    			     else if(header.type == READ_REQUEST)  //Read request not working
    			     {
        		    	 int location = header.location;
        			     int len = header.len_buffer;
        			     fprintf(stderr, "location: %d length: %d\n", location, len);

        			     unsigned char *ptr = &buffer[0];

        			     int numBytesRead = get_bytes(storage, ptr, location, len);

        			     header_out.type = DATA;
        			     header_out.len_message = numBytesRead;        
        			     header_out.location = -1;       // These value is not used for SHUTDOWN messages,
        			     header_out.len_buffer = numBytesRead;

        			     //Sends Data message
        			     if(write(fd_out, &header_out, sizeof(header_out)) != sizeof(header_out)){
                      fprintf(stderr, "Write Error\n");
                   }

                   //Sends Data
        			     if(write(fd_out, &buffer[0], numBytesRead) != numBytesRead){
                      fprintf(stderr, "Write Error\n");
                   }
                   memset(buffer, 0, BUFSIZE);

    			     }
      			   

      		}// Communication Loop end

          //Shutdown message was recived
          header_out.type = ACKNOWLEDGE;
          header_out.len_message = 0;
          header_out.location = -1;
          header_out.len_buffer = -1;

          if(write(fd_out, &header_out, sizeof(header_out)) != sizeof(header_out)){
              fprintf(stderr, "Write Error\n");
          }

          sleep(1);
          fprintf(stderr, "Closing connection\n");
          close(fd_in);
          close(fd_out);
          close_storage(storage);
          
    	}  //Initial connnection end

  } //endless Connection loop

  // Should never reach here
  return(0);
}
