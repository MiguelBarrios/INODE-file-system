/**
 *  Project 3
 *  oufs_lib.c
 *
 *  Author: CS3113
 *
 */

#include "oufs_lib.h"
#include "oufs_lib_support.h"
#include "virtual_disk.h"

// Yes ... a global variable
int debug = 0;

// Translate inode types to descriptive strings
const char *INODE_TYPE_NAME[] = {"UNUSED", "DIRECTORY", "FILE"};

/**
 Read the OUFS_PWD, OUFS_DISK, OUFS_PIPE_NAME_BASE environment
 variables copy their values into cwd, disk_name an pipe_name_base.  If these
 environment variables are not set, then reasonable defaults are
 given.

 @param cwd String buffer in which to place the OUFS current working directory.
 @param disk_name String buffer in which to place file name of the virtual disk.
 @param pipe_name_base String buffer in which to place the base name of the
            named pipes for communication to the server.

 PROVIDED
 */
void oufs_get_environment(char *cwd, char *disk_name,
			  char *pipe_name_base)
{
  // Current working directory for the OUFS
  char *str = getenv("OUFS_PWD");
  if(str == NULL) {
    // Provide default
    strcpy(cwd, "/");
  }else{
    // Exists
    strncpy(cwd, str, MAX_PATH_LENGTH-1);
  }

  // Virtual disk location
  str = getenv("OUFS_DISK");
  if(str == NULL) {
    // Default
    strcpy(disk_name, "vdisk1");
  }else{
    // Exists: copy
    strncpy(disk_name, str, MAX_PATH_LENGTH-1);
  }

  // Pipe name base
  str = getenv("OUFS_PIPE_NAME_BASE");
  if(str == NULL) {
    // Default
    strcpy(pipe_name_base, "pipe");
  }else{
    // Exists: copy
    strncpy(pipe_name_base, str, MAX_PATH_LENGTH-1);
  }

}

/**
 * Completely format the virtual disk (including creation of the space).
 *
 * NOTE: this function attaches to the virtual disk at the beginning and
 *  detaches after the format is complete.
 *	
 * - Zero out all blocks on the disk.					COMPLETE
 * - Initialize the master block: 
 		- mark inode 0 as allocated 					COMPLETE
 		- Initialize the linked list of free blocks     COMPLETE
 * - Initialize root directory inode 
 * - Initialize the root directory in block ROOT_DIRECTORY_BLOCK
 *
 * @return 0 if no errors
 *         -x if an error has occurred.
 *
 */

int oufs_format_disk(char  *virtual_disk_name, char *pipe_name_base)
{
  // Attach to the virtual disk
  if(virtual_disk_attach(virtual_disk_name, pipe_name_base) != 0) {
    return(-1);
  }

  BLOCK block;

  //------------ Zero out the block -----------
  memset(&block, 0, BLOCK_SIZE);
  for(int i = 0; i < N_BLOCKS; ++i) {
    if(virtual_disk_write_block(i, &block) < 0) {
      return(-2);
    }
  }

  //-------- Master Block initialization ---------

  block.next_block = UNALLOCATED_BLOCK;
  block.content.master.inode_allocated_flag[0] = 0x80;  //mark inode 0(Root dir inode) as allocated

  //Initialize the linked list of free blocks
  BLOCK_REFERENCE front = 6;  //6 becuase the root directory will already be alocated
  BLOCK_REFERENCE end = 127;
  block.content.master.unallocated_front = front; //first free block after inode blocks
  block.content.master.unallocated_end = end; //last block

  //writing master block to disk
  if(virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &block) == -1){
    fprintf(stderr, "writing master block to disk error: oufs_lib -> oufs_format_disk\n");
  }

  //initializes remainder of free list
  BLOCK link;
  for(BLOCK_REFERENCE cur = 6, next = 7; cur < N_BLOCKS - 1; ++next, ++cur)
  {
      link.next_block = next;
      
      //write block to disk
      if(virtual_disk_write_block(cur, &link) == -1){
        fprintf(stderr, "write to block error: oufs_lib -> oufs_format_disk\n");
      }
  }

  //Writes Last link  in Free list to disk
  link.next_block = UNALLOCATED_BLOCK;
  if(virtual_disk_write_block(N_BLOCKS - 1, &link) == -1){
        fprintf(stderr, "write to block error\n");
  }
  //--------- End Master Block initialization --------


  //-------- Inodes initialization -------------------  
  BLOCK inodeBlock;
  inodeBlock.next_block = UNALLOCATED_BLOCK;

  INODE inode;
  inode.type = UNUSED_TYPE;
  inode.n_references = 0;
  inode.content = UNALLOCATED_BLOCK;
  inode.size = 0;

  for(int i = 0; i < N_INODES_PER_BLOCK; ++i){
  	 inodeBlock.content.inodes.inode[i] = inode;
  }

  for(BLOCK_REFERENCE i = 1; i <= N_INODE_BLOCKS; ++i){
  	if(virtual_disk_write_block(i, &inodeBlock) == -1){
        fprintf(stderr, "Writing inode block error oufs_lib -> oufs_format_disk()\n");
 	 }
  }

  //----------- init Root directory inode and block -------
  INODE rootInode;
  oufs_init_directory_structures(&rootInode, &block, ROOT_DIRECTORY_BLOCK,
				 ROOT_DIRECTORY_INODE, ROOT_DIRECTORY_INODE);

  if(virtual_disk_write_block(ROOT_DIRECTORY_BLOCK, &block)  == -1){
      fprintf(stderr, "Write to block error: oufs_lib_support -> oufs_init_directory_structures\n");
  }

  // Write the results to the disk
  if(oufs_write_inode_by_reference(ROOT_DIRECTORY_INODE  , &rootInode) != 0) {
	    return(-3);  fprintf(stderr, "Write inode by refferenced failed: oufs_lib\n");
  }

  // Done
  virtual_disk_detach();
 
  return(0);
}

