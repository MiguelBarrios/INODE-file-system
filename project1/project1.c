//  Miguel A Barrios Davila
//  project1
//
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "storage.h"
//silence pointer warning,   from float to int conversion
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

void list(void);
void zeroBuffer(void);
void writeToFile(int offset, int len, STORAGE *storage);
void readFromFile(int offset, int len, STORAGE *storage);

void writeHexadecimal(int location, char * value);
void writeInteger(int location, int intValue);
void writeFloat(char * location, char * numberString);
void writeString(int location, char * value);
void writeByte(int location, int value); //b
void writeChar(int location, char * value);

void readInteger(int location);//32 bit
void readFloat(int location);  //IEEE
void readString(int location);
void readHexadecimal(int location);
void readChar(int location);
void readByte(int location);

int hasCorrectNumArgs(char * command, int numArgs);
void printByteToHex(char c);
int integerValue(char * location);
float calculateMantissa(int ieeeRepresentation);
int convertHex(char c);
int hexToInt(char * value);

#define BUFF_SIZE 128
#define INTEGER_MAX 2147483647
#define INT_OFFSET 48
#define FIRST8BITS 255
#define LEADINGBIT 128

unsigned char *BUFFER;
const char SEPARATORS[] = " \t\n";
const char HEXADECIMAL[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','\0'};

int main(int argc, const char * argv[]) {
    
    STORAGE *storage = init_storage("storage.bin");
    BUFFER = malloc(BUFF_SIZE * sizeof(BUFFER));
    zeroBuffer();
    
    char in_buffer[BUFF_SIZE];                  // Input buffer from STDIN
    char * args[10];                            // pointers to arg strings
    char ** arg;                                // working pointer that steps through the args
    
    while(fgets(in_buffer, 129, stdin))
    {
        arg = args;
        *arg++ = strtok(in_buffer,SEPARATORS);   // tokenize input
        while ((*arg++ = strtok(NULL,SEPARATORS)));
        
        int numberOfArguments = 0;
        for(int i = 0;i < 4 && args[i] != NULL; ++i, ++numberOfArguments){} //gets number of arguments
        
        if(numberOfArguments <= 3 && hasCorrectNumArgs(args[0], numberOfArguments) != -1)
        {
            char commmand = args[0][0];
            switch (commmand)
            {
                case 'l': list();
                    break;
                case 'z': zeroBuffer();
                    break;
                case 'b': writeByte(integerValue(args[1]), integerValue(args[2]));
                    break;
                case 'h': writeHexadecimal(integerValue(args[1]), args[2]);
                    break;
                case 'i': writeInteger(integerValue(args[1]), integerValue(args[2]));
                    break;
                case 'f': writeFloat(args[1], args[2]);
                    break;
                case 's': writeString(integerValue(args[1]), args[2]);
                    break;
                case 'c': writeChar(integerValue(args[1]), args[2]);
                    break;
                case 'B': readByte(integerValue(args[1]));
                    break;
                case 'H': readHexadecimal(integerValue(args[1]));
                    break;
                case 'I': readInteger(integerValue(args[1]));
                    break;
                case 'F': readFloat(integerValue(args[1]));
                    break;
                case 'S': readString(integerValue(args[1]));
                    break;
                case 'C': readChar(integerValue(args[1]));
                    break;
                case 'w': writeToFile(integerValue(args[1]), integerValue(args[2]), storage);
                    break;
                case 'r': readFromFile(integerValue(args[1]), integerValue(args[2]), storage);
                    break;
            }
        }
        else
        {
            fprintf(stderr, "Invalid  Argument\n");
        }
    }
    //close_storage(storage);    TODO: check to see if we have to close at the end of program
    return 0;
}

void readFromFile(int offset, int len, STORAGE *storage)
{
    get_bytes(storage, BUFFER, offset, len);
}

void writeToFile(int offset, int len, STORAGE *storage)
{
    put_bytes(storage, BUFFER, offset, len);
}

int hasCorrectNumArgs(char * command, int numArgs)
{
    if(command == NULL)
        return -1;
    
    int tokenSize = (int)strlen(command);
    if(tokenSize == 1)
    {
        char c = command[0];
        if(numArgs == 1 && (c == 'l' || c == 'z') )
            return 1;
        else if(numArgs == 2 && (c == 'C' ||c == 'B' || c == 'H' || c == 'I' || c == 'F' || c == 'S'))
            return 1;
        else if(numArgs == 3 && (c == 'c' ||c == 'b' || c == 'h' || c == 'i' || c == 'f' || c == 's' || c == 'w' || c == 'r'))
            return 1;
    }
    return -1;  //invalid command
}

void list()
{
    for(int index = 0; index < BUFF_SIZE; index += 16)
    {
        for(int i = index; i < index + 16; ++i)
            printByteToHex(BUFFER[i]);
        printf("\n");
    }
}

void printByteToHex(char c)
{
    int value = c;
    int left = value & 15;
    int right = (value >>4) & 15;
    printf("%c%c ", HEXADECIMAL[right], HEXADECIMAL[left]);
}

void zeroBuffer() // z
{
    for(int i = 0; i < BUFF_SIZE; ++i)
        BUFFER[i] = 0;
}

void writeByte(int index, int value)
{
    BUFFER[index] = value;
}

//get's correct index from loc argument
int integerValue(char * location)
{
    int index;
    sscanf(location, "%d", &index);
    return index;
}

void writeHexadecimal(int location, char * value)
{
    int size = (int)strlen(value);
    int number = 0;
    
    if(size == 2)
    {
        number = convertHex(value[0]);
        number = number <<4;
        number = number | convertHex(value[1]);
    }
    else{
        number = convertHex(value[0]);
    }
    
    BUFFER[location] = number;
}

void readHexadecimal(int location)
{
    int number = BUFFER[location];
    int first4Bits = number & 15;
    int last4Bits = number >>4;
    
    if(last4Bits != 0)
        printf("%c%c\n", HEXADECIMAL[last4Bits], HEXADECIMAL[first4Bits]);
    else
        printf("%c\n", HEXADECIMAL[first4Bits]);
}

int hexToInt(char * value)
{
    int charOffset = 40;
    int shift = 4;
    int intValue = 0;
    
    for(int index = (int)strlen(value) - 1; index >= 0; --index)
    {
        int currValue = value[index] - INT_OFFSET;
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

int convertHex(char c)
{
    int value = c - INT_OFFSET;
    if(value > 9)
    {
        if(c == 'A' || c == 'a')
            value = 10;
        else if(c == 'B' || c == 'b')
            value = 11;
        else if(c == 'C' || c == 'c')
            value = 12;
        else if(c == 'D' || c == 'd')
            value = 13;
        else if(c == 'E' || c == 'e')
            value = 14;
        else if(c == 'F' || c == 'f')
            value = 15;
    }
    return value;
}

void writeInteger(int location, int intValue)
{
    BUFFER[location] = intValue & FIRST8BITS;              //byteOne
    BUFFER[location + 1] = (intValue >>8) & FIRST8BITS;    //byteTwo
    BUFFER[location + 2] = (intValue >>16) & FIRST8BITS;   //byteThree
    BUFFER[location + 3] = (intValue >>24) & FIRST8BITS;   //bytefour
}
void readInteger(int location)
{
    int byteOne = BUFFER[location];
    int byteTwo = BUFFER[location +1];
    int byteThree = BUFFER[location + 2];
    int byteFour = BUFFER[location + 3];
    
    int outputNumber = byteOne | (byteTwo <<8) | (byteThree <<16) | (byteFour <<24);
    
    if((byteFour & LEADINGBIT) == LEADINGBIT)
        outputNumber = (outputNumber & INTEGER_MAX) - 2147483648;
    
    printf("%d\n", outputNumber);
}

//Add Case for when user enters a whole number without a deimal
void writeFloat(char * location, char * numberString)
{
    int index = integerValue(location);
    float number;
    
    sscanf(numberString, "%f", &number);
    
    //How to get bits of a floating point number
    //https://stackoverflow.com/questions/1697425/how-to-print-out-each-bit-of-a-floating-point-number
    int *integerValueOfNumber;
    integerValueOfNumber = &number;
    int IEErep = *integerValueOfNumber;  //integer containing IEE Representation of float inputed
    
    writeInteger(index, IEErep);
}

void readFloat(int location)
{
    int byteOne = BUFFER[location];
    int byteTwo = BUFFER[location +1];
    int byteThree = BUFFER[location + 2];
    int byteFour = BUFFER[location + 3];
    
    int IEEENumRepresentation = byteOne | (byteTwo <<8) | (byteThree <<16) | (byteFour <<24);
    
    int exponentialBias = 127;
    float mantissa = calculateMantissa(IEEENumRepresentation);
    int exponentBits = 2139095040;
    int exponent = ((IEEENumRepresentation & exponentBits) >>23) - exponentialBias;  //turns of all but exponent bits, subtract Bias
    float number = (1 + mantissa) * pow(2, exponent);
    
    int isNegative = ((byteFour & LEADINGBIT) == LEADINGBIT) ? 1 : 0;
    if(isNegative)
        number *= -1;
    
    printf("%f\n", number);
}

float calculateMantissa(int ieeeRepresentation)
{
    int stream = ieeeRepresentation & 8388607;  //turns off all but last 23 bits
    float mantissa = 0;
    //For each bit in mantissa, calulate turn on bits to correct power of 2
    for(int i = 0, power = -23; i < 23; ++i, ++power, stream = stream >>1)
    {
        if((stream & 1) == 1)
            mantissa += pow(2, power);
    }
    
    return mantissa;
}

void writeString(int location, char * value)
{
    for(int i = 0; i <= (int)strlen(value); ++i)
        BUFFER[location + i] = value[i];
}

void readString(int location)
{
    for(int i = location; i < BUFF_SIZE && BUFFER[i] != 0; ++i)
    {
        char c = BUFFER[i];
        printf("%c", c);
    }
    printf("\n");
}

void writeChar(int location, char * value)
{
    BUFFER[location] = value[0];
}

void readChar(int location)
{
    printf("%c\n",BUFFER[location]);
}

//works, CHECK to see if output is suppose to be char, dec or hex
void readByte(int location) //B
{
    printf("%d\n", BUFFER[location]);
}

