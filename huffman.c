#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define TRUE 1
#define FALSE 0


int valid_parameters(char* file_name, char* mode);
void to_lower(char* str);

int main(int argc, char **argv) {
    if(argc < 3 || !valid_parameters(argv[1], argv[2])) {
        fprintf(stderr, "Pass valid text file and mode (encode or decode)\n");
        return EXIT_FAILURE;
    }

    printf("File: %s Mode: %s\n", argv[1], argv[2]);
    return EXIT_SUCCESS;
}

int valid_parameters(char* file_name, char* mode){
    // Check mode
    to_lower(mode);
    if(strcmp(mode,"encode") != 0 && strcmp(mode,"decode") != 0) {
        return FALSE;
    }

    // Check file exists
    FILE* fp;
    if (!(fp = fopen(file_name, "r"))) {
        return FALSE;
    } else {
        fclose(fp);
    }
    
    return TRUE;
}

void to_lower(char * str) {
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}