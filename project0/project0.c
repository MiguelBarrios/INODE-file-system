#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char * numbers[] = {"","one", "two", "three", "four", "five", "six" , "seven","eight", "nine",
        "ten", "eleven", "twelve", "thirteen","fourteen", "fifteen", "sixteen", "seventeen", "eightteen", "nineteen", "twenty",
        "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninty", "hundred", "thousand",
        "million"};
    
    char * tens[] = {"ten", "twenty", "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninty"};
    char * ones[] = {"","one", "two", "three", "four", "five", "six" , "seven","eight", "nine" };
    char * hundredPlus[] = {"hundred", "thousand", "million"};
    
    if(argc == 1 || (*argv[1] == 117 && argc == 2))
    {
        
        char numberString[11];
        printf("%s\n", "input number: ");
        //end of file is simulated by pressing CTRL-d
        while(fgets(numberString,11,stdin))
        {
            int firstNonZero = 0;
            for(int i = 0; i < strlen(numberString) - 1; ++i)
            {
                if(numberString[i] != 48)
                {
                    firstNonZero = i;
                    break;
                }
            }
            
            int inputNumber[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
            int power = ((int)strlen(numberString) - 1) - firstNonZero;
            int offset = 9 - power;
            
            //converts string to int array with correct number
            for(int i = firstNonZero; i < strlen(numberString) - 1; ++i, ++offset)
            {
                int currentNum =  numberString[i] % 48;
                inputNumber[offset] = currentNum;
            }
            
            for(int index = 0; index < 9; ++index)
            {
                int currentNumber = inputNumber[index];
                if(inputNumber[index] != -1)
                {
                    ////output hundred
                    if((index == 0 || index == 3 || index == 6) && (inputNumber[index] != 0))
                    {
                        printf("%s %s ",ones[inputNumber[index]], numbers[28]);
                    }
                    
                    //output tens
                    if((index == 1 || index == 4 || index == 7) && (inputNumber[index] != 0))
                    {
                        printf("%s ",tens[inputNumber[index] - 1]);
                    }
                    
                    //output one's
                    if((index == 2 || index == 5 || index == 8) && (inputNumber[index] != 0))
                    {
                        printf("%s ", ones[inputNumber[index]]);
                    }
                    
                    
                    //output milion
                    if(index == 2)
                    {
                        printf("%s ",hundredPlus[2]);
                    }
                    //output thousand
                    if((index == 5) && (inputNumber[index] != -1 || inputNumber[index -1] != -1 || inputNumber[index -2] != -1))
                    {
                        if(inputNumber[index] != 0 || inputNumber[index -1] != 0 || inputNumber[index -2] != 0)
                            printf("%s ",hundredPlus[1]);
                    }
                    
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Invalid  Argument\n");
        return -1;
    }
    
    return 0;
}