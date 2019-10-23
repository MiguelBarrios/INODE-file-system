/*
    Miguel A Barrios Davila  
    Project 2
*/
#include <string.h>
#include "storage_remote.h"
#include "storage_common.h"
#include "comm.h"

/**
   initialize the storage

   Open the two fds (in and out), wait for an init message, initialize the local storage
 */
STORAGE * init_storage(char * name)       
{
  // Create space for the STORAGE object
  STORAGE *s = malloc(sizeof(STORAGE));

  s -> fd_to_storage = open(PIPE_NAME_TO_STORAGE, O_WRONLY); 
  fprintf(stderr,"write to server Pipe OPENED\n");

  s -> fd_from_storage = open(PIPE_NAME_FROM_STORAGE, O_RDONLY); 
  fprintf(stderr, "read from server pipe opened\n");

  HEADER header;
  header.type = INIT_CONNECTION;
  header.len_message = strlen(name) + 1;
  
  //Sends connection message
  if(write(s -> fd_to_storage, &header, sizeof(header)) != sizeof(header)){
      fprintf(stderr, "Write Error\n");
  }

  //sends name of file
  if(header.len_message > 1){
     write(s -> fd_to_storage, name, strlen(name) + 1);  //sends name of storage
  }

  //recives Acknowledgment message
  if(read(s -> fd_from_storage, &header, sizeof(header)) != (int)sizeof(header)){
    fprintf(stderr, "ACKNOWLEDGE Not Recived\n");
  }

  return s;
};

/**
   Shut down the connection

   Tell the server to shut down
 */
int close_storage(STORAGE *storage)
{
  // Create the shutdown message
  HEADER header;
  header.type = SHUTDOWN;
  if(write(storage -> fd_to_storage, &header, sizeof(header)) != sizeof(header)){
      fprintf(stderr, "Write Error\n");
  }
  
  // Free the storage struction
  free(storage);

  // Done
  return(0);
}

/**
   read bytes from the storage
 */
int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
  HEADER header;
  header.type = READ_REQUEST;
  header.len_message = 0;
  header.location = location;
  header.len_buffer = len;
  if(write(storage -> fd_to_storage, &header, sizeof(header)) != sizeof(header)){
      fprintf(stderr, "Write error\n");
  }

  //Recive data message
  if(read(storage-> fd_from_storage, &header, sizeof(header)) != sizeof(header)){
      fprintf(stderr, "Read Error\n");
  }
  int numIncomingBytes = header.len_buffer;

  if(read(storage -> fd_from_storage, &buf[0], numIncomingBytes) != sizeof(numIncomingBytes)){
      fprintf(stderr, "Read error\n");
  }


  // Success
  return(numIncomingBytes);
};


/**
   Write bytes to the storae

   Send the write request message + the data
 */
int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{

  unsigned char tmpbuffer[len];
  unsigned char *ptr = tmpbuffer;

  for(int i = 0;i < len; ++i)
  {
    tmpbuffer[i] = buf[i];
  }

  for(int i = 0; i < len; ++i)
  {
    fprintf(stderr, "%c ", tmpbuffer[i]);
  }
  
  HEADER header;
  header.type = WRITE_REQUEST;
  header.len_message = len;
  header.location = location;
  header.len_buffer = len;
  if(write(storage -> fd_to_storage, &header, sizeof(header)) != sizeof(header)){
      fprintf(stderr, "Write Error\n");
  }

  int bytesWritten = write(storage -> fd_to_storage, ptr, len); 

  if(read(storage -> fd_from_storage, &header, sizeof(header)) != sizeof(header)){
    printf(stderr, "Read Error\n");
  }

  //Todo finish writing
  // Success
  return(bytesWritten);
};



































