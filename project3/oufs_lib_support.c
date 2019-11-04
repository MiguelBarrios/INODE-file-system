/*
 *  Project 3
 *  oufs_lib_support.c
 *
 *  Author: CS3113
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "virtual_disk.h"
#include "oufs_lib_support.h"

extern int debug;

//change dir in oufs:  export OUFS_PWD=/foo

/**
 * Deallocate a single block.
 * - Modify the in-memory copy of the master block
 * - Add the specified block to THE END of the free block linked list
 * - Modify the disk copy of the deallocated block: next_block points to
 *     UNALLOCATED_BLOCK
 *   
 *
 * @param master_block Pointer to a loaded master block.  Changes to the MB will
 *           be made here, but not written to disk
 *
 * @param block_reference Reference to the block that is being deallocated
 *
 */
int oufs_deallocate_block(BLOCK *master_block, BLOCK_REFERENCE block_reference)
{
  BLOCK b;

  if(master_block->content.master.unallocated_front == UNALLOCATED_BLOCK) 
  {
    // No blocks on the free list.  Both pointers point to this block now
    master_block->content.master.unallocated_front = master_block->content.master.unallocated_end =
      block_reference;

  }
  else
  {
      //---link old last block to recently dealocated block, update master block ref free list end to newly dealocated block
      BLOCK oldLastBlock;
      if(virtual_disk_read_block(master_block->content.master.unallocated_end, &oldLastBlock) != 0){
          fprintf(stderr, "Read block error");
          return -1;
      }

      BLOCK_REFERENCE oldRef = master_block->content.master.unallocated_end;

      //Upadate masterblock unallocated end to dealocated block
      master_block->content.master.unallocated_end = block_reference;

      //Link current last block to block being dealocated
      oldLastBlock.next_block = block_reference;

      //write old last black back to disk
      if(virtual_disk_write_block(oldRef, &oldLastBlock) != 0){
          fprintf(stderr, "Read block error");
      }      

  }


  // Update the new end block
  if(virtual_disk_read_block(block_reference, &b) != 0) {
    fprintf(stderr, "deallocate_block: error reading new end block\n");
    return(-1);
  }

  // Change the new end block to point to nowhere
  b.next_block = UNALLOCATED_BLOCK;

  // Write the block back
  if(virtual_disk_write_block(block_reference, &b) != 0) {
    fprintf(stderr, "deallocate_block: error writing new end block\n");
    return(-1);
  }

  return(0);
};


/**
 *  Initialize an inode and a directory block structure as a new directory.
 *  - Inode points to directory block (self_block_reference)
 *  - Inode size = 2 (for . and ..)
 *  - Direcory block: add entries . (self_inode_reference and .. (parent_inode_reference)
 *  -- Set all other entries to UNALLOCATED_INODE
 *
 * @param inode Pointer to inode structure to initialize
 * @param block Pointer to block structure to initialize as a directory
 * @param self_block_reference The block reference to the new directory block
 * @param self_inode_reference The inode reference to the new inode
 * @param parent_inode_reference The inode reference to the parent inode
 */ //calling method
void oufs_init_directory_structures(INODE *inode, BLOCK *block,
				    BLOCK_REFERENCE self_block_reference,
				    INODE_REFERENCE self_inode_reference,
				    INODE_REFERENCE parent_inode_reference)
{

    debug = 0;

    memset(block, 0, BLOCK_SIZE);  //RESET BLOCK

    // TODO
    //Initialize root directory inode
    //oufs_set_inode(INODE *inode, INODE_TYPE type, int n_references, BLOCK_REFERENCE content, int size)
    oufs_set_inode(inode, DIRECTORY_TYPE, 1, self_block_reference, 2); 

    
    //------initialize Root Directory --------    
    block -> next_block = UNALLOCATED_BLOCK;
    block -> content.directory.entry[0].inode_reference = self_inode_reference;
    strcpy(block -> content.directory.entry[0].name, "."); 
    block -> content.directory.entry[1].inode_reference = parent_inode_reference;
    strcpy(block -> content.directory.entry[1].name, "..");

    for(int i = 2; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i){
      block -> content.directory.entry[i].inode_reference = UNALLOCATED_INODE;
    }

}


/**
 *  Given an inode reference, read the inode from the virtual disk.
 *
 *  @param i Inode reference (index into the inode list)
 *  @param inode Pointer to an inode memory structure.  This structure will be
 *                filled in before return)
 *  @return 0 = successfully loaded the inode
 *         -1 = an error has occurred
 *
 */
