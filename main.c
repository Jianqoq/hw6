#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

#define BUFSIZE 256
#define CONSTANT 128

double begin = 0;

typedef struct Entry {
  int *arr;
  struct Entry *next;
} Entry;

typedef struct Node {
  int *board;
  int size;
  struct Node *parent;
  int move;
} Node;

typedef struct HashSet {
  Entry **Entries;
  size_t size;
} HashSet;

typedef struct Queue {
  Node **node;
  int size;
  int front;
  int rear;
} Queue;

int abs(int val) {
  if (val < 0) {
    return -val;
  } else {
    return val;
  }
}

int estimate_cost(int *board, int size) {
  int total = 0;
  for (int i = 0; i < size * size; i++) {
    if (board[i] != 0) {
      int goal_row = (board[i] - 1) / size;
      int goal_col = (board[i] - 1) % size;
      int actual_row = i / size;
      int actual_col = i % size;
      total += abs(goal_row - actual_row) + abs(goal_col - actual_col);
    }
  }
  return total;
}
void enqueue(Queue *queue, Node *node) {
  if (queue->rear == queue->size - 1) {
    queue->node[queue->rear] = node;
    queue->rear = 0;
  } else {
    queue->node[queue->rear++] = node;
  }
}

Node *dequeue(Queue *queue) {
  if (queue->front == queue->rear) {
    return NULL;
  } else {
    if (queue->front == queue->size - 1) {
      int tmp = queue->front;
      queue->front = 0;
      return queue->node[tmp];
    } else {
      return queue->node[queue->front++];
    }
  }
}

int is_empty(Queue *queue) {
  if (queue->front == queue->rear) {
    return 1;
  } else {
    return 0;
  }
}

Queue *init_queue(int size) {
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  queue->node = (Node **)malloc(sizeof(Node *) * size);
  queue->size = size;
  queue->front = 0;
  queue->rear = 0;
  return queue;
}

size_t hash(int *arr, int arr_len, size_t size) {
  size_t hash = 0;

  for (size_t i = 0; i < arr_len; i++) {
    hash = (hash << 5) ^ (hash >> 27) ^ (size_t)arr[i];
  }

  return hash % size;
}

HashSet *init(size_t size) {
  Entry **entries = (Entry **)malloc(sizeof(Entry *) * (size + 1));
  for (size_t i = 0; i < (size + 1); i++)
    entries[i] = NULL;
  HashSet *set = (HashSet *)malloc(sizeof(HashSet));
  set->Entries = entries;
  set->size = size + 1;
  return set;
}

void insert(HashSet *set, int *arr, int size) {
  size_t index = hash(arr, size, set->size);
  if (set->Entries[index] != NULL) {
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    entry->next = NULL;
    entry->arr = arr;
    Entry *next = set->Entries[index];
    while (next->next != NULL) {
      next = next->next;
    }
    next->next = entry;
  } else {
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    entry->arr = arr;
    entry->next = NULL;
    set->Entries[index] = entry;
  }
}

int arr_equal(int *arr1, int *arr2, int size) {
  for (int i = 0; i < size; i++) {
    if (arr1[i] != arr2[i])
      return 0;
  }
  return 1;
}

int is_member(HashSet *set, int *arr, int size) {
  size_t index = hash(arr, size, set->size);
  if (set->Entries[index] != NULL) {
    if (arr_equal(arr, set->Entries[index]->arr, size)) {
      return 1;
    } else {
      Entry *entry = set->Entries[index]->next;
      while (entry != NULL) {
        if (arr_equal(arr, entry->arr, size))
          return 1;
        entry = entry->next;
      }
    }
  } else {
    return 0;
  }
}

void printBoard(int *board, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("%d ", board[i * size + j]);
    }
    printf("\n");
  }
  printf("\n");
}

void swap(int *board, int i, int j) {
  int temp = board[i];
  board[i] = board[j];
  board[j] = temp;
}

typedef struct Node2 {
  Node *node;
  int estimate_cost;
  struct Node2 *left;
  struct Node2 *right;
} Node2;

