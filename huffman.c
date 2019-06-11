#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0
#define ASCII_VALUES 128

typedef struct node
{
    char character;
    int frequency;
} Node;

Node *heap[ASCII_VALUES];
int heap_size = 0;

int valid_parameters(char *file_name, char *mode);
void to_lower(char *str);
void encode(char *file_name);
void decode(char *file_name);
Node *generate_huffman_tree(int character_frequency[]);

// Source (heap_insert, heap_remove): https://www.sanfoundry.com/c-program-implement-heap/
void heap_insert(Node* node);
Node* heap_remove();
void heap_init(int character_frequency[]);
void heap_print();
Node* create_node(int count, char character);


int main(int argc, char **argv)
{
    if (argc < 3 || !valid_parameters(argv[1], argv[2]))
    {
        perror("Pass valid text file and mode (encode or decode)\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[2], "encode") == 0)
    {
        encode(argv[1]);
    }
    else if (strcmp(argv[2], "decode") == 0)
    {
        // decode(argv[1]);
    }

    printf("File: %s Mode: %s\n", argv[1], argv[2]);
    return EXIT_SUCCESS;
}

void encode(char *file_name)
{
    int character_frequency[ASCII_VALUES] = {0};
    int fd = open(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        perror("Couldn't retrieve file size\n");
        return;
    }

    char *data = (char*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    for (int i = 0; i < sb.st_size; i++)
    {
        character_frequency[data[i]] += 1;
    }

    Node *root = generate_huffman_tree(character_frequency);

    munmap(data, sb.st_size);
    close(fd);
}

void decode(char *file_name) {}

Node *generate_huffman_tree(int character_frequency[])
{
    heap_init(character_frequency);
    heap_print();
    return NULL;
}

void heap_insert(Node *node)
{
    heap_size++;
    heap[heap_size] = node;
    int now = heap_size;
    while (heap[now / 2]->frequency > node->frequency)
    {
        heap[now] = heap[now / 2];
        now /= 2;
    }
    heap[now] = node;
}

Node *heap_remove()
{
    Node *min_node = heap[1];
    Node *last_node = heap[heap_size--];
    /* now refers to the index at which we are now */
    int now, child;
    for (now = 1; now * 2 <= heap_size; now = child)
    {
        /* child is the index of the element which is minimum among both the children */
        /* Indexes of children are i*2 and i*2 + 1*/
        child = now * 2;
        /*child!=heapSize beacuse heap[heapSize+1] does not exist, which means it has only one
         child */
        if (child != heap_size && heap[child + 1]->frequency < heap[child]->frequency)
        {
            child++;
        }
        /* To check if the last element fits ot not it suffices to check if the last element
         is less than the minimum element among both the children*/
        if (last_node->frequency > heap[child]->frequency)
        {
            heap[now] = heap[child];
        }
        else /* It fits there */
        {
            break;
        }
    }
    heap[now] = last_node;
    return min_node;
}

void heap_init(int character_frequency[]){
    heap_size = 0;
    heap[0] = create_node(NULL, NULL);

    for(int i = 0; i < ASCII_VALUES; i++){
        if(character_frequency[i] > 0) {
            heap_insert(create_node(character_frequency[i]/* count */, i /* character */));
        }
    }
}

void heap_print(){
    while(heap_size > 0) {
        Node* removed_node = heap_remove();
        printf("Char: %c Count: %d\n", removed_node->character,removed_node->frequency);
        free(removed_node);
    }
}

Node* create_node(int count, char character){
    Node* node = (Node*) malloc(sizeof(Node));
    if(node == NULL){
        perror("Couldn't allocate memory for heap node\n");
        exit(EXIT_FAILURE);
    }
    node->frequency = count;
    node->character = character;
    return node;
}

int valid_parameters(char *file_name, char *mode)
{
    // Check mode
    to_lower(mode);
    if (strcmp(mode, "encode") != 0 && strcmp(mode, "decode") != 0)
    {
        return FALSE;
    }

    // Check file exists
    int fd = open(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd != -1)
    {
        close(fd);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void to_lower(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}