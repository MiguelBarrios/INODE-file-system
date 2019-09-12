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
char executeCommand(char * input, char * buffer);
int getNumTokens(char * input);
int isValidCommand(char * firstToken);
const char SEPARATORS[] = " \t\n";         // Declare as a global constant

int main(int argc, const char * argv[]) {

    char buffer[128];
    zeroBuffer(buffer);
    printf("type..\n");
    
    
    
    char in_buffer[128];                 // Input buffer from STDIN
    char * args[10];                      // pointers to arg strings
    char ** arg;                         // working pointer that steps through the args
    while(fgets(in_buffer, 129, stdin))
    {
        arg = args;
        *arg++ = strtok(in_buffer,SEPARATORS);   // tokenize input
        while ((*arg++ = strtok(NULL,SEPARATORS)));
        
        
        
        if(isValidCommand(args[0]) != 0)
        {
            printf("Valid command inputed\n");
        }
        else
        {
            printf("invalid command");
        }
        
        
    }
    
    return 0;
}

int isValidCommand(char * firstToken)
{
    if((int)strlen(firstToken) == 1)
    {
        char c = firstToken[0];
        if(c == 'z' || c == 'b' || c == 'B' ||  c == 'h' || c == 'H'
           || c == 'i' || c == 'I' || c == 'f' || c == 'F' || c == 's'
           || c == 'S' || c == 'w' || c == 'r')
        {
            return 1;
        }
    }
    
    return 0;  //invalid command
}

char executeCommand(char * input, char * buffer)
{
    char c = input[0];
    
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