/*
 * Compare two inodes for sorting, handling the
 *  cases where the inodes are not valid
 *
 * @param e1 Pointer to a directory entry
 * @param e2 Pointer to a directory entry
 * @return -1 if e1 comes before e2 (or if e1 is the only valid one)
 * @return  0 if equal (or if both are invalid)
 * @return  1 if e1 comes after e2 (or if e2 is the only valid one)
 *
 * Note: this function is useful for qsort()
 */
static int inode_compare_to(const void *d1, const void *d2)
{
  // Type casting from generic to DIRECTORY_ENTRY*
  DIRECTORY_ENTRY* e1 = (DIRECTORY_ENTRY*) d1;
  DIRECTORY_ENTRY* e2 = (DIRECTORY_ENTRY*) d2;

  if(e1 -> inode_reference == UNALLOCATED_INODE && e2 -> inode_reference == UNALLOCATED_INODE){
      return 0;
  }

  if(e1 -> inode_reference == UNALLOCATED_INODE && e2 -> inode_reference != UNALLOCATED_INODE){
      return -1;
  }
  else if(e1 -> inode_reference != UNALLOCATED_INODE && e2 -> inode_reference == UNALLOCATED_INODE){
      return 1;
  }

  if(strcmp(e1 -> name, e2 -> name) == 0)
    return 0;
  else if(strcmp(e1 -> name, e2 -> name) < 0)
    return -1;
  else
    return 1;

}


/**
 * Print out the specified file (if it exists) or the contents of the 
 *   specified directory (if it exists)
 *
 * If a directory is listed, then the valid contents are printed in sorted order
 *   (as defined by strcmp()), one per line.  We know that a directory entry is
 *   valid if the inode_reference is not UNALLOCATED_INODE.
 *   Hint: qsort() will do to sort for you.  You just have to provide a compareTo()
 *   function (just like in Java!)
 *   Note: if an entry is a directory itself, then its name must be followed by "/"
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *                          OUPUT MUST BE SORTED
 */

int oufs_list(char *cwd, char *path)
{

  INODE_REFERENCE parentInodeRef;
  INODE_REFERENCE childInodeRef;

  // Look up the inodes for the parent and child
  int ret = oufs_find_file(cwd, path, &parentInodeRef, &childInodeRef, NULL);

  // Did we find the specified file?
  if(ret == 0 && childInodeRef != UNALLOCATED_INODE) 
  {
      INODE parentInode;
      INODE childInode;
      BLOCK parentBlock;
      BLOCK childBlock;

      //Read all block and inodes
      if(oufs_read_inode_by_reference(parentInodeRef, &parentInode) != 0 ||
         oufs_read_inode_by_reference(childInodeRef, &childInode) != 0 ||
         virtual_disk_read_block(parentInode.content, &parentBlock) != 0 ||
         virtual_disk_read_block(childInode.content, &childBlock) != 0)
      {
          fprintf(stderr, "Read error\n");
          return -1;
      }

      if(debug)
      {
          fprintf(stderr, "\tDEBUG: Child found (type=%s).\n",  INODE_TYPE_NAME[childInode.type]);
          fprintf(stderr, "\tDEBUG: Parent found (type=%s).\n",  INODE_TYPE_NAME[parentInode.type]);
      }
      
      
      //Display file name Only
      if(childInode.type == FILE_TYPE)
      {
          for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
          {
              DIRECTORY_ENTRY entry = parentBlock.content.directory.entry[i];
              if(entry.inode_reference == childInodeRef)
              {
                  printf("%s\n", entry.name);
                  return 0;
              }
          }
      }

      qsort(&childBlock.content.directory.entry[0], N_DIRECTORY_ENTRIES_PER_BLOCK, sizeof(DIRECTORY_ENTRY), inode_compare_to);

      //Display content of a directory
      for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
      {
          DIRECTORY_ENTRY entry = childBlock.content.directory.entry[i];
          if(entry.inode_reference != UNALLOCATED_INODE)
          {
              INODE temp;
              if(oufs_read_inode_by_reference(entry.inode_reference, &temp) != 0){
                  fprintf(stderr, "Read inode by ref error\n");
                  return -1;
              }

              //Print Directory name
              if(temp.type == DIRECTORY_TYPE){
                  printf("%s/\n", entry.name);
              }//Print File name
              else if(temp.type == FILE_TYPE){
                  printf("%s\n", entry.name);
              }
          }
      }

      return 0;  //All contents have been displayed
  }
  else
  {
      // Did not find the specified file/directory
      fprintf(stderr, "Not found\n");
      if(debug)
          fprintf(stderr, "\tDEBUG: (%d)\n", ret);
  }


  //-1 if child not found, 0 if parent and child was found
  return -1;

}


