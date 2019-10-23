/**
 *	Miguel A Barrios Davila
 *  Project 2
 */

#include <stdio.h>
#include <string.h>
#include "storage.h"

// Constants
const int MAX_ARGS = 16;
const int INBUFSIZE = 100;
const int BUFSIZE = 128;
const char SEPARATORS[] = " \t\n";           // token sparators

int main(int argc, char **argv)
{
  // Input character buffer
  char in_buffer[INBUFSIZE];

  // Internal byte buffer
  unsigned char buffer[BUFSIZE];

  // For strtok
  char * args[MAX_ARGS];                     // pointers to arg strings
  char ** arg;                               // working pointer thru args

  // Vars for user input/output
  int i1, i2;
  float f2;
  char c;

  // Clear the buffer
  memset(buffer, 0, BUFSIZE);

  // Open the storage
  STORAGE * storage;
  if(argc == 1) //no Arguments
  {
      storage = init_storage("storage.bin");
  }
  else if(argc == 2) //new name of storage
  {
      storage = init_storage((char*)argv[1]);
  }
  else
  {
      fprintf(stderr, "Invalid number of Argument");
      return -1;
  }

  // Interaction loop: loop until EOF
  while(fgets(in_buffer, INBUFSIZE, stdin) != NULL){

    // Parse the arguments
    arg = args;
    *arg++ = strtok(in_buffer,SEPARATORS);   // tokenize input
    while ((*arg++ = strtok(NULL,SEPARATORS)));

    // Interpret the input
    if(args[0]) {
      // Some command exists
      if(args[1]) {
	// At least 1 argument
	if(args[2]) {
	  // At least 2 arguments
	  if(args[3]) {
	    // 3 or more arguments
	    fprintf(stderr, "Too many arguments\n");
	  }else{
	    // 2 arguments

	    // WRITE from buffer to disk
	    if(strcmp(args[0], "w") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%d", &i2) == 1) {
		// Write the  data to the file
		put_bytes(storage, buffer, i1, i2);
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }
	      
	      // Read from disk to buffer
	    }else if(strcmp(args[0], "r") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%d", &i2) == 1) {
		// Read the data from the file
		get_bytes(storage, buffer, i1, i2);
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }

	      // BYTE to buffer
	    }else if(strcmp(args[0], "b") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%d", &i2) == 1) {
		if(i1 < BUFSIZE)
		  // Copy value
		  buffer[i1] = i2;
		else
		  fprintf(stderr, "Buffer overrun.\n");
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }
		
	      // INTEGER to buffer
	    }else if(strcmp(args[0], "i") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%d", &i2) == 1) {
		if(i1 <= BUFSIZE-sizeof(i2))
		  // Copy value
		  memcpy(&buffer[i1], &i2, sizeof(i2));
		else
		  fprintf(stderr, "Buffer overrun.\n");
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }

	      // INTEGER to buffer
	    }else if(strcmp(args[0], "h") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%x", &i2) == 1) {
		if(i1 < BUFSIZE)
		  // Copy value
		  buffer[i1] = i2;
		else
		  fprintf(stderr, "Buffer overrun.\n");
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }

	      // FLOATING POINT to buffer
	    }else if(strcmp(args[0], "f") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%f", &f2) == 1) {
		if(i1 <= BUFSIZE-sizeof(f2))
		  // Copy value
		  memcpy(&buffer[i1], &f2, sizeof(f2));
		else
		  fprintf(stderr, "Buffer overrun.\n");
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }

	      // Character to memory
	    }else if(strcmp(args[0], "c") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1 &&
		 sscanf(args[2], "%c", &c) == 1) {
		if(i1 < BUFSIZE)
		  // Copy value
		  buffer[i1] = c;
		else
		  fprintf(stderr, "Buffer overrun.\n");
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }

	      // String input
	    }else if(strcmp(args[0], "s") == 0){
	      // Parse arguments
	      if(sscanf(args[1], "%d", &i1) == 1) {
		if(i1 <= BUFSIZE - strlen(args[2]) - 1)
		  // Copy value
		  strcpy((char*) &buffer[i1], args[2]);
		else
		  fprintf(stderr, "Buffer overrun.\n");
	      }else{
		fprintf(stderr, "Bad arguments\n");
	      }

	      // Default case (2 arguments)
	    }else{
	      fprintf(stderr, "Bad command\n");
	    }

	  }
	}else{
	  // 1-argument commands

	  // Extract buffer value, interpret as a float and print
	  if(strcmp(args[0], "F") == 0){
	    // Parse arguments
	    if(sscanf(args[1], "%d", &i1) == 1) {
	      if(i1 <= BUFSIZE-sizeof(i2)) {
		// Copy value
		memcpy(&f2, &buffer[i1], sizeof(f2));
		printf("%f\n", f2);
	      }else
		  fprintf(stderr, "Buffer overrun.\n");
	    }else{
	      fprintf(stderr, "Bad arguments\n");
	    }

	  // Extract buffer value, interpret as an int and print it
	  }else if(strcmp(args[0], "I") == 0){
	    // Parse arguments
	    if(sscanf(args[1], "%d", &i1) == 1) {
	      if(i1 <= BUFSIZE-sizeof(i2)) {
		// Copy value
		memcpy(&i2, &buffer[i1], sizeof(i2));
		printf("%d\n", i2);
	      }else
		  fprintf(stderr, "Buffer overrun.\n");
	      
	    }else{
	      fprintf(stderr, "Bad arguments\n");
	    }

	    // Extract buffer value, interpret as an int and print as a hex value
	  }else if(strcmp(args[0], "H") == 0){
	    // Parse arguments
	    if(sscanf(args[1], "%d", &i1) == 1) {
	      if(i1 <= BUFSIZE - sizeof(i2)) {
		// Copy value
		i2 = buffer[i1];
		printf("%x\n", i2);
	      }else
		  fprintf(stderr, "Buffer overrun.\n");
	    }else{
	      fprintf(stderr, "Bad arguments\n");
	    }
	    
	    // Extract buffer value, interpret as a char and print
	  }else if(strcmp(args[0], "C") == 0){
	    // Parse arguments
	    if(sscanf(args[1], "%d", &i1) == 1) {
	      // Copy value
	      i2 = buffer[i1];
	      printf("%c\n", i2);
	    }else{
	      fprintf(stderr, "Bad arguments\n");
	    }

	    // Extract buffer value, interpret as a byte and print
	  }else if(strcmp(args[0], "B") == 0){
	    // Parse arguments
	    if(sscanf(args[1], "%d", &i1) == 1) {
	      // Copy value
	      i2 = buffer[i1];
	      printf("%d\n", i2);
	    }else{
	      fprintf(stderr, "Bad arguments\n");
	    }

	    // Interpret address as a string
	  }else if(strcmp(args[0], "S") == 0){
	    // Parse arguments
	    if(sscanf(args[1], "%d", &i1) == 1) {
	      int i;
	      // Scan to end of string: we want to make sure that  we don't overrun the buffer
	      for(i = i1; i < BUFSIZE && buffer[i]; ++i);
	      
	      if(i < BUFSIZE) {
		// All okay: display the string
		printf("%s\n", (char*) (&buffer[i1]));
	      }else
		fprintf(stderr, "Buffer overrun.\n");
	    }else{
	      fprintf(stderr, "Bad arguments\n");
	    }
	    
	  }else{
	    // Default case (1 argument)
	    fprintf(stderr, "Bad command\n");
	  }
	};

      }else{
	// Only zero-argument commands
	
	if(strcmp(args[0], "z") == 0) {
	  // Clear the guffer
	  memset(buffer, 0, BUFSIZE);

	  // Print all values in buffer
	}else if(strcmp(args[0], "l") == 0) {
	  for(int i = 0; i < BUFSIZE/16; ++i) {
	    for(int j = 0; j < 16; ++j) {
	      printf("%02x ", buffer[j+i*16]);
	    }
	    printf("\n");
	  }

	}else{
	  fprintf(stderr, "Bad command\n");
	};
      };
    };
  }

  // Clean up
  if(close_storage(storage) < 0) {
    fprintf(stderr, "Error closing file.");
    exit(-1);
  }
  
  return(0);
}

