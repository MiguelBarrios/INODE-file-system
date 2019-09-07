#include <stdio.h>
#include <string.h>



int main(int argc, char *argv[])
{
        char * numbers[] = {"","one", "two", "three", "four", "five", "six" , "seven"
                                ,"eight", "nine", "ten", "eleven", "twelve", "thirteen"
                                ,"fourteen", "fifteen", "sixteen", "seventeen", "eightteen"
                                ,"nineteen", "twenty", "thirty", "fourty", "fifty", "sixty"
                                , "seventy", "eighty", "ninty", "hundred", "thousand", "million"};

        if(argc == 1 || (*argv[1] == 117 && argc == 2))
        {
                printf("Started input numbers\n");

                 char name[11];

                 //end of file is simulated by pressing CTRL-d
                while(fgets(name,11,stdin))
                {
                    int firstNonZero = 0;
                    for(int i = 0; i < strlen(name) - 1; ++i)
                    {
                        if(name[i] != 48)
                        {
                            firstNonZero = i;
                            break;
                        }
                    }

                    int power = (strlen(name) - 1) - firstNonZero;
                    for(int i = firstNonZero; i < strlen(name) - 1; ++i)
                    {
                        int index = name[i] % 48;

                        if(power % 3 == 0 )
                        {
                            printf("%s %s ", numbers[index], numbers[28]);
                        }
                        else
                        { 

                            int number = 0;
                            if(power >= 2)
                            {
                                number = ((name[i] % 48) * 10) + (name[i+1] % 48);
                                ++i;
                            }
                            else
                            {
                                number = name[i] % 48;
                            }
                            

                            if(number < 20)
                            {
                                printf("%s ", numbers[number]);
                            }
                            else
                            {
                                int power10 = number  / 10;
                                int offset = 18 + power10;
                                int base = number % 10;
                                printf("%s %s ", numbers[offset], numbers[base]);
                            }
                        }
                    
                        --power;    
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