///////////////////////////////////
/**
 * Make a new directory
 *
 * To be successful:
 *  - the parent must exist and be a directory
 *  - the parent must have space for the new directory
 *  - the child must not exist
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_mkdir(char *cwd, char *path)             //Casese not yet working:  if dir foo and bar exist in parent foo/bar wont init
{
  debug = 0;
  INODE_REFERENCE parrentInodeRef;
  INODE_REFERENCE childInodeRef;

  // Name of a directory within another directory
  char local_name[MAX_PATH_LENGTH];
  int ret;

  // Attempt to find the specified directory
  if((ret = oufs_find_file(cwd, path, &parrentInodeRef, &childInodeRef, local_name)) < -1) {
    if(debug)
      fprintf(stderr, "oufs_mkdir(): ret = %d\n", ret);
    return(-1);
  };

  INODE parentInode;
  INODE childInode;

  //returns if parent inode is not a directory
  if(oufs_read_inode_by_reference(parrentInodeRef, &parentInode) != 0 || parentInode.type != DIRECTORY_TYPE){
      fprintf(stderr, "read inode erro\n");
      return -1;
  }

  //returns if parant does not have space for the new directory
  if(parentInode.size >= N_DIRECTORY_ENTRIES_PER_BLOCK){
      fprintf(stderr, "new directory cannot be added, Parent directory does is full\n");
      return -1;
  }

  //-----All conditions met, make new directory----------

  BLOCK block;
  if(virtual_disk_read_block(parrentInodeRef, &block) == 0)
  {

      INODE_REFERENCE newDirectoryInodeRef = oufs_allocate_new_directory(parrentInodeRef);

      if(newDirectoryInodeRef == UNALLOCATED_INODE)
          return -1;

      //---------------update parent directory to include new directory 
      //gets the name of the new directory
      char *fullpath;
      char *directory_name;
      fullpath = strtok(path, "/");
      while(fullpath != NULL){
          directory_name = fullpath;
          fullpath = strtok(NULL, "/");  //Gets next token
      }
      

      BLOCK parrentDirectory;
      if(virtual_disk_read_block(parentInode.content, &parrentDirectory)!= 0){
          fprintf(stderr, "read inode erro\n");
          return -1;
      }

      DIRECTORY_ENTRY newEntry;
      strcpy(newEntry.name, directory_name);
      newEntry.inode_reference = newDirectoryInodeRef;


      for(int i = 2; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
      {
          if(parrentDirectory.content.directory.entry[i].inode_reference == UNALLOCATED_INODE)
          {
              parrentDirectory.content.directory.entry[i] = newEntry;
              break;
          }
      }

      //Update parent inode size
      parentInode.size = parentInode.size + 1;

      //done with parent inode
      if(oufs_write_inode_by_reference(parrentInodeRef, &parentInode) != 0){
          fprintf(stderr, "Write inode by ref error\n");
      }

      //done with parent block
      if(virtual_disk_write_block(parentInode.content, &parrentDirectory) != 0){
          fprintf(stderr, "Write block error\n");
      }

      return 0;
  }

  if(debug)
      fprintf(stderr, "New directory not allocated\n");
  return -1;  
}

/**
 * Remove a directory
 *
 * To be successul:
 *  - The directory must exist and must be empty
 *  - The directory must not be . or ..
 *  - The directory must not be /
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Abslute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_rmdir(char *cwd, char *path)
{

  if(strcmp(path, ".") == 0 || strcmp(path, "..") == 0 || strcmp(path, "/") == 0)
  {
      fprintf(stderr, "Illegal to delete . or .. or /\n");
      return -1;
  }

  INODE_REFERENCE parentInodeRef;
  INODE_REFERENCE childInodeRef;
  char local_name[MAX_PATH_LENGTH];

  // Try to find the inode of the child
  if(oufs_find_file(cwd, path, &parentInodeRef, &childInodeRef, local_name) != 0) {
    fprintf(stderr, "rmdir failed: Directory %s not found\n", path);
    return -1;
  }

  //-------------Read in all Blocks---------------
  BLOCK masterBlock;
  BLOCK parentBlock;
  BLOCK childBlock;
  INODE parentInode;
  INODE childInode;
  if(oufs_read_inode_by_reference(parentInodeRef, &parentInode) != 0 ||
      oufs_read_inode_by_reference(childInodeRef, &childInode) != 0 ||
      virtual_disk_read_block(parentInode.content, &parentBlock) != 0 ||
      virtual_disk_read_block(childInode.content, &childBlock) != 0 ||
      virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &masterBlock))
  {
    fprintf(stderr, "Read error -> oufs_rmdir()");
    return -1;
  }

  if(childInode.size > 2){
    fprintf(stderr, "Directory not empty\n");
    return -1;
  }

  //update masterblock free list
  oufs_deallocate_block(&masterBlock, childInode.content);

  //Update child inode
  childInode.type = UNUSED_TYPE;
  childInode.content = UNALLOCATED_BLOCK;
  childInode.size = 0;

  //write inode back to disk
  if(oufs_write_inode_by_reference(childInodeRef, &childInode) != 0){
      fprintf(stderr, "Write inode error");
  }

  //update inode allocation table
  int byte = childInodeRef / 8;
  int bit = 7 - (childInodeRef % 8);  
  char num = ~(1 <<bit);
  char updatedValue = masterBlock.content.master.inode_allocated_flag[byte] & num;

  //Udate inode allocation table with correct value
  masterBlock.content.master.inode_allocated_flag[byte] = updatedValue;

  //write master block back to disk
  if(virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &masterBlock) != 0){
      fprintf(stderr, "Write master block to disk error\n");
  }


  //remove dealocated dictenatry from parent directory
  for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
  {
      DIRECTORY_ENTRY entry = parentBlock.content.directory.entry[i];
      if(entry.inode_reference == childInodeRef)
      {
          parentBlock.content.directory.entry[i].inode_reference = UNALLOCATED_INODE;
          strcpy(parentBlock.content.directory.entry[i].name, "");
          break;
      }
  }

  //Write parent block back to disk
  if(virtual_disk_write_block(parentInode.content, &parentBlock) != 0){
      fprintf(stderr, "Write to block error\n");
  }

  //update parent inode size
  parentInode.size = parentInode.size - 1;

  if(oufs_write_inode_by_reference(parentInodeRef, &parentInode) != 0){
      fprintf(stderr, "Write inode by ref error\n");
  }


  // Success
  return(0);
}


/*********************************************************************/
// Project 4
/**
 * Open a file
 * - mode = "r": the file must exist; offset is set to 0
 * - mode = "w": the file may or may not exist;
 *                 - if it does not exist, it is created 
 *                 - if it does exist, then the file is truncated
 *                       (size=0 and data blocks deallocated);
 *                 offset = 0 and size = 0
 * - mode = "a": the file may or may not exist
 *                 - if it does not exist, it is created 
 *                 offset = size
 *
 * @param cwd Absolute path for the current working directory
 * @param path Relative or absolute path for the file in question
 * @param mode String: one of "r", "w" or "a"
 *                 (note: only the first character matters here)
 * @return Pointer to a new OUFILE structure if success
 *         NULL if error
 */