int oufs_read_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{
  if(debug)
    fprintf(stderr, "\tDEBUG: Fetching inode %d\n", i);

  // Find the address of the inode block and the inode within the block
  BLOCK_REFERENCE block = (i / N_INODES_PER_BLOCK) + 1;
  int index = (i % N_INODES_PER_BLOCK);

  // Load the block that contains the inode
  BLOCK b;
  if(virtual_disk_read_block(block, &b) == 0) {
    // Successfully loaded the block: copy just this inode
    *inode = b.content.inodes.inode[index];
    return(0);
  }
  // Error case
  return(-1);
}


/**
 * Write a single inode to the disk
 *
 * @param i Inode reference index
 * @param inode Pointer to an inode structure
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_write_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{
  if(debug)
    fprintf(stderr, "\tDEBUG: Writing inode %d\n", i);

  //TODO
  //get block, where inode is housed
  BLOCK_REFERENCE blockIndex = (unsigned short)(i / N_INODES_PER_BLOCK) + 1;
  BLOCK block;
  if(virtual_disk_read_block(blockIndex,&block) == -1){
    fprintf(stderr, "Read block error: oufsLibSupport -> oufs_write_inode_by_reference\n");
  }

  //write inode to block   .. check to see if correct
  unsigned short arrayIndex = i % N_INODES_PER_BLOCK;
  block.content.inodes.inode[arrayIndex] = *inode;

  //write block to disk
  if(virtual_disk_write_block(blockIndex, &block) == -1){
    fprintf(stderr, "write to block error oufsLibSupport -> oufs_write_inode_by_reference\n");
  }

  // Success
  return(0);
}

/**
 * Set all of the properties of an inode
 *
 * @param inode Pointer to the inode structure to be initialized
 * @param type Type of inode
 * @param n_references Number of references to this inode
 *          (when first created, will always be 1)
 * @param content Block reference to the block that contains the information within this inode
 * @param size Size of the inode (# of directory entries or size of file in bytes)
 *
 */

void oufs_set_inode(INODE *inode, INODE_TYPE type, int n_references,
		    BLOCK_REFERENCE content, int size)
{
  inode->type = type;
  inode->n_references = n_references;
  inode->content = content;
  inode->size = size;
}


/*
 * Given a valid directory inode, return the inode reference for the sub-item
 * that matches <element_name>
 *
 * @param inode Pointer to a loaded inode structure.  Must be a directory inode
 * @param element_name Name of the directory element to look up
 *
 * @return = INODE_REFERENCE for the sub-item if found; UNALLOCATED_INODE if not found
 */

int oufs_find_directory_element(INODE *inode, char *element_name)         //TODO   check for bugs
{
  if(debug)
    fprintf(stderr,"\tDEBUG: oufs_find_directory_element: %s\n", element_name);

  //returns  if not a valid directory inode
  if(inode -> type != DIRECTORY_TYPE || inode -> content == UNALLOCATED_BLOCK)   
    return UNALLOCATED_INODE;

  BLOCK block;
  if(virtual_disk_read_block(inode -> content, &block) != 0){
    fprintf(stderr, "oufs_lib_support -> oufs_find_directory_element() error");
  }

  for(INODE_REFERENCE i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i)
  {
      DIRECTORY_ENTRY entry = block.content.directory.entry[i];
      if(entry.inode_reference != UNALLOCATED_INODE){
        if(strcmp(element_name, (const char *)&entry.name) == 0){           //TODO check to see if works
          return entry.inode_reference;
        }
      }
  }

  return -1;
}

/**
 *  Given a current working directory and either an absolute or relative path, find both the inode of the
 * file or directory and the inode of the parent directory.  If one or both are not found, then they are
 * set to UNALLOCATED_INODE.
 *
 *  This implementation handles a variety of strange cases, such as consecutive /'s and /'s at the end of
 * of the path (we have to maintain some extra state to make this work properly).
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file/directory to be found
 * @param parent Pointer to the found inode reference for the parent directory
 * @param child ointer to the found node reference for the file or directory specified by path
 * @param local_name String name of the file or directory without any path information
 *             (i.e., name relative to the parent)
 * @return 0 if no errors
 *         -1 if child not found
           -2 no directory in file
 *         -x if an error
 *
 */