typedef struct Heap {
  int capacity;
  int size;
  Node2 **array;
} Heap;

int is_empty_heap(Heap *heap) { return heap->size == 0; }

Heap *init_heap(int capacity) {
  Heap *heap = (Heap *)malloc(sizeof(Heap));
  heap->capacity = capacity;
  heap->size = 0;
  heap->array = (Node2 **)malloc(sizeof(Node2 *) * capacity);
  for (int i = 0; i < heap->capacity; i++) {
    heap->array[i] = NULL;
  }
  return heap;
}

int parent(Heap *heap, int index) {
  int parent_index = (int)((index - 1) / 2);
  return parent_index;
}

void swap_heap(Node2 **codes, int index, int index2) {
  Node2 *tmp = codes[index];
  codes[index] = codes[index2];
  codes[index2] = tmp;
}

void upheap(Heap *myHeap, int index) {
  if (index == 0)
    return; // current node is the root
  int parentIndex = parent(myHeap, index);
  Node2 *parentValue = myHeap->array[parentIndex];
  if (parentValue->estimate_cost <= myHeap->array[index]->estimate_cost)
    return; // current node’s value is larger than its parent’s
  // else, we need to upheap current value by swapping with the parent
  swap_heap(myHeap->array, index, parentIndex);
  upheap(myHeap, parentIndex);
}

Node2 *init_node2(Node *__node, int size) {
  Node2 *node = (Node2 *)malloc(sizeof(Node2));
  node->node = __node;
  node->estimate_cost = estimate_cost(__node->board, size);
  node->left = NULL;
  node->right = NULL;
  return node;
}

void insert_heap(Heap *myHeap, Node2 *value) {
  if (myHeap->size == myHeap->capacity) {
    // printf("realloc\n");
    myHeap->array = (Node2 **)realloc(myHeap->array,
                                      sizeof(Node2 *) * myHeap->capacity * 2);
    myHeap->capacity *= 2;
    // printf("realloc done\n");
  }
  myHeap->array[myHeap->size] = value;
  upheap(myHeap, myHeap->size);
  myHeap->size++;
}

int min_child_index(Heap *heap, int index) {
  if (heap->size - 1 >= index * 2 + 1) { /*not a leaf*/
    if (heap->array[index * 2 + 1]->estimate_cost >=
        heap->array[index * 2 + 2]->estimate_cost)
      return index * 2 + 2;
    else
      return index * 2 + 1;
  } else if (heap->size - 1 == (index * 2 + 2))
    return index * 2 + 2;
  else
    return -1;
}

void down_heap(Heap *heap, int index) {
  if (heap->size - 1 < (index * 2 + 2)) {
    if (heap->size - 1 >= (index * 2 + 1)) {
      if (heap->array[index]->estimate_cost >=
          heap->array[index * 2 + 1]->estimate_cost)
        swap_heap(heap->array, index, index * 2 + 1);
    }
    return;
  }
  int min_child = min_child_index(heap, index);
  if (heap->array[index]->estimate_cost <=
      heap->array[min_child]->estimate_cost)
    return;
  swap_heap(heap->array, index, min_child);
  down_heap(heap, min_child);
}

Node *delete_min(Heap *heap) {
  Node2 *tmp = heap->array[0];
  swap_heap(heap->array, 0, heap->size - 1);
  heap->size--;
  down_heap(heap, 0);
  return tmp->node;
}

HashSet *set2 = NULL;
HashSet *set1 = NULL;
Queue *queue = NULL;
Heap *heap = NULL;
int *goal = NULL;