OUFILE* oufs_fopen(char *cwd, char *path, char *mode)
{
  debug = 0;
  INODE_REFERENCE parent;
  INODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INODE inode;
  int ret;

  // Check for valid mode
  if(mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') {
    fprintf(stderr, "fopen(): bad mode.\n");
    return(NULL);
  };


  // Try to find the inode of the child
  if((ret = oufs_find_file(cwd, path, &parent, &child, local_name)) < -1) {
    if(debug)
      fprintf(stderr, "oufs_fopen(%d)\n", ret);
    return(NULL);
  }

  fprintf(stderr, "ret from oufs_find_file: %d\n", ret);
  
  if(parent == UNALLOCATED_INODE) {
    fprintf(stderr, "Parent directory not found.\n");
    return(NULL);
  }

  fprintf(stderr, "parentRef: %d, childRef: %d\n", parent, child);

  INODE parentInode;
  oufs_read_inode_by_reference(parent, &parentInode);

  BLOCK parentDirectory;
  if(virtual_disk_read_block(parentInode.content, &parentDirectory) != 0){
  		fprintf(stderr, "Read error");
  }

  //todo get file name if absolute path is provided
  char *directory_name;
  directory_name = strtok(path, "/");
  char *fileName;
  while(directory_name != NULL)
  {
      fileName = directory_name;
      directory_name = strtok(NULL, "/");  //Gets next token
  }

  for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
  {
  		DIRECTORY_ENTRY entry = parentDirectory.content.directory.entry[i];
  		//fprintf(stderr, "Comparing %s : %s, = %d\n", fileName, entry.name, strcmp(fileName, entry.name));
		if(strcmp(fileName, entry.name) == 0)
		{
		//	fprintf(stderr, "inner loop 1\n");
 			  INODE entryInode;
  			if(oufs_read_inode_by_reference(entry.inode_reference, &entryInode) == 0 && entryInode.type == FILE_TYPE)
  			{  	 
  				//File already exists
  				OUFILE *file = malloc(sizeof(OUFILE));
  	    	file -> inode_reference = entry.inode_reference;
 		 		  file -> mode = *mode;

  				if(mode[0] == 'r')
  				{
  					file -> offset = 0;
  					return file;
  				}
  				else if(mode[0] == 'a')
  				{
  					file -> offset = entryInode.size;
            printFileInfo(file);
  					return file;
  				}
          else if(mode[0] == 'w')
          {
            //FILE already exist and is of size zero
            if(entryInode.content == UNALLOCATED_BLOCK){
                file -> offset = entryInode.size = 0;
                return file;
            }


            //File already exist but is not of size 0
            BLOCK master;
            BLOCK lastBlock;
            BLOCK_REFERENCE firstBlockInFileRef = entryInode.content;
            BLOCK_REFERENCE lastBlockInFileRef = entryInode.content;

            //This part assumes the block are allocated  correctly
            virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master);
            virtual_disk_read_block(lastBlockInFileRef, &lastBlock);
            
            
            while(lastBlock.next_block != UNALLOCATED_BLOCK){
                lastBlockInFileRef = lastBlock.next_block;
                virtual_disk_read_block(lastBlockInFileRef, &lastBlock);
            }

            //update Free list
            if(master.content.master.unallocated_front == master.content.master.unallocated_end){
                master.content.master.unallocated_front = master.content.master.unallocated_end = firstBlockInFileRef;
            }
            else{
              master.content.master.unallocated_end = firstBlockInFileRef;
              master.content.master.unallocated_end = lastBlockInFileRef;
            }

            fprintf(stderr, "unallocated_front: %d , unallocated_end: %d",  firstBlockInFileRef, lastBlockInFileRef);


            entryInode.content = UNALLOCATED_BLOCK;
            file -> offset = entryInode.size = 0;

            virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &master);
            oufs_write_inode_by_reference(entry.inode_reference, &entryInode);

            return file;
          }  	     	 	
			}
		}  		
  }

  //file does not exist
  if(mode[0] == 'a' || mode[0] == 'w')
  {
  		OUFILE *file = malloc(sizeof(OUFILE));

  		file -> inode_reference = oufs_create_file(parent, fileName);
  		if(file -> inode_reference == UNALLOCATED_INODE)
  			 return NULL;


 		file -> mode = *mode;
  	file -> offset = 0;


  	//TODO: deallocate reamainng blocks on create: mode w
    printFileInfo(file);
  	return(file);
  }

  return NULL;
  
};

