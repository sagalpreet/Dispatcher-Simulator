#include <stdlib.h>

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