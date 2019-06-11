#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define ASCII_VALUES 128

typedef struct node { 
    char character;
    int frequency;
    
    struct node* left;
    struct node* right; 
} Node;

int valid_parameters(char* file_name, char* mode);
void to_lower(char* str);
void encode(char* file_name);
void decode(char* file_name);
Node* generate_huffman_tree(int character_frequency[]);

int main(int argc, char **argv) {
    if(argc < 3 || !valid_parameters(argv[1], argv[2])) {
        perror("Pass valid text file and mode (encode or decode)\n");
        return EXIT_FAILURE;
    }

    if(strcmp(argv[2],"encode") == 0) {
        encode(argv[1]);
    } else if(strcmp(argv[2],"decode") == 0){
        // decode(argv[1]);
    }

    printf("File: %s Mode: %s\n", argv[1], argv[2]);
    return EXIT_SUCCESS;
}

void encode(char* file_name) {
    int character_frequency[ASCII_VALUES] = {0};
    int fd = open(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    struct stat sb;
    if(fstat(fd, &sb) == -1) {
        perror("Couldn't retrieve file size\n");
        return; 
    }
    
    char* data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    for(int i =0; i < sb.st_size;i++) {
        character_frequency[data[i]] += 1;
    }

    Node* root = generate_huffman_tree(character_frequency);

    munmap(data, sb.st_size);
    close(fd);
}

void decode(char* file_name){}

Node* generate_huffman_tree(int character_frequency []) { return NULL;}

int valid_parameters(char* file_name, char* mode){
    // Check mode
    to_lower(mode);
    if(strcmp(mode,"encode") != 0 && strcmp(mode,"decode") != 0) {
        return FALSE;
    }

    // Check file exists
    int fd = open(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd != -1) {
        close(fd);
    } else {
        return FALSE;
    }
    
    return TRUE;
}

void to_lower(char* str) {
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}