void printFileInfo(OUFILE* file)
{
   fprintf(stderr, "  ------File Info--------\n");
   fprintf(stderr, "  File inode_reference: %d\n", file -> inode_reference);
   fprintf(stderr, "  Mode: %c\n", file -> mode);
   fprintf(stderr, "  Offset: %d\n", file -> offset);
   fprintf(stderr, "  n_data_block: %d\n", file -> n_data_blocks);
   fprintf(stderr, "  File References: ");
   for(int i = 0; i < file -> n_data_blocks; ++i)
   {
      fprintf(stderr, "-> %d", file -> block_reference_cache[i]);
   }
   fprintf(stderr,"\n  ---------------------------------\n");
}

/**
 *  Close a file
 *   Deallocates the OUFILE structure
 *
 * @param fp Pointer to the OUFILE structure
 */
     
void oufs_fclose(OUFILE *fp) {
  fp->inode_reference = UNALLOCATED_INODE;
  free(fp);
}



/*
 * Write bytes to an open file.
 * - Allocate new data blocks, as necessary
 * - Can allocate up to MAX_BLOCKS_IN_FILE, at which point, no more bytes may be written
 * - file offset will always match file size; both will be updated as bytes are written
 *
 * @param fp OUFILE pointer (must be opened for w or a)
 * @param buf Character buffer of bytes to write
 * @param len Number of bytes to write
 * @return The number of written bytes
 *          0 if file is full and no more bytes can be written
 *         -x if an error
 * 
 */
