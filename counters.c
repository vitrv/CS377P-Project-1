/*
 * CS 377P: Programming For Performance
 * Assignment 1: Performance Counters
 * Laura Catano, UT EID: lnc652
 * George Ellis, UT EID: ghe88
 */

#include "counters.h"

// Globals
int eventSet = PAPI_NULL;                     // Event set for PAPI
long long values[EVENT_COUNT];                // Return values for timed events
FILE *file;                                   // Output file
const char *file_path = "./output.csv";       // Output file path
const char *file_flags = "w+";                // Writeable, rewrite previous
int file_open;                                // File opened status
struct timespec thread_time_begin;            // Timers
struct timespec real_time_begin;
struct timespec thread_time_end;
struct timespec real_time_end;
int eventCode1[EVENT_COUNT] = {               // PAPI defined CONSTANTS
  PAPI_TOT_CYC,
  PAPI_TOT_INS,
  PAPI_LST_INS,
  PAPI_FP_INS
};
int eventCode2[EVENT_COUNT] = {               // PAPI defined CONSTANTS
  PAPI_L1_DCA,
  PAPI_L1_DCM,
  PAPI_L2_DCA,
  PAPI_L2_DCM
};
const char *eventStrings1[EVENT_COUNT] = {    // Descriptions of timed events
  "Total cycles",
  "Total instructions",
  "Total load/store instructions",
  "Total floating point instructions"
};
const char *eventStrings2[EVENT_COUNT] = {    // Descriptions of timed events
  "L1 data cache accesses",
  "L1 data cache misses",
  "L2 data cache accesses",
  "L2 data cache misses"
};
const char *orderStrings[6] = {               // Descriptions of loops orders
  "I-J-K",
  "I-K-J",
  "J-I-K",
  "J-K-I",
  "K-I-J",
  "K-J-I"
};
int indexes[INDEX_COUNT] = {                  // Matrix sizes to multiply
  50, 100, 200, 400, 800, 1200, 1600, 2000
};

int main(int argc, char **argv) {
  extern void dummy(void *);
  int iteration, index, order, cache_size;
  m_struct matrices;
  void* buffer = NULL;

  init_file();
  init_timers();
  eventSet = PAPI_NULL;
  init_papi();
  cache_size = init_cache_buffer(&buffer);

  for(index = 0; index < INDEX_COUNT; index++) {
    for(order = 0; order < ORDER_COUNT; order++) {
      // Initialize matrices
      matrices = init_matrices(indexes[index]);

      // PAPI iterations
      for(iteration = 0; iteration < 2; iteration++) {
        clear_cache(buffer, cache_size);
        papi_MxM(matrices, index, order, iteration);
      }

      // clock_gettime() iteration
      clear_cache(buffer, cache_size);
      clock_MxM(matrices, index, order);

      // Free matrices
      free_matrices(matrices, indexes[index]);
    }
  }

  end_papi();
  end(SUCCESS);
}

/*
 * Multiplies matrices and times operations using PAPI
 */
static void papi_MxM(m_struct matrices, int index, int order, int iteration) {
  clear_papi_values();
  start_papi(iteration);
  multiply_matrices(matrices, order, indexes[index]);
  stop_papi(iteration);
  output_papi_results(indexes[index], order, iteration);
}

/*
 * Multiplies matrices and times operations clock_gettime()
 */
static void clock_MxM(m_struct matrices, int index, int order) {
  start_timers();
  multiply_matrices(matrices, order, indexes[index]);
  end_timers();
  output_time_results();
}

/*
 * Initializes file output
 */
static void init_file() {
  int i;
  file = fopen(file_path, file_flags);
  if(file == NULL) {
    printf("Error creating/opening file. Output to console only.\n");
    file_open = FAILURE;
  } else file_open = SUCCESS;
  fprintf(file, "Size,Order,");
  for(i = 0; i < EVENT_COUNT; i++) {
    fprintf(file, "%s,", eventStrings1[i]);
  }
  for(i = 0; i < EVENT_COUNT; i++) {
    fprintf(file, "%s,", eventStrings2[i]);
  }
  fprintf(file, "Thread time (sec),Real time (sec)\n");
}

