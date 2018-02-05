#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
#include "papi.h"

//
// TODO: index should be an array of predefined matrix sizes
//

// Definitions
#define INDEX 500
#define FAILURE 0
#define SUCCESS 1
#define IJK 0
#define IKJ 1
#define JIK 2
#define JKI 3
#define KIJ 4
#define KJI 5
#define EVENT_COUNT 7

// Macros
#define MATRIX_MULTIPLY(i, j, k, index, a, b, r) for (i = 0; i < index; i++)  \
 for(j = 0; j < index; j++)                                                   \
  for(k = 0; k < index; k++)                                                  \
    r[i][j] = r[i][j] + a[i][k] * b[k][j];
#define MATRIX_INIT(i, index, a, b, r) for(i = 0; i < index * index; i++) {   \
  r[0][i] = 0.0;                                                              \
  a[0][i] = b[0][i] = rand() * (double)1.1;                                   \
}

// Function prototypes
static int init_papi();
static void init_file();
static void output_papi_results(int order);
static int reset_papi_counters();
static int end_papi();
static void end(int status) __attribute__((noreturn));
