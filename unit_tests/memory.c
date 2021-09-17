#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* duplicate_sptr(char *source)
{
    // deep copy a string
    int size = 0;
    for (int i = 0; ; i++)
    {
        if (source[i]) size++;
        else break;
    }
    char *destination = (char *) calloc(size+1, sizeof(char));
    for (int i = 0; i < size; i++) destination[i] = source[i];
    return destination;
}

char** duplicate_dptr(char **source)
{
    // deep copy an array of strings
    int size = 0;
    for (int i = 0; ; i++)
    {
        if (source[i]) size++;
        else break;
    }
    char **destination = (char **) calloc(size+1, sizeof(char*));

    for (int i = 0; i < size; i++) destination[i] = duplicate_sptr(source[i]);
    return destination;
}

void free_sptr(char **source)
{
    // free the space allocated to a string
    free(*source);
}

void free_dptr(char ***source)
{
    // free the space allocated to array of strings
    for (int i = 0; ; i++)
    {
        if ((*source)[i] == 0) break;
        free_sptr(&((*source)[i]));
    }
    free(*source);
}

int main()
{
    char *x = "sagal";
    char *y = duplicate_sptr(x);
    char* (ar[3]) = {"sagal", "preet"};
    ar[2] = 0;

    if (!strcmp(x, y))
    {
        printf("TEST-1 PASSED\n");
    }
    else
    {
        printf("TEST-1 FAILED\n");
    }

    free_sptr(&y);
    if (strcmp(x, y))
    {
        printf("TEST-2 PASSED\n");
    }
    else
    {
        printf("TEST-2 FAILED\n");
    }

    char **dar = duplicate_dptr(ar);
    for (int i = 0; i < 2; i++)
    {
        if (!strcmp(ar[i], *(dar+i)))
        {
            printf("TEST-%d PASSED\n", i+3);
        }
        else
        {
            printf("TEST-%d FAILED\n", i+3);
        }
    }

    free_dptr(&dar);
    

    return 1;
}