#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
#include "papi.h"
#include "emmintrin.h"

// Definitions
#define FAILURE 0
#define SUCCESS 1
#define IJK 0
#define IKJ 1
#define JIK 2
#define JKI 3
#define KIJ 4
#define KJI 5
#define ORDER_COUNT 6
#define EVENT_COUNT 7
#define INDEX_COUNT 8

// Macros
#define MATRIX_MULTIPLY(i, j, k, index, a, b, r) for (i = 0; i < index; i++)  \
 for(j = 0; j < index; j++)                                                   \
  for(k = 0; k < index; k++)                                                  \
    r[i][j] = r[i][j] + a[i][k] * b[k][j];

typedef struct m_struct {
  double** matrixa;
  double** matrixb;
  double** mresult;
} m_struct;

// Function prototypes
static void init_file();
static int init_cache_buffer(void** buffer);
static void clear_cache(void* buffer, int cache_size);
static void init_papi();
static inline void clear_papi_values();
static void output_papi_results(int index, int order);
static void reset_papi_counters();
static void end_papi();
static m_struct init_matrices(int index);
static double** alloc_array(int index);
static void load_matrices(m_struct matrices, int index);
static void free_matrices(m_struct matrices, int index);
static void multiply_matrices(m_struct matrices, int order, int index);


static void end(int status) __attribute__((noreturn));
