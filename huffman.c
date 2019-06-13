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
#define AUXILIARY_CHAR '&'
#define MAX_HUFFMAN_TREE_HEIGHT 100
#define ENCODED_FILE "encoded.txt"
#define DECODE_DATA_FILE "decode-data.txt"
#define DECODED_FILE "decoded.txt"


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
int create_fd(char * file_name, int oflags, mode_t mode);
void write_data_for_decode(int* character_frequency, int num_unique_chars);
void read_data_for_decode(int* character_frequency, int* num_unique_chars);
void encode(char *file_name);
void decode(char *file_name);
Node *generate_huffman_tree(int* character_frequency, int num_unique_chars);

// Source for heap operations: https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3/
void heap_insert(MinHeap *min_heap, Node *node);
Node *heap_remove();
MinHeap *heap_init(int num_unique_chars);
void heap_build(MinHeap *min_heap);
void heap_print(MinHeap *min_heap);
void heap_print_codes(Node *node, char code[], int height);
int huffman_get_code(Node *node, char c, char code[], int height);
void huffman_free(Node* node);
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
        decode(argv[1]);
    }

    printf("File: %s Mode: %s\n", argv[1], argv[2]);
    return EXIT_SUCCESS;
}

void encode(char *file_name)
{
    int character_frequency[ASCII_VALUES] = {0};
    int input_file_fd = create_fd(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    struct stat sb;
    if (fstat(input_file_fd, &sb) == -1)
    {
        perror("Couldn't retrieve file size\n");
        return;
    }

    char *data = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, input_file_fd, 0);
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
    char code[MAX_HUFFMAN_TREE_HEIGHT];
    // heap_print_codes(root, code, 0);

    int encoded_file_fd = create_fd(ENCODED_FILE, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    for (int i = 0; i < sb.st_size; i++)
    {
        int code_len = huffman_get_code(root, data[i], code, 0);
        int bytes_written = write(encoded_file_fd, code, code_len);
        if(bytes_written < code_len) {
            perror("WTF, couldn't write all bytes");
            exit(EXIT_FAILURE);
        }
    }
    
    huffman_free(root);
    munmap(data, sb.st_size);
    close(encoded_file_fd);
    close(input_file_fd);

    write_data_for_decode(character_frequency, num_unique_chars);
}

void decode(char *file_name) {
    int character_frequency[ASCII_VALUES] = {0};
    int num_unique_chars = 0;
    read_data_for_decode(character_frequency, &num_unique_chars);
    
    int encoded_file_fd = create_fd(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
    int decoded_file_fd = create_fd(DECODED_FILE, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;
    if (fstat(encoded_file_fd, &sb) == -1)
    {
        perror("Couldn't retrieve file size\n");
        return;
    }

    char *data = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, encoded_file_fd, 0);
    Node* root = generate_huffman_tree(character_frequency, num_unique_chars);
    
    Node* curr = root;
    int i = 0;
    while(i <= sb.st_size) {
        if(curr->left == NULL && curr->right == NULL) {
            write(decoded_file_fd, &curr->character, 1);
            curr = root;
        }

        if(data[i] == '0') {
            curr = curr->left;
        } else if(data[i] == '1') {
            curr = curr->right;
        }
        i++;
    }

    huffman_free(root);
    munmap(data, sb.st_size);
    close(encoded_file_fd);
    close(decoded_file_fd);
}

Node *generate_huffman_tree(int* character_frequency, int num_unique_chars)
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

    while (min_heap->size > 1)
    {
        Node *left = heap_remove(min_heap);
        Node *right = heap_remove(min_heap);

        Node *auxiliary = create_node(left->frequency + right->frequency /* count */, AUXILIARY_CHAR);

        auxiliary->left = left;
        auxiliary->right = right;

        heap_insert(min_heap, auxiliary);
    }

    return heap_remove(min_heap);
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

void heap_build(MinHeap *min_heap)
{

    int n = min_heap->size - 1;
    int i;

    for (i = (n - 1) / 2; i >= 0; --i)
        heapify(min_heap, i);
}

void heap_print(MinHeap *min_heap)
{

    while (min_heap->size > 0)
    {
        Node *node = heap_remove(min_heap);
        printf("Char: %c Frequency: %d\n", node->character, node->frequency);
        free(node);
    }
}

void heap_print_codes(Node *node, char code[], int height)
{
    if (node->left == NULL && node->right == NULL)
    {
        // Leaf
        printf("Char: %c Freq: %d ", node->character, node->frequency);
        for (int i = 0; i < height; i++)
        {
            printf("%c ", code[i]);
        }
        printf("\n");
        return;
    }

    if (node->left != NULL)
    {
        code[height] = '0';
        heap_print_codes(node->left, code, height + 1);
    }

    if (node->right != NULL)
    {
        code[height] = '1';
        heap_print_codes(node->right, code, height + 1);
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

int huffman_get_code(Node *node, char c, char code[], int height) {
    if (node->left == NULL && node->right == NULL)
    {
        // Leaf
        if(node->character == c) {
            return height;
        }
        return 0;
    }

    code[height] = '0';
    int code_len = huffman_get_code(node->left, c, code, height + 1);
    if(code_len == 0) {
        code[height] = '1';
        code_len = huffman_get_code(node->right, c, code, height + 1);
    }

    return code_len;
}

void huffman_free(Node* node){
    if(node == NULL) {
        return;
    }

    huffman_free(node->left);
    huffman_free(node->right);
    free(node);
}

void write_data_for_decode(int* character_frequency, int num_unique_chars){
    int fd = create_fd(DECODE_DATA_FILE, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    int output_buffer[ASCII_VALUES + 2];
    int i;
    for(i = 0; i < ASCII_VALUES; i++) {
        dprintf(fd,"%d", character_frequency[i]);
        if(i + 1 < ASCII_VALUES){
            dprintf(fd," ");
        }
    }
    dprintf(fd, "\n%d",num_unique_chars);
    close(fd);
}

void read_data_for_decode(int* character_frequency, int* num_unique_chars){
    FILE* fp = fopen (DECODE_DATA_FILE, "r");
    if(fp == NULL) {
        perror("Could not open decode-data file\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ASCII_VALUES; i++)
    {
        fscanf (fp, "%d", &character_frequency[i]);
    }
    fscanf(fp, "%*c", NULL);
    fscanf(fp, "%d", num_unique_chars);
    fclose(fp);
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
    int fd = create_fd(file_name, O_RDONLY, S_IRUSR | S_IWUSR);
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

int create_fd(char * file_name, int oflags, mode_t mode) {
    int fd;
    if(!mode){
        fd = open(file_name, oflags);
    }else{
        fd = open(file_name, oflags, mode);
    }

    if(fd == -1) {
        fprintf(stderr, "Could not create file descriptor for %s\n", file_name);
        exit(EXIT_FAILURE);
    }
    return fd;
}