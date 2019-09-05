#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

        if(argc == 1)
        {
                printf("Output will be in lowercase\n");
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