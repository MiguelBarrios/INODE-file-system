#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

        if(argc == 1)
        {
                printf("Started input numbers\n");

                 char name[10];
                 printf("Who are you? ");
                 //end of file is simulated by pressing CTRL-d
                while(fgets(name,10,stdin))
                {
                    printf("Glad to meet you, %s",name);
                }
        }
        else if(*argv[1] != 117 || argc > 2)
        {
            fprintf(stderr, "Invalid  Argument\n");
            return -1;
        }
        else
        {
                printf("Output will be in UPPERCASE\n");
        }

   return 0;
}