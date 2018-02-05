#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
#include "papi.h"

// Definitions

//
// TODO: index should be an array of predefined matrix sizes
//

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

typedef struct v_struct {
  int *i;
  int *j;
  int *k;
} v_struct;

// Function prototypes
static int init_papi();
static void init_file();
static void output_papi_results(int order);
static int reset_papi_counters();
static int end_papi();
static void end(int status) __attribute__((noreturn));