/*
 * Initializes cache buffer by reading size from PAPI and allocating
 * corresponding space
 */
static int init_cache_buffer(void** buffer) {
  const PAPI_hw_info_t *hw_info = NULL;
  if ((hw_info = PAPI_get_hardware_info()) == NULL) {
    printf("\tError: couldn't retrieve cache size.\n");
    end(FAILURE);
  }
  int cache_size = (*hw_info).mem_hierarchy.level[0].cache[0].size +
    (*hw_info).mem_hierarchy.level[1].cache[0].size;
  *buffer = malloc(cache_size);
  if(buffer == NULL) {
    printf("\tError: couldn't allocate cache buffer.\n");
    end(FAILURE);
  }
  return cache_size;
}

/*
 * Clears cache by writing 'x' to every byte
 */
static void clear_cache(void* buffer, int cache_size) {
  memset(buffer, 99, (size_t)cache_size);
}

/*
 * Initializes timer structs
 */
static void init_timers() {
  thread_time_begin.tv_sec = thread_time_end.tv_sec = 0;
  thread_time_begin.tv_nsec = thread_time_end.tv_nsec = 0;
  real_time_begin.tv_sec = real_time_end.tv_sec = 0;
  real_time_begin.tv_nsec = real_time_end.tv_nsec = 0;
}

/*
 * Starts timers
 */
static void start_timers() {
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &thread_time_begin);
  clock_gettime(CLOCK_REALTIME, &real_time_begin);
}

/*
 * Ends timers
 */
static void end_timers() {
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &thread_time_end);
  clock_gettime(CLOCK_REALTIME, &real_time_end);
}

/*
 * Outputs time data to console and file
 */
static void output_time_results() {
  double thread_elapsed = ((unsigned)(thread_time_end.tv_nsec -
    thread_time_begin.tv_nsec)) / (double) 1000000000;
  double real_elapsed = ((unsigned)(real_time_end.tv_nsec -
    real_time_begin.tv_nsec)) / (double) 1000000000;
  if(file_open) {
    fprintf(file, "%f,%f\n", thread_elapsed, real_elapsed);
  }
  printf("Thread time: %f seconds\n", thread_elapsed);
  printf("Real time: %f seconds\n", real_elapsed);
}

/*
 * Initializes PAPI library and event set, adds events listed in eventCode1
 * to event set, and initializes return value array to 0.0
 */