int oufs_fwrite(OUFILE *fp, unsigned char * buf, int len)
{

  fprintf(stderr, "-------------------------------------------\n");

  if(fp->mode == 'r') {
    fprintf(stderr, "Can't write to read-only file");
    return(0);
  }
  if(debug)
    fprintf(stderr, "-------\noufs_fwrite(%d)\n", len);
    
  INODE fileInode;
  if(oufs_read_inode_by_reference(fp->inode_reference, &fileInode) != 0 || fileInode.type != FILE_TYPE) {
    return(-1);
  }

  // Compute the index for the last block in the file + the first free byte within the block
  BLOCK masterBlock;
  if(virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &masterBlock) != 0){
  		fprintf(stderr, "read error");
  }

  //------Start------
  BLOCK currentBlock;
  BLOCK_REFERENCE currentBlockRef = fileInode.content;
  
  int current_blocks = (fp->offset + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
  int used_bytes_in_last_block = fp->offset % DATA_BLOCK_SIZE;
  int free_bytes_in_last_block = DATA_BLOCK_SIZE - used_bytes_in_last_block;
  int len_written = 0;

  fprintf(stderr, "###Number of bytes to write: %d\n", len);
  fprintf(stderr, "File offset: %d, ", fp-> offset);
  fprintf(stderr, "Current Block: %d \n", current_blocks);
  fprintf(stderr, "used_bytes_in_last_block %d, ", used_bytes_in_last_block);
  fprintf(stderr, "free_bytes_in_last_block %d\n", free_bytes_in_last_block);
  

  if(fileInode.content == UNALLOCATED_BLOCK)
  {
		  currentBlockRef =  oufs_allocate_new_block(&masterBlock, &currentBlock);

  	 	if(currentBlockRef == UNALLOCATED_BLOCK){
  			fprintf(stderr, "File is full\n");
  			return 0;
		}
		fileInode.content = currentBlockRef;
  }
  else
  {

      virtual_disk_read_block(currentBlockRef, &currentBlock);

  		for(int i = 0; i < current_blocks; ++i)
  		{
        if(currentBlock.next_block == UNALLOCATED_BLOCK)
        {


            if(free_bytes_in_last_block > 0)
            {
                break;
            }
            if(masterBlock.content.master.unallocated_front == UNALLOCATED_BLOCK){
                return 0;
            }
            currentBlock.next_block = masterBlock.content.master.unallocated_front;
            virtual_disk_write_block(currentBlockRef, &currentBlock);
            currentBlockRef = oufs_allocate_new_block(&masterBlock, &currentBlock);

        }
        else
        {
           currentBlockRef = currentBlock.next_block;
           virtual_disk_read_block(currentBlock.next_block, &currentBlock);
        }

  		}
  		fprintf(stderr, "\nfinal blockRef %d\n", currentBlockRef);

  }

  int curIndex = 0;

  while(len_written < len)
  {
  		int writeSize = 0 ;
  		if(len < free_bytes_in_last_block)
  			writeSize = len;
  		else if(len >= free_bytes_in_last_block)
  			writeSize = free_bytes_in_last_block;

      if(writeSize > len - len_written)
        writeSize = len - len_written;


  	  fprintf(stderr, "Write size: %d\n", writeSize);

    	  fprintf(stderr, "    #Before Write to file#\n");
  		  fprintf(stderr, "    current block %d\n", currentBlockRef);
    		fprintf(stderr, "    used_bytes_in_last_block %d\n", used_bytes_in_last_block);
    		fprintf(stderr, "    writeSize %d\n", writeSize);
    		fprintf(stderr, "    curIndex %d \n", curIndex);
  		
  		
  		memcpy(&currentBlock.content.data.data[used_bytes_in_last_block], &buf[curIndex], writeSize);


  		//update current info
  		len_written += writeSize;
  		free_bytes_in_last_block = free_bytes_in_last_block - writeSize;
  		used_bytes_in_last_block = 0;
  		fp -> offset = fp -> offset + writeSize;

      fprintf(stderr, "         $After write to File$\n");
      fprintf(stderr, "         len written %d\n", len_written);


  		if(len_written == len)
  		{
  			currentBlock.next_block = UNALLOCATED_BLOCK;
  			virtual_disk_write_block(currentBlockRef, &currentBlock);
  			break;
  		}
  		else if(len_written < len)
  		{

  			if(masterBlock.content.master.unallocated_front != UNALLOCATED_BLOCK)
  			{
  				fprintf(stderr, "update: currentBlock -> next block to %d  ", masterBlock.content.master.unallocated_front);
  				currentBlock.next_block = masterBlock.content.master.unallocated_front;
  				if(virtual_disk_write_block(currentBlockRef, &currentBlock) == 0){
              fprintf(stderr, "Block %d wrote back to disk\n", currentBlockRef);
          }

  				currentBlockRef = oufs_allocate_new_block(&masterBlock, &currentBlock);
  			}

  			if(currentBlockRef == UNALLOCATED_BLOCK){
  	 			fprintf(stderr, "File is full\n");
  	 			return 0;
  	 		}

  	 		used_bytes_in_last_block = 0;
  	 		curIndex = len_written;
  	 		free_bytes_in_last_block = DATA_BLOCK_SIZE;
  	 		virtual_disk_read_block(currentBlockRef, &currentBlock);
  	 		//fprintf(stderr, "Additional Block added to file\n");
  		}
  }

  fileInode.size = fileInode.size + len_written;
  oufs_write_inode_by_reference(fp -> inode_reference, &fileInode);

  virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &masterBlock);


  // Done
  return(len_written);
}