Node **yield_node(Node *node) {
  Node **nodes = (Node **)malloc(sizeof(Node *) * 4);
  for (int row = 0; row < node->size; row++) {
    for (int column = 0; column < node->size; column++) {
      if (node->board[row * node->size + column] == 0) {
        // Up
        if (row == 0) {
          nodes[0] = NULL;
        } else {
          Node *tmp0 = (Node *)malloc(sizeof(Node));
          tmp0->move = node->board[(row - 1) * node->size + column];
          tmp0->size = node->size;
          tmp0->board = (int *)malloc(sizeof(int) * node->size * node->size);
          memcpy(tmp0->board, node->board,
                 sizeof(int) * node->size * node->size);
          swap(tmp0->board, row * node->size + column,
               (row - 1) * node->size + column);
          tmp0->parent = node;
          nodes[0] = tmp0;
        }
        // Down
        if (row == node->size - 1) {
          nodes[1] = NULL;
        } else {
          Node *tmp1 = (Node *)malloc(sizeof(Node));
          tmp1->move = node->board[(row + 1) * node->size + column];
          tmp1->size = node->size;
          tmp1->board = (int *)malloc(sizeof(int) * node->size * node->size);
          memcpy(tmp1->board, node->board,
                 sizeof(int) * node->size * node->size);
          tmp1->parent = node;
          swap(tmp1->board, row * node->size + column,
               (row + 1) * node->size + column);
          nodes[1] = tmp1;
        }
        // Left
        if (column == 0) {
          nodes[2] = NULL;
        } else {
          Node *tmp2 = (Node *)malloc(sizeof(Node));
          tmp2->move = node->board[row * node->size + column - 1];
          tmp2->size = node->size;
          tmp2->board = (int *)malloc(sizeof(int) * node->size * node->size);
          memcpy(tmp2->board, node->board,
                 sizeof(int) * node->size * node->size);
          swap(tmp2->board, row * node->size + column,
               row * node->size + column - 1);
          tmp2->parent = node;
          nodes[2] = tmp2;
        }
        // Right
        if (column == node->size - 1) {
          nodes[3] = NULL;
        } else {
          Node *tmp3 = (Node *)malloc(sizeof(Node));
          tmp3->move = node->board[row * node->size + column + 1];
          tmp3->size = node->size;
          tmp3->board = (int *)malloc(sizeof(int) * node->size * node->size);
          memcpy(tmp3->board, node->board,
                 sizeof(int) * node->size * node->size);
          swap(tmp3->board, row * node->size + column,
               row * node->size + column + 1);
          tmp3->parent = node;
          nodes[3] = tmp3;
        }
        if (nodes[0] == NULL && nodes[1] == NULL && nodes[2] == NULL &&
            nodes[3] == NULL) {
          return NULL;
        }
        return nodes;
      }
    }
  }

  free(nodes);
  return NULL;
}

char *build_graph(Node *root) {
  insert(set2, root->board, root->size * root->size);
  insert_heap(heap, init_node2(root, root->size));
  int total_size = root->size * root->size;
  while (!is_empty_heap(heap)) {
    Node *node = delete_min(heap);
    if (node->board[total_size - 1] == goal[total_size - 1] &&
        arr_equal(node->board, goal, node->size * node->size)) {
      Node *tmp = node;
      int length = 0;
      while (tmp != NULL) {
        length++;
        tmp = tmp->parent;
      }
      int *moves = (int *)malloc(sizeof(int) * (length - 1));
      tmp = node;
      int idx = length - 2;
      while (tmp != NULL) {
        moves[idx--] = tmp->move;
        tmp = tmp->parent;
      }
      char *result = (char *)malloc(sizeof(char) * 1000);
      int tracker = 0;
      for (int i = 0; i < length - 1; i++) {
        char num[100];
        sprintf(num, "%d", moves[i]);
        for (int j = 0; j < strlen(num); j++) {
          result[tracker++] = num[j];
        }
        result[tracker++] = ' ';
      }
      result[tracker] = '\0';
      return result;
    }
    Node **nodes = yield_node(node);
    if (nodes == NULL) {
      continue;
    }
    for (int i = 0; i < 4; i++) {
      if (nodes[i] != NULL) {
        if (is_member(set2, nodes[i]->board, root->size * root->size)) {
          continue;
        } else {
          insert_heap(heap, init_node2(nodes[i], root->size));
          insert(set2, nodes[i]->board, nodes[i]->size * nodes[i]->size);
        }
      }
    }
  }
  return NULL;
}

