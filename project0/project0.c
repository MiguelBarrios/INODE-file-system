#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char * ones[] = {"","one ", "two ", "three ", "four ", "five ", "six " , "seven ","eight ", "nine " };
    char * teens[] = {"ten ", "elleven ", "twelve ", "thirteen ", "fourteen ", "fifteen ", "sixteen ",
        "seventeen ", "eighteen ", "nineteen "};
    char * tens[] = {"ten ", "twenty ", "thirty ", "forty ", "fifty ", "sixty ", "seventy ", "eighty ", "ninety "};
    char * powerTen[] = {"hundred ", "thousand ", "million "};
    
    if(argc == 1 || (argc == 2 && *argv[1] == 117))
    {
        char numberString[11];
        char ouputString[200];
        
        while(fgets(numberString,11,stdin))
        {
            ouputString[0] = '\0';            //resets string
            int length = (int)strlen(numberString) - 1;
            int firstNonZeroIndex = -1;
            for(int i = 0; i < length; ++i)
            {
                if(numberString[i] != 48)
                {
                    firstNonZeroIndex = i;
                    break;
                }
            }
            //number is not zero
            if(firstNonZeroIndex != -1)
            {
                int inputNumber[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
                int offset = 9 - (length - firstNonZeroIndex);
                
                //converts string to int array with correct number
                for(int i = firstNonZeroIndex, j = offset; i < length; ++i, ++j)
                {
                    inputNumber[j] = numberString[i] % 48;
                
                }
                
                for(int index = offset; index < 9; ++index)
                {
                    int currentNumber = inputNumber[index];
                    ////output hundred
                    if((index == 0 || index == 3 || index == 6) && (currentNumber != 0))
                    {
                        strcat(ouputString, ones[currentNumber]);
                        strcat(ouputString, powerTen[0]);
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
                            //apends one's digit
                            if((index == 2 || index == 5 || index == 8) && (currentNumber != 0))
                            {
                                strcat(ouputString, ones[currentNumber]);
                            }
                        }
                    }
                    if((index == 8 || index == 5 || index == 2 ) && (currentNumber != 0))
                    {
                        int sum = currentNumber + (((inputNumber[index - 1] != -1) ? inputNumber[index - 1] : 0) * 10);
                        if(sum < 10 && sum != 0)
                            strcat(ouputString, ones[sum]);
                    }
                    
                    //appends milion if applies
                    if(index == 2)
                    {
                        strcat(ouputString, powerTen[2]);
                    }
                    
                    //appends thousand if applies
                    if((index == 5) && (currentNumber != -1 || inputNumber[index -1] != -1 || inputNumber[index -2] != -1))
                    {
                        if(currentNumber != 0 || inputNumber[index -1] != 0 || inputNumber[index -2] != 0)
                            strcat(ouputString, powerTen[1]);
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
    }
    else
    {
        fprintf(stderr, "Invalid  Argument\n");
        return -1;
    }
    return 0;
}