/*
 * Read a sequence of bytes from an open file.
 * - offset is the current position within the file, and will never be larger than size
 * - offset will be updated with each read operation
 *
 * @param fp OUFILE pointer (must be opened for r)
 * @param buf Character buffer to place the bytes into
 * @param len Number of bytes to read at max
 * @return The number of bytes read
 *         0 if offset is at size
 *         -x if an error
 * 
 */

int oufs_fread(OUFILE *fp, unsigned char * buf, int len)
{
  // Check open mode
  if(fp->mode != 'r') {
    fprintf(stderr, "Can't read from a write-only file");
    return(0);
  }
  if(debug)
    fprintf(stderr, "\n-------\noufs_fread(%d)\n", len);
    
  INODE fileInode;
  BLOCK currentBlock;
  if(oufs_read_inode_by_reference(fp->inode_reference, &fileInode) != 0) {
    return(-1);
  }

  // Compute the current block and offset within the block
  int current_block = fp->offset / DATA_BLOCK_SIZE;
  int byte_offset_in_block = fp->offset % DATA_BLOCK_SIZE;
  int len_read = 0;
  int end_of_file = fileInode.size;
  int len_left = fileInode.size - (fp -> offset);
  int curIndex = 0;
  BLOCK_REFERENCE nextBlockRef = fileInode.content;

  /*
  fprintf(stderr, "fileInode.size %d\n", fileInode.size);
  fprintf(stderr, "currentBlock %d\n", current_block);
  fprintf(stderr, "byte_offset_in_block %d\n", byte_offset_in_block);
  fprintf(stderr, "end_of_file %d\n", end_of_file);
  fprintf(stderr, "len_left %d\n", len_left);
  fprintf(stderr, "nextBlockRef %d\n", nextBlockRef);
  fprintf(stderr, "###################\n");
	*/

  while(len_read < end_of_file)
  {
  		//TODO:  IF FILE SIZE IS ABOVE 1000 logic must be added
  		if(fp -> offset == end_of_file)
  			return 0;

  		if(virtual_disk_read_block(nextBlockRef, &currentBlock) != 0){
  			return -1;
  		}

  		int readAmount = MIN(DATA_BLOCK_SIZE, len_left);

  		if((readAmount + len_read) / 1000 > 0)
  		{
  			fprintf(stderr, "Buffer will be full\n");
  			readAmount = (readAmount + len_read) % 1000;
  		}

  		memcpy(&buf[curIndex], &currentBlock.content.data.data[0], readAmount);

  		curIndex += readAmount;
  		len_left -= readAmount;
  		len_read += readAmount;

  		nextBlockRef = currentBlock.next_block;

  		fprintf(stderr, "next block ref %d\n", nextBlockRef);
  }

  fp ->offset = len_read;

  // Done
  return(len_read);
}


/**
 * Remove a file
 *
 * Full implementation:
 * - Remove the directory entry
 * - Decrement inode.n_references
 * - If n_references == 0, then deallocate the contents and the inode
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file to be removed
 * @return 0 if success
 *         -x if error
 *
 */

