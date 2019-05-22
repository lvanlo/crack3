#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "md5.h"

const int PASS_LEN=50;        // Maximum any password can be
const int HASH_LEN=33;        // Length of MD5 hash strings
char *contents;


// Stucture to hold both a plaintext password and a hash.
struct entry 
{
    char *password;
    char *hash;
};

int file_length(char *filename)
{
    struct stat info;
    int res = stat(filename, &info);
    if (res == -1) return -1;
    else return info.st_size;
}

int hashcomp(const void *a, const void *b)
{
    struct entry *pa = (struct entry *)a;
    struct entry *pb = (struct entry *)b;
    return strcmp(pa->hash, pb->hash);
}

int hashfind(const void *target, const void *elem)
{
    char *target_str = (char *)target;
    struct entry *pelem = (struct entry *)elem;
    return strcmp(target_str, (*pelem).hash);
}

// TODO
// Read in the dictionary file and return an array of structs.
// Each entry should contain both the hash and the dictionary
// word.
struct entry *read_dictionary(char *filename, int *size)
{
    int filelength = file_length(filename);
    FILE *c = fopen(filename, "r");
    if (!c)
    {
        printf("Can't open %s for reading\n", filename);
        exit(1);
    }
    
    contents = malloc(filelength);
    fread(contents, 1, filelength, c);
    fclose(c);
    
    // Loop through contents, replace \n with \0
    int lines = 0;
    for (int i = 0; i < filelength; i++)
    {
        if (contents[i] == '\n') {
            contents[i] = '\0';
            lines++;
        }
    }
    //printf("lines %d\n", lines);
    
    // Allocate array of structs
    struct entry *entries = malloc(lines * sizeof(struct entry));

    //char pword[20];
    
    // Copy the first password into the entries array
    entries[0].password = &contents[0];
    entries[0].hash = md5(entries[0].password, strlen(entries[0].password));
    
    int count = 1;
    for (int i = 0; i < filelength - 1; i++)
    {
        if (contents[i] == '\0')
        {
            entries[count].password = &contents[i+1];
            entries[count].hash = md5(entries[count].password, strlen(entries[count].password));
            count++;
        }
    }
    
    *size = lines;
    return entries;
}


int main(int argc, char *argv[])
{
    if (argc < 3) 
    {
        printf("Usage: %s hash_file dict_file\n", argv[0]);
        exit(1);
    }
    int dlen;
    // TODO: Read the dictionary file into an array of entry structures
    struct entry *dict = read_dictionary(argv[2], &dlen);
    
    // Sort them by password
    qsort(dict, dlen, sizeof(struct entry), hashcomp);
    
    FILE *h = fopen(argv[1], "r");
    if (!h)
    {
        printf("Can't open %s for reading\n", argv[1]);
        exit(1);
    }
    
    char line[33];
    while(fgets(line,33,h)!=NULL)
    {
        struct entry *found = bsearch(line, dict, dlen, sizeof(struct entry), hashfind);
        if (found != NULL)
        {
            printf("Hacker Voice: I'm in. %s %s\n", found->password, found->hash);
            
        }
    }
    
    for (int i=0; i <dlen; i++)
    {
        free(dict[i].hash);
    }
    
    free(contents);
    free(dict);
    fclose(h);
}
