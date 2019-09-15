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

int writeByte(int location, char * value);
int writeHexadecimal(int location, char * value);
void writeInteger(int location, char * value);
int writeFloat(int location, char * value);
int writeString(int location, char * value);

void readByte(int location);
void readHexadecimal(int location);
void readInteger(int location);
void readFloat(int location);
void readString(int location);

int getIntegerValueOf(char * value);
void zeroBuffer(void);
void list(void);
int isValidCommand(char * command, int numArgs);
char * toHex(char c);
int toDecimal(char * c);

int hexToInt(char * value);
int getLocation(char * location);


//Global Variables
unsigned char BUFFER[128];
const char SEPARATORS[] = " \t\n";
const char HEXADECIMAL[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f', '\0'};
const char DECIMAL[] = {'0','1','2','3','4','5','6','7','8','9', '\0'};

int main(int argc, const char * argv[]) {
    
    zeroBuffer();
    
    char in_buffer[128];                 // Input buffer from STDIN
    char * args[10];                     // pointers to arg strings
    char ** arg;                         // working pointer that steps through the args
    
    while(fgets(in_buffer, 129, stdin))
    {
        arg = args;
        *arg++ = strtok(in_buffer,SEPARATORS);   // tokenize input
        while ((*arg++ = strtok(NULL,SEPARATORS)));
        
        int numberOfArguments = 0;
        for(int i = 0;i < 4 && args[i] != NULL; ++i, ++numberOfArguments){} //gets number of arguments
        
        if(numberOfArguments <= 3 && isValidCommand(args[0], numberOfArguments) != -1)
        {
            char commmand = args[0][0];
            switch (commmand)
            {
                case 'l': list();
                    break;
                case 'z': zeroBuffer();
                    break;
                case 'b': writeByte(getLocation(args[1]), args[2]);  //TODO:  Check for errors when a value greater than 1 char is inputed
                    break;
                case 'h': writeHexadecimal(getLocation(args[1]), args[2]);      //Implement
                    break;
                case 'i': writeInteger(getLocation(args[1]), args[2]);
                    break;
                case 'f': writeFloat(getLocation(args[1]), args[2]);            //Implement
                    break;
                case 's': writeString(getLocation(args[1]), args[2]);           //Implement
                    break;
                case 'B': readByte(getLocation(args[1]));
                    break;
                case 'H': readHexadecimal(getLocation(args[1]));
                    break;
                case 'I': readInteger(getLocation(args[1]));
                    break;
                case 'F': readFloat(getLocation(args[1]));                      //Implement
                    break;
                case 'S': readString(getLocation(args[1]));                     //Implement
                    break;
                    //Write offset cmd w
                    //read part of the file into buffer cmd r
            }
        }
        else
        {
            fprintf(stderr, "Invalid  Argument\n");
        }
    }
    return 0;
}

//-----------------------------------------Completed and Working----------------------------------------

void list() //l
{
    for(int index = 0; index < 128; index += 16)
    {
        for(int i = index; i < index + 16; ++i)
        {
            printf("%s ", toHex(BUFFER[i]));
        }
        printf("\n");
    }
}

void zeroBuffer() // z
{
    for(int i = 0; i < 128; ++i)
    {
        BUFFER[i] = 0;
    }
}

char * toHex(char c)
{
    char one = HEXADECIMAL[c / 16];
    char two = HEXADECIMAL[c % 16];
    
    char * output = malloc(3);
    output[0] = one;
    output[1] = two;
    return output;
}

int isValidCommand(char * command, int numArgs)
{
    if(command == NULL)
        return -1;
    
    
    int tokenSize = (int)strlen(command);
    if(tokenSize == 1)
    {
        char c = command[0];
        if(numArgs == 1 && (c == 'l' || c == 'z') )
        {
            return 1;
        }
        else if(numArgs == 2 && (c == 'B' || c == 'H' || c == 'I' || c == 'F' || c == 'S'))
        {
            return 1;
        }
        else if(numArgs == 3 && (c == 'b' || c == 'h' || c == 'i' || c == 'f' || c == 's' || c == 'w' || c == 'r'))
        {
            return 1;
        }
    }
    return -1;  //invalid command
}

//get's correct index from loc argument
int getLocation(char * location)
{
    int size = (int)strlen(location);
    int index = -1;
    
    if(size == 3)
        index = (100 * (location[0] % 48)) + ((location[1] * 10)  % 48) + (location[2] % 48);
    else if(size == 2)
        index = ((location[0] * 10)  % 48) + (location[1] % 48);
    else if(size == 1)
        index = location[0] % 48;
    
    return index;
}

//TODO:  Check for errors when a value greater than 1 char is inputed
int writeByte(int location, char * value) //b
{
    int intValue = getIntegerValueOf(value);
    if(location >= 0 && location < 128)
    {
        BUFFER[location] = intValue;
        return 1;
    }
    return 0;
}

//works, CHECK to see if output is suppose to be char, dec or hex
void readByte(int location) //B
{
    printf("%d\n", BUFFER[location]);
}

int getIntegerValueOf(char * value)
{
    int output = 0;
    for(int i = (int)strlen(value) - 1, power10 = 10; i >= 0; --i)
    {
        int cur = value[i] - 48;
        if(i < (int)strlen(value) - 1)
        {
            cur = cur * power10;
            power10 *= 10;
        }
        output += cur;
    }
    
    return output;
}

void writeInteger(int location, char * value)
{
    int intValue = getIntegerValueOf(value);
    
    if(intValue == 0)
    {
        BUFFER[location] = 0;
    }
    
    while(intValue > 0)
    {
        int curByte = intValue & 255;
        intValue = intValue >>8;
        
        char c = curByte;
        BUFFER[location] = c;
        ++location;
    }
}

void readInteger(int location)
{
    int value = BUFFER[location];
    for(int i = location + 1; BUFFER[i] != 0 && i < 128; ++i)
    {
        int cur = BUFFER[i] <<8;
        value = value | cur;
    }
    printf("%d\n", value);
}

void readHexadecimal(int location)
{
    char c = BUFFER[location];
    char one = HEXADECIMAL[c / 16];
    char two = HEXADECIMAL[c % 16];
    
    if(one != '0')
        printf("%c", one);
    
    printf("%c\n", two);
}

int writeHexadecimal(int location, char * value)
{
    int intValue = hexToInt(value);
    //////////Duplicatae code from write int merge later
    if(intValue == 0)
    {
        BUFFER[location] = 0;
    }
    
    while(intValue > 0)
    {
        int curByte = intValue & 255;
        intValue = intValue >>8;
        
        char c = curByte;
        BUFFER[location] = c;
        ++location;
    }
    //////////////
    return 1;
}

int hexToInt(char * value)
{
    int intOffset = 48;
    int charOffset = 40;
    int shift = 4;
    int intValue = 0;
    
    for(int index = (int)strlen(value) - 1; index >= 0; --index)
    {
        int currValue = value[index] - intOffset;
        if(currValue > 9)
            currValue -= charOffset;
        
        if(index == (int)strlen(value) - 1)
        {
            intValue = currValue;
        }
        else
        {
            currValue = currValue <<shift;
            intValue = intValue | currValue;
            shift += 4;
        }
    }
    return intValue;
}

//------------------------------------inProgress-----------------------------------------------------------







//-------------------------------------Need to be implemented-------------------------------------------

void readString(int location)
{
    
}

void readFloat(int location)
{
    
}

int writeFloat(int location, char * valu)
{
    return -1;
}

int writeString(int location, char * value)
{
    for(int i = 0; i < (int)strlen(value); ++i)
    {
        BUFFER[location + i] = value[i];
    }
    return -1;
}