int oufs_remove(char *cwd, char *path)
{

  fprintf(stderr, "---------------------Remove method--------------------\n");
  INODE_REFERENCE parentInodeRef;
  INODE_REFERENCE childInodeRef;
  char local_name[MAX_PATH_LENGTH];
  INODE inode_child;
  INODE inode_parent;

  // Try to find the inode of the child
  if(oufs_find_file(cwd, path, &parentInodeRef, &childInodeRef, local_name) < -1) {
    return(-3);
  };

  fprintf(stderr, "parentInodeRef: %d, childInodeRef: %d \n", parentInodeRef, childInodeRef);
  
  if(childInodeRef == UNALLOCATED_INODE) {
    fprintf(stderr, "File not found\n");
    return(-1);
  }
  // Get the inode
  if(oufs_read_inode_by_reference(childInodeRef, &inode_child) != 0){
    return(-4);
  }

  if(oufs_read_inode_by_reference(parentInodeRef, &inode_parent) != 0){
    return (-4);
  }


  fprintf(stderr, "initial parent inode size %d, ref %d\n", inode_parent.size, parentInodeRef);
  // Is it a file?
  if(inode_child.type != FILE_TYPE) {
    // Not a file
    fprintf(stderr, "Not a file\n");
    return(-2);
  }

  //-------------File Exists-------------

  BLOCK masterBlock;
  virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &masterBlock);

  BLOCK fileBlock;
  BLOCK directoryBlock;
  virtual_disk_read_block(inode_child.content, &fileBlock);
  virtual_disk_read_block(inode_parent.content, &directoryBlock);

  //----------------------------update free list-----------------------------
  //links end of free list to begining of file blocks
  BLOCK temp;
  virtual_disk_read_block(masterBlock.content.master.unallocated_end, &temp);
  temp.next_block = inode_child.content;
  virtual_disk_write_block(masterBlock.content.master.unallocated_end, &temp);

  BLOCK_REFERENCE cur = inode_child.content;
  while(fileBlock.next_block != UNALLOCATED_BLOCK)
  {
      cur = fileBlock.next_block;
      if(virtual_disk_read_block(fileBlock.next_block, &fileBlock) != 0)
      {
          fprintf(stderr, "Read error\n");
      }
  }

  //Update end of free list with end of file block
  masterBlock.content.master.unallocated_end = cur;
  fprintf(stderr, "unalocated end %d\n",cur);

  //Free list is now updated

  //-------------update inode allocation table
  int byte = childInodeRef / 8;
  int bit = 7 - (childInodeRef % 8);  
  char num = ~(1 <<bit);
  char updatedValue = masterBlock.content.master.inode_allocated_flag[byte] & num;

  //Udate inode allocation table with correct value
  masterBlock.content.master.inode_allocated_flag[byte] = updatedValue;

  //------------write master block back to disk--------------
  if(virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &masterBlock) != 0){
      fprintf(stderr, "Write master block to disk error\n");
  }

  //----------Update child inode------------
  inode_child.type = UNUSED_TYPE;
  inode_child.content = UNALLOCATED_BLOCK;
  inode_child.size = 0;

  //write inode back to disk
  if(oufs_write_inode_by_reference(childInodeRef, &inode_child) != 0){
      fprintf(stderr, "Write inode error");
  }

  //-----------------remove file from parent directory-----------------

  
  for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
  {
      DIRECTORY_ENTRY entry = directoryBlock.content.directory.entry[i];
      if(entry.inode_reference == childInodeRef)
      {
          directoryBlock.content.directory.entry[i].inode_reference = UNALLOCATED_INODE;
          strcpy(directoryBlock.content.directory.entry[i].name, "");
          break;
      }
  }
  

  //Write parent block back to disk
  if(virtual_disk_write_block(inode_parent.content, &directoryBlock) != 0){
      fprintf(stderr, "Write to block error\n");
  }

  //update parent inode size
  inode_parent.size -= 1;

  if(oufs_write_inode_by_reference(parentInodeRef, &inode_parent) != 0){
      fprintf(stderr, "Write inode by ref error\n");
  }


  // Success
  return(0);
};


/**
 * Create a hard link to a specified file
 *
 * Full implemenation:
 * - Add the new directory entry
 * - Increment inode.n_references
 *
 * @param cwd Absolute path for the current working directory
 * @param path_src Absolute or relative path of the existing file to be linked
 * @param path_dst Absolute or relative path of the new file inode to be linked
 * @return 0 if success
 *         -x if error
 * 
 */
int oufs_link(char *cwd, char *path_src, char *path_dst)
{
  INODE_REFERENCE parent_src;
  INODE_REFERENCE child_src;
  INODE_REFERENCE parent_dst;
  INODE_REFERENCE child_dst;
  char local_name[MAX_PATH_LENGTH];
  char local_name_bogus[MAX_PATH_LENGTH];
  INODE inode_src;
  INODE inode_dst;
  BLOCK block;

  // Try to find the inodes
  if(oufs_find_file(cwd, path_src, &parent_src, &child_src, local_name_bogus) < -1) {
    return(-5);
  }
  if(oufs_find_file(cwd, path_dst, &parent_dst, &child_dst, local_name) < -1) {
    return(-6);
  }

  // SRC must exist
  if(child_src == UNALLOCATED_INODE) {
    fprintf(stderr, "Source not found\n");
    return(-1);
  }

  // DST must not exist, but its parent must exist
  if(parent_dst == UNALLOCATED_INODE) {
    fprintf(stderr, "Destination parent does not exist.\n");
    return(-2);
  }
  if(child_dst != UNALLOCATED_INODE) {
    fprintf(stderr, "Destination already exists.\n");
    return(-3);
  }

  // Get the inode of the dst parent
  if(oufs_read_inode_by_reference(parent_dst, &inode_dst) != 0) {
    return(-7);
  }

  if(inode_dst.type != DIRECTORY_TYPE) {
    fprintf(stderr, "Destination parent must be a directory.");
  }
  // There must be space in the directory
  if(inode_dst.size == N_DIRECTORY_ENTRIES_PER_BLOCK) {
    fprintf(stderr, "No space in destination parent.\n");
    return(-4);
  }


  // TODO
  return -1;
}