static void init_papi() {
  int i;
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if(retval != PAPI_VER_CURRENT) {
    printf("Error initializing PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  if((retval = PAPI_create_eventset(&eventSet)) < PAPI_OK) {
    printf("Error initializing EventSet to PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
}

/*
 * Clears PAPI results array
 */
static inline void clear_papi_values() {
  int i;
  for(i = 0 ; i < EVENT_COUNT; i++) values[i] = 0.0;
}

/*
 * Adds specified event codes to event set and starts PAPI counters
 */
static void start_papi(int iteration) {
  int retval;
  if((retval = PAPI_add_events(eventSet,
    iteration == 0 ? eventCode1 : eventCode2, EVENT_COUNT)) < PAPI_OK) {
    printf("Error adding events to PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  PAPI_start(eventSet);
}

/*
 * Stops PAPI counters and removes last event codes from event set
 */
static void stop_papi(int iteration) {
  int retval;
  PAPI_stop(eventSet, values);
  if((retval = PAPI_cleanup_eventset(eventSet) != PAPI_OK)) {
    printf("Error cleaning up event set\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
}

/*
 * Output events to screen and write to .csv file for aggregation
 */
static void output_papi_results(int index, int order, int iteration) {
  int i;
  if(iteration == 0) {
    if(file_open) {
      fprintf(file, "%d,%s,", index, orderStrings[order]);
    }
    printf("\n\t\t=== Size: %d Order: %s===\n", index, orderStrings[order]);
  }
  for(i = 0; i < EVENT_COUNT; i++) {
    printf("%s: %lld\n",
      iteration == 0 ? eventStrings1[i] : eventStrings2[i], values[i]);
    fprintf(file, "%lld,", values[i]);
  }
}

/*
 * Cleans up event set and once empty, destroys it
 */
static void end_papi() {
  int retval;
  if((retval = PAPI_cleanup_eventset(eventSet)) < PAPI_OK) {
    printf("Error clearing event setn\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  if((retval = PAPI_destroy_eventset(&eventSet)) < PAPI_OK) {
    printf("Error destroying event set\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  eventSet = PAPI_NULL;
}

/*
 * Initializes matrices by allocating memory and returning pointers
 * in m_struct structure
 */
static m_struct init_matrices(int index) {
  m_struct matrices;
  matrices.matrixa = alloc_array(index);
  matrices.matrixb = alloc_array(index);
  matrices.mresult = alloc_array(index);
  if(matrices.matrixa == NULL && matrices.matrixb == NULL
    && matrices.mresult == NULL) {
      printf("Error: Couldn't initialize matrices\n");
      end(FAILURE);
  }
  load_matrices(matrices, index);
  return matrices;
}

/*
 * Allocates memory for a 2D array
 */
static double** alloc_array(int index) {
double** matrix;
matrix = (double**) malloc(index * sizeof(double*));
for (int i = 0; i < index; i++)
   matrix[i] = (double*) malloc(index * sizeof(double));
   return matrix;
}

/*
 * Zeroes all fields in result matrix and loads random doubles into
 * matrix a and b
 */
static void load_matrices(m_struct matrices, int index) {
  int i, j;
  for(i = 0; i < index; i++) {
    for(j = 0; j < index; j++) {
      matrices.mresult[i][j] = 0.0;
      matrices.matrixa[i][j] = matrices.matrixb[i][j] = rand() * (double)1.1;
    }
  }
}

/*
 * Frees memory resources used by matrices
 */
static void free_matrices(m_struct matrices, int index) {
  int i;
  for (i = 0; i < index; i++){
    free(matrices.matrixa[i]);
    free(matrices.matrixb[i]);
    free(matrices.mresult[i]);
  }
  free(matrices.matrixa);
  free(matrices.matrixb);
  free(matrices.mresult);
  matrices.matrixa = matrices.matrixb = matrices.mresult = NULL;
}

/*
 * Multiplies two matrices in specified order
 */
static void multiply_matrices(m_struct matrices, int order, int index) {
  // TODO: memory fence these operations
  int i, j, k;
  double** matrixa = matrices.matrixa;
  double** matrixb = matrices.matrixb;
  double** mresult = matrices.mresult;

  unsigned level, eax, ebx, ecx, edx;
  eax = level = 0;
  _mm_mfence();
  __get_cpuid(level, &eax, &ebx, &ecx, &edx);

  if(order == IJK)
    MATRIX_MULTIPLY(i, j, k, index, matrixa, matrixb, mresult)
  else if(order == IKJ)
    MATRIX_MULTIPLY(i, k, j, index, matrixa, matrixb, mresult)
  else if(order == JIK)
    MATRIX_MULTIPLY(j, i, k, index, matrixa, matrixb, mresult)
  else if(order == JKI)
    MATRIX_MULTIPLY(j, k, i, index, matrixa, matrixb, mresult)
  else if(order == KIJ)
    MATRIX_MULTIPLY(k, i, j, index, matrixa, matrixb, mresult)
  else
    MATRIX_MULTIPLY(k, j, i, index, matrixa, matrixb, mresult)
}

/*
 * End the program
 */
static void end(int status) {
  if(file_open) fclose(file);
  file = NULL;

  if(status == FAILURE) {
    printf("\n\t*** Irrecoverable failure. Exiting. ***\n\n");
    exit(-1);
  }

  printf("\n\t*** Success. Exiting. ***\n\n");
  exit(0);
  __builtin_unreachable();
}
