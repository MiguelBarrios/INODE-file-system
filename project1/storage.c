//
//  storage.c
//  project1
//
//  Created by Miguel Barrios on 9/11/19.
//  Copyright © 2019 Miguel Barrios. All rights reserved.
//

#include "storage.h"

STORAGE * init_storage(char * name)
{
    //creates file if it doesn't already exist, opens for reading and writing
    open(name, O_CREAT | O_RDWR);
    return NULL;
}

int close_storage(STORAGE *storage)
{
    return -1;
}

int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
    return 12;
}

int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
    return 7;
}
