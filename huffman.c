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
    unsigned int frequency;
    struct node *left;
    struct node *right;
} Node;

typedef struct
{
    unsigned int size;
    unsigned int capacity;

    Node **array;
} MinHeap;

int valid_parameters(char *file_name, char *mode);
void to_lower(char *str);
void encode(char *file_name);
void decode(char *file_name);
Node *generate_huffman_tree(int character_frequency[], int num_unique_chars);

// Source for heap operations: https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3/
void heap_insert(MinHeap *min_heap, Node *node);
Node *heap_remove();
MinHeap *heap_init(int num_unique_chars);
void heap_build(MinHeap* min_heap);
void heap_print(MinHeap* min_heap);
Node *create_node(int count, char character);
void swap_node(Node **a, Node **b);
void heapify(MinHeap *min_heap, int idx);

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

    char *data = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    int num_unique_chars = 0;
    for (int i = 0; i < sb.st_size; i++)
    {
        if (character_frequency[data[i]] == 0)
        {
            num_unique_chars += 1;
        }
        character_frequency[data[i]] += 1;
    }

    Node *root = generate_huffman_tree(character_frequency, num_unique_chars);

    munmap(data, sb.st_size);
    close(fd);
}

void decode(char *file_name) {}

Node *generate_huffman_tree(int character_frequency[], int num_unique_chars)
{
    MinHeap *min_heap = heap_init(num_unique_chars);
    int index = 0;
    for (int i = 0; i < ASCII_VALUES; i++)
    {
        if (character_frequency[i] > 0)
        {
            min_heap->array[index] = create_node(character_frequency[i] /* count */, i /* char */);
            min_heap->size += 1;
            index++;
        }
    }
    heap_build(min_heap);
    heap_print(min_heap);
    return NULL;
}

void heap_insert(MinHeap *min_heap, Node *node)
{

    ++min_heap->size;
    int i = min_heap->size - 1;

    while (i && node->frequency < min_heap->array[(i - 1) / 2]->frequency)
    {

        min_heap->array[i] = min_heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    min_heap->array[i] = node;
}

Node *heap_remove(MinHeap *min_heap)
{
    Node *temp = min_heap->array[0];
    min_heap->array[0] = min_heap->array[min_heap->size - 1];

    --min_heap->size;
    heapify(min_heap, 0);

    return temp;
}

MinHeap *heap_init(int num_unique_chars)
{
    MinHeap *min_heap = (MinHeap *)malloc(sizeof(MinHeap));
    min_heap->size = 0;
    min_heap->capacity = num_unique_chars;
    min_heap->array = (Node **)malloc(min_heap->capacity * sizeof(Node *));
    return min_heap;
}

void heap_build(MinHeap* min_heap)
{ 
  
    int n = min_heap->size - 1; 
    int i; 
  
    for (i = (n - 1) / 2; i >= 0; --i) 
        heapify(min_heap, i); 
}

void heap_print(MinHeap* min_heap) {
    
    while(min_heap->size > 0){
        Node* node = heap_remove(min_heap);
        printf("Char: %c Frequency: %d\n", node->character, node->frequency);
        free(node);
    }
}

// The standard minHeapify function.
void heapify(MinHeap *min_heap, int idx)
{

    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    if (left < min_heap->size && min_heap->array[left]->frequency < min_heap->array[smallest]->frequency)
        smallest = left;

    if (right < min_heap->size && min_heap->array[right]->frequency < min_heap->array[smallest]->frequency)
        smallest = right;

    if (smallest != idx)
    {
        swap_node(&min_heap->array[smallest], &min_heap->array[idx]);
        heapify(min_heap, smallest);
    }
}

Node *create_node(int count, char character)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL)
    {
        perror("Couldn't allocate memory for heap node\n");
        exit(EXIT_FAILURE);
    }
    node->frequency = count;
    node->character = character;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void swap_node(Node **a, Node **b)
{
    Node *t = *a;
    *a = *b;
    *b = t;
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