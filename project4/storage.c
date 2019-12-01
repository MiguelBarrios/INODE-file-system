//  Miguel A Barrios Davila
//  project1
//

#include "storage.h"
STORAGE * init_storage(char * name, char *pipe_name_base)
{
    //creates file if it doesn't already exist, opens for reading and writing
    STORAGE  *storage = malloc(sizeof(STORAGE));
    storage -> fd = open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    
    if(storage -> fd == -1)
    {
        perror("Unable to open file");  //prints out error, and relevent info about the error
        return NULL;
    }
    
    return storage;
}

int close_storage(STORAGE *storage)
{  
    int ret = close(storage -> fd);
    if(ret != 0)   //close returns zero if succesful
    {
        perror("Close error");
        return -1;
    }
    free(storage);
    return 0;
}

int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
    int offset = lseek(storage -> fd, 0, SEEK_SET); //Start of file
    offset = lseek(storage -> fd, location, SEEK_CUR);
    
    
    int outcome = read(storage -> fd, &buf[0], len);
    return outcome;
}

int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
    
    int offset = lseek(storage -> fd, 0, SEEK_SET); //Start of file
    offset = lseek(storage -> fd, location, SEEK_CUR);
    
    
    int outcome = write(storage -> fd, &buf[0], len);

    if(outcome != len){
        perror("Write error");
        return -1;
    }
    
    return outcome;
}
