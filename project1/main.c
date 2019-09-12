//
//  main.c
//  project1
//
//  Created by Miguel Barrios on 9/11/19.
//  Copyright © 2019 Miguel Barrios. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "storage.h"

void zeroBuffer(char * buffer);
void list(char * buffer);
char integerToChar(void);
char floatToChar(void);
char hexToChar(void);
void tokenize(char * input, char *command, int *location, char * value);
char getCommand(char * input, char * buffer);
int getNumTokens(char * input);


int main(int argc, const char * argv[]) {

    char buffer[128];
    char command;
    int location = -1;
    char value[128];
    value[0] = '\0';


    zeroBuffer(buffer);
    printf("type..\n");
    
    char inputString[129];
    while(fgets(inputString, 129, stdin))
    {
        /*
        command = getCommand(inputString, buffer);
        printf("inputed: %c \n", command);
        inputString[0] = '\0';
         */
    }
    
    return 0;
}

int getNumTokens(char * input)
{
    int numTokens = 0;
    for(int i = 0; i < (int)strlen(input) - 1; ++i)
    {
        if(input[i] == 32)
            ++numTokens;
    }
    return numTokens;
}

char getCommand(char * input, char * buffer)
{
    char c = input[0];
    int numTokens = getNumTokens(input);
    
    if(c == 'l')
        list(buffer);
    else if(c == 'z')
        zeroBuffer(buffer);
    else if(c == 'b')
        return 't';
    else if(c == 'B')
        return 't';
    else if(c == 'h')
        return 't';
    else if(c == 'i')
        return 't';
    else if(c == 'I')
        return 't';
    else if(c == 'f')
        return 't';
    else if(c == 'F')
        return 't';
    else if(c == 's')
        return 't';
    else if(c == 'S')
        return 't';
    else if(c == 'w')
        return 't';
    else if(c == 'r')
        return 't';
    
    return 't';
}

void list(char * buffer)
{
    for(int index = 0; index < 128; index += 16)
    {
        for(int i = index; i < index + 16; ++i)
            printf("%c ", buffer[index]);
        
        printf("\n");
    }
}

char integerToChar()
{
    return 'v';
}

char floatToChar()
{
    return 'k';
}

char hexToChar()
{
    return 'k';
}

void zeroBuffer(char * buffer)
{
    for(int i = 0; i < 128; ++i)
    {
        buffer[i] = '0';
    }
}



/* 16 x 8
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 43 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*/