int oufs_find_file(char *cwd, char * path, INODE_REFERENCE *parent, INODE_REFERENCE *child,
       char *local_name)
{

  debug = 0;

  //test
  if(strlen(path) == 0){
      if(debug){
     fprintf(stderr,"Path was not provied will find cur directory\n");
   }
     path = cwd;
  }
  //endTest



  INODE_REFERENCE grandparent;
  char full_path[MAX_PATH_LENGTH];

  // Construct an absolute path the file/directory in question
  if(path[0] == '/') {
    strncpy(full_path, path, MAX_PATH_LENGTH-1);
  }else{
    if(strlen(cwd) > 1) {
      strncpy(full_path, cwd, MAX_PATH_LENGTH-1);
      strncat(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-1-strnlen(full_path, MAX_PATH_LENGTH));
    }else{
      strncpy(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-2);
    }
  }

  if(debug) {
    fprintf(stderr, "\tDEBUG: Full path: %s\n", full_path);
  };

  // Start scanning from the root directory
  // Root directory inode
  grandparent = *parent = *child = 0;
  if(debug){
    fprintf(stderr, "\tDEBUG: Start search: %d\n", *parent);
    fprintf(stderr, "\tinitial values grandparent %d, parent %d, child %d\n", grandparent, *parent, *child);
  }

  
  //----------------newCode-------------
  BLOCK block;
  if(virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &block) != 0){
    fprintf(stderr, "oufs_lib_support -> oufs_find_file() ERROR\n");
  }

  unsigned char *inodesTable = block.content.master.inode_allocated_flag;       
  int numAlocatedInodes = 0;
  INODE_REFERENCE allocatedInodeRef[(N_INODES >>3)];

  for(int byte = 0; byte < (N_INODES >>3); ++byte)
  {
      int c = (int)inodesTable[byte];
      for(int bit = 7, compare = 0x80; bit >= 0 && c != 0; --bit)
      {
          if((c & compare) == compare)  //INDOE is allocated
          {
              allocatedInodeRef[numAlocatedInodes] = byte + (7 - bit);
              ++numAlocatedInodes;
          }
          compare = compare >> 1;
      }
  }


  if(debug){
      fprintf(stderr, "Number of Alocated inodes: %d", numAlocatedInodes);
  }
  if(numAlocatedInodes == 0){
      *parent = UNALLOCATED_INODE;
      *child = UNALLOCATED_INODE;
      return -2;
  }



  //-------------End newCode-----------

    // Parse the full path
  char *directory_name;
  directory_name = strtok(full_path, "/");
  while(directory_name != NULL)
  {   // Truncate the name
      if(strlen(directory_name) >= FILE_NAME_SIZE-1){  
        directory_name[FILE_NAME_SIZE - 1] = 0;
      }
      if(debug){
        fprintf(stderr, "\tDEBUG: Directory: %s\n", directory_name);
      }
    
    //-------------TODO-------------------
      //need to find directory inodes
      int found = 0;
      for(int i = 0; i < numAlocatedInodes && found == 0; ++i)
      {
          INODE curInode;
          if((oufs_read_inode_by_reference(allocatedInodeRef[i], &curInode) == 0) && (curInode.type == DIRECTORY_TYPE))
          {
                BLOCK block2;
                if(virtual_disk_read_block(curInode.content, &block2) == 0)
                {
                   

                    for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK && found == 0; ++i)
                    {
                        DIRECTORY_ENTRY entry = block2.content.directory.entry[i];
                        if((entry.inode_reference != UNALLOCATED_INODE) && (strcmp(entry.name, directory_name) == 0))
                        {
                            //Parent directory found
                            if((oufs_read_inode_by_reference(entry.inode_reference,&curInode) == 0 )&& (curInode.type == DIRECTORY_TYPE))
                            {
                                //Directry that we are looking for
                                if(virtual_disk_read_block(curInode.content, &block2) == 0)
                                {
                                    fprintf(stderr, "");
                                    grandparent = *parent;
                                    *parent = entry.inode_reference;
                                    *child = block2.content.directory.entry[0].inode_reference;
                                    found = 1;
                                }

                            }
                        }
                    }
                }else{fprintf(stderr, "Read error \n"); }
          }else{fprintf(stderr, "Read error Error");}
      }

      if(found == 0)
      {
          if(debug)
            fprintf(stderr, "Directory: %s not found!\n", directory_name);
          *child = UNALLOCATED_INODE;
          //*parent = UNALLOCATED_INODE;
          return -1;
      }
      //case where directory not found 
      directory_name = strtok(NULL, "/");  //Gets next token
  }

      // Item found.
  if(*child == UNALLOCATED_INODE) {
    // We went too far - roll back one step ***
    *child = *parent;
    *parent = grandparent;
  }
  if(debug) {
    fprintf(stderr, "\tDEBUG: Found: %d, %d\n", *parent, *child);
  }
  // Success!
  return(0);


} 