int main(int argc, char **argv) {
  FILE *fp_in, *fp_out;

  fp_in = fopen(argv[1], "r");
  if (fp_in == NULL) {
    printf("Could not open a file.\n");
    return -1;
  }

  fp_out = fopen(argv[2], "w");
  if (fp_out == NULL) {
    printf("Could not open a file.\n");
    return -1;
  }

  char *line = NULL;
  size_t lineBuffSize = 0;
  ssize_t lineSize;
  int k;

  getline(&line, &lineBuffSize,
          fp_in); // ignore the first line in file, which is a comment
  fscanf(fp_in, "%d\n", &k); // read size of the board
  // printf("k = %d\n", k); //make sure you read k properly for DEBUG purposes
  getline(&line, &lineBuffSize,
          fp_in); // ignore the second line in file, which is a comment

  int initial_board[k * k]; // get kxk memory to hold the initial board
  for (int i = 0; i < k * k; i++)
    fscanf(fp_in, "%d ", &initial_board[i]);
  // Assuming that I have a function to print the board, print it here to make
  // sure I read the input board properly for DEBUG purposes
  fclose(fp_in);
  ////////////////////
  // do the rest to solve the puzzle
  ////////////////////
  char *moves = NULL;
  int solvable = 0;
  int inverse = 0;
  for (int i = 0; i < k * k; i++) {
    for (int j = i; j < k * k; j++) {
      if (initial_board[i] != 0 && initial_board[j] != 0 &&
          initial_board[i] > initial_board[j])
        inverse++;
    }
  }
  heap = init_heap(100000);
  if (k % 2 == 0) {
    int row = 0;
    for (int i = 0; i < k; i++) {
      for (int j = 0; j < k; j++) {
        if (initial_board[i * k + j] == 0) {
          row = i;
        }
      }
    }
    if ((row + inverse) % 2 == 0) {
      solvable = 0;
    } else {
      solvable = 1;
      set2 = init(100013);
      Node *root = (Node *)malloc(sizeof(Node));
      root->size = k;
      root->board = (int *)malloc(sizeof(int) * k * k);
      memcpy(root->board, initial_board, sizeof(int) * k * k);
      root->parent = NULL;
      root->move = -1;
      goal = (int *)malloc(sizeof(int) * k * k);
      for (int i = 1; i < k * k; i++) {
        goal[i - 1] = i;
      }
      goal[k * k - 1] = 0;
      moves = build_graph(root);
      free(set2->Entries);
      free(set2);
      free(goal);
    }
  } else {
    if (inverse % 2 == 0) {
      solvable = 1;
      set2 = init(100000);
      Node *root = (Node *)malloc(sizeof(Node));
      root->size = k;
      root->board = (int *)malloc(sizeof(int) * k * k);
      memcpy(root->board, initial_board, sizeof(int) * k * k);
      root->parent = NULL;
      root->move = -1;
      goal = (int *)malloc(sizeof(int) * k * k);
      for (int i = 1; i < k * k; i++) {
        goal[i - 1] = i;
      }
      goal[k * k - 1] = 0;
      moves = build_graph(root);
      free(set2->Entries);
      free(set2);
      free(goal);
    } else {
      solvable = 0;
    }
  }
  // once you are done, you can use the code similar to the one below to print
  // the output into file if the puzzle is NOT solvable use something as follows
  fprintf(fp_out, "#moves\n");
  if (moves != NULL)
    fprintf(fp_out, "%s\n", moves);
  else
    fprintf(fp_out, "no solution\n");
  // if it is solvable, then use something as follows:
  // probably within a loop, or however you stored proper moves, print them one
  // by one by leaving a space between moves, as below
  //  for(int i=0;i<numberOfMoves;i++)
  //  	fprintf(fp_out, "%d ", move[i]);

  fclose(fp_out);

  return 0;
}
