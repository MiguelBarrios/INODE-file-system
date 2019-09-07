#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char * ones[] = {"","one ", "two ", "three ", "four ", "five ", "six " , "seven ","eight ", "nine " };
    char * teens[] = {"ten ", "elleven ", "twelve ", "thirteen ", "fourteen ", "fifteen ", "sixteen ", "seventeen ", "eighteen ", "nineteen "};
    char * tens[] = {"ten ", "twenty ", "thirty ", "forty ", "fifty ", "sixty ", "seventy ", "eighty ", "ninety "};
    char * hundredPlus[] = {"hundred ", "thousand ", "million "};
    
    if(argc == 1 || (argc == 2 && *argv[1] == 117))
    {
        
        char numberString[11];
        printf("%s\n", "input number: ");
        //end of file is simulated by pressing CTRL-d
        while(fgets(numberString,11,stdin))
        {
            char ouputString[200];
            ouputString[0] = '\0';
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
            
            for(int index = firstNonZero; index < 9; ++index)
            {
                int currentNumber = inputNumber[index];
                if(currentNumber != -1)
                {
                    ////output hundred
                    if((index == 0 || index == 3 || index == 6) && (currentNumber != 0))
                    {
                        strcat(ouputString, ones[currentNumber]);
                        strcat(ouputString, hundredPlus[0]);
                        
                    }
                
                    //output tens
                    int isTeens = -1;
                    if((index == 1 || index == 4 || index == 7) && (currentNumber != 0))
                    {
                        isTeens = currentNumber * 10 + inputNumber[index + 1];
                        if(isTeens < 20 && isTeens >= 10)
                        {
                            strcat(ouputString, teens[isTeens % 10]);
                        }
                        else if(isTeens >= 20)
                        {
                            strcat(ouputString, tens[currentNumber - 1]);
                            ++index;
                            currentNumber = inputNumber[index];
                            //output one's
                            if((index == 2 || index == 5 || index == 8) && (currentNumber != 0))
                            {
                                strcat(ouputString, ones[currentNumber]);
                            }
                        }
                    }
                    if(index == 8 || index == 5 || index == 2)
                    {
                        int sum = currentNumber + (((inputNumber[index - 1] != -1) ? inputNumber[index - 1] : 0) * 10);
                        if(sum < 10 && sum != 0)
                            strcat(ouputString, ones[sum]);
                    }
                    
                    
        
                    //output milion
                    if(index == 2)
                    {
                        strcat(ouputString, hundredPlus[2]);
                        
                    }
                    //output thousand
                    if((index == 5) && (currentNumber != -1 || inputNumber[index -1] != -1 || inputNumber[index -2] != -1))
                    {
                        if(currentNumber != 0 || inputNumber[index -1] != 0 || inputNumber[index -2] != 0)
                            strcat(ouputString, hundredPlus[1]);
                    }
                    
                }
            }
            //Transforms output string to upper case if u argument was imputed
            if(argc == 2 && *argv[1] == 117)
            {
                for(int i = 0; i < strlen(ouputString) - 1; ++i)
                {
                    if(ouputString[i] != 32)
                        ouputString[i] = ouputString[i] - 32;
                }
            }
            
            printf("%s\n", ouputString);
        }
    }
    else
    {
        fprintf(stderr, "Invalid  Argument\n");
        return -1;
    }
    return 0;
}