/**
 * Return the bit index for the first 0 bit in a byte (starting from 7th bit
 *   and scanning to the right)
 *
 * @param value: a byte
 * @return The bit number of the first 0 in value (starting from the 7th bit
 *         -1 if no zero bit is found
 */

int oufs_find_open_bit(unsigned char value)
{

  int one = value;
  int two =  1<<7;

  for(int i = 7; i >= 0; --i)
  {
     if((one & two) != two){
        return i;
     }

      two = two >> 1;
  }

  // Not found
  return(-1);
}

/**
 *  Allocate a new directory (an inode and block to contain the directory).  This
 *  includes initialization of the new directory.
 *
 * @param parent_reference The inode of the parent directory
 * @return The inode reference of the new directory
 *         UNALLOCATED_INODE if we cannot allocate the directory
 */
int oufs_allocate_new_directory(INODE_REFERENCE parent_reference)
{
  debug = 0;
  BLOCK masterBlock;
  // Read the master block
  if(virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &masterBlock) != 0) {
    // Read error
    return(UNALLOCATED_INODE);
  }

  //find available inode
  int bit = -1;
  int byte = 0;
  for(byte = 0; byte < (N_INODES >> 3); ++byte)
  {
     unsigned char cur = masterBlock.content.master.inode_allocated_flag[byte];
     bit = oufs_find_open_bit(cur);
     if(bit != -1){  //Available inode found
        break;
     }
  }

  if(bit == -1){
    fprintf(stderr, "All inodes are allocated, new directory cannot be allocated\n");
    return UNALLOCATED_INODE;
  }

  //------------updates inode allocation Table-----------
  unsigned char currentlyAllocatedBits = masterBlock.content.master.inode_allocated_flag[byte];
  unsigned char allocatedBit = (128 >>(7 - bit));                                            
  unsigned char updatedBits = currentlyAllocatedBits | allocatedBit;                                             
  masterBlock.content.master.inode_allocated_flag[byte] = updatedBits;
  //fprintf(stderr, "current: %d  open: %d updatedBits %d\n", currentlyAllocatedBits, allocatedBit, updatedBits);

  INODE_REFERENCE newDirectoryInodeRef = (byte * 8) + (7 - bit);   //check to see if corret
  BLOCK_REFERENCE newBlockReference = masterBlock.content.master.unallocated_front;

  //get next avaliable free block
  BLOCK newDirectoryBlock;
  if(virtual_disk_read_block(newBlockReference, &newDirectoryBlock) != 0) {
     fprintf(stderr, "Read error oufs_lib_support -> oufs_allocate_new_directory\n");
  }

  
  //updates free list with new front
  masterBlock.content.master.unallocated_front = newDirectoryBlock.next_block;

  //Master block allocation table and free list are now updated, 
  if(virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &masterBlock) != 0){
      fprintf(stderr, "Write master block error\n");
  }


  //clear directory next block
  newDirectoryBlock.next_block = UNALLOCATED_BLOCK;

  if(debug)
    fprintf(stderr, "Sending child ref: %d parent Ref: %d to oufs_init_directory_structures()\n", newDirectoryInodeRef, parent_reference);
  
  INODE newDirectoryInode;
  oufs_init_directory_structures(&newDirectoryInode, &newDirectoryBlock, newBlockReference, newDirectoryInodeRef, parent_reference);

  //---Test code, will attemt to brign all writing to this statge

  //write new Directory back to disk
  if(virtual_disk_write_block(newBlockReference, &newDirectoryBlock)  == -1){
      fprintf(stderr, "Write to block error\n");
      return -1;
  }

  //write new inode back to file
  if(oufs_write_inode_by_reference(newDirectoryInodeRef  , &newDirectoryInode) != 0) {
      fprintf(stderr, "Write inode by refferenced failed: oufs_lib\n");
      return(-1);  
  }

  return newDirectoryInodeRef;
};
