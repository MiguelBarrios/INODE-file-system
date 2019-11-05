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
int debug = 1;

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
  // ?   do the inodes have to be linked to the blocks at this point
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
  debug = 0;

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

      //CHECK:  parent and child are of type directory
      if(parentInode.type != DIRECTORY_TYPE || childInode.type != DIRECTORY_TYPE){
          fprintf(stderr, "parent or child not of type directory\n");
          return -1;
      }

      //CHECK:  is child a directory of parent
      int isSubdirectory = 0;
      for(int i = 0; i < parentInode.size && isSubdirectory == 0; ++i)
      {
          INODE_REFERENCE entryRef = parentBlock.content.directory.entry[i].inode_reference;
          if(entryRef == childInodeRef)
              isSubdirectory = 1;
      }

      if(isSubdirectory == 0){
          fprintf(stderr, "Child is not a directory of parent\n");
          //return -1;        ?????
      }

      
      qsort(&childBlock.content.directory.entry[0], N_DIRECTORY_ENTRIES_PER_BLOCK, sizeof(DIRECTORY_ENTRY), inode_compare_to);


      for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
      {
          DIRECTORY_ENTRY entry = childBlock.content.directory.entry[i];
          if(entry.inode_reference != UNALLOCATED_INODE)
          {
             printf("%s/\n", entry.name);
          }
      }

      return 0;
  }
  else
  {
      // Did not find the specified file/directory
      fprintf(stderr, "Not found\n");
      if(debug)
          fprintf(stderr, "\tDEBUG: (%d)\n", ret);
  }


  //-1 if child not found, 0 if parent and child was found, -2 if no directory in file, -x error
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
      //MASTER_BLOCK masterblock = block.content.master;

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

  fprintf(stderr, "new Dircotry was not allocated\n");

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
   //TODO: complet implementation, given directory exists
        get directory inode
        get directory block
        remove name from parente directory entry
        set directory entry to unalocated inode
        clear directory block   
        set directory block next block to unalocated block
        add block to the end of free list
        update inode allocation table
        set directory inode: type = unsused type, content = UNALLOCATED_BLOCK
        decrement parent inode size

 */
int oufs_rmdir(char *cwd, char *path)
{

  fprintf(stderr, "Attempting to remove Directory %s", path);
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

  fprintf(stderr, "Parent Inode ref %d child inode ref %d\n", parentInodeRef, childInodeRef);

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
    fprintf(stderr, "Read error");
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
  fprintf(stderr, "num: %d\n", num);

  char updatedValue = masterBlock.content.master.inode_allocated_flag[byte] & num;
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
          fprintf(stderr, "Entry removed\n");
          break;
      }
      
  }

  //Write parent block back to disk
  if(virtual_disk_write_block(parentInode.content, &parentBlock) != 0){
      fprintf(stderr, "Write to block error\n");
  }


  parentInode.size = parentInode.size - 1;

  if(oufs_write_inode_by_reference(parentInodeRef, &parentInode) != 0){
      fprintf(stderr, "Write inode by ref error\n");
  }


  // Success
  return(0);
}
