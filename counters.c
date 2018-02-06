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
int eventCode[EVENT_COUNT] = {                // PAPI defined CONSTANTS
  PAPI_TOT_CYC,
  PAPI_TOT_INS,
  PAPI_LST_INS,
  PAPI_FP_INS,
  PAPI_L1_DCA,
  PAPI_L1_DCM,
  PAPI_L2_DCA,
  PAPI_L2_DCM
};
// The best TODO ever:
//  _____  _                        _
// |  _  || |                      | |
// | | | || |__       _ __    ___  | |
// | | | || '_ \     | '_ \  / _ \ | |
// \ \_/ /| | | | _  | | | || (_) ||_|
//  \___/ |_| |_|( ) |_| |_| \___/ (_)
//               |/
//
//  _____
// |_   _|
//   | |    ___    ___    _ __ ___    __ _  _ __   _   _
//   | |   / _ \  / _ \  | '_ ` _ \  / _` || '_ \ | | | |
//   | |  | (_) || (_) | | | | | | || (_| || | | || |_| |
//   \_/   \___/  \___/  |_| |_| |_| \__,_||_| |_| \__, |
//                                                  __/ |
//                                                 |___/
//                             _
//                            | |
//   ___   ___   _   _  _ __  | |_   ___  _ __  ___
//  / __| / _ \ | | | || '_ \ | __| / _ \| '__|/ __|
// | (__ | (_) || |_| || | | || |_ |  __/| |   \__ \
//  \___| \___/  \__,_||_| |_| \__| \___||_|   |___/
//
//
//       __
//  _   / /
// (_) | |
//     | |
//  _  | |
// (_) | |
//      \_\
//
//
//
//
//
//
//
//
//
//  _    _                       _
// | |  | |                     | |
// | |  | |  ___    ___   _ __  | | _   _
// | |/\| | / _ \  / _ \ | '_ \ | || | | |
// \  /\  /|  __/ | (_) || | | || || |_| |
//  \/  \/  \___|  \___/ |_| |_||_| \__, |
//                                   __/ |
//                                  |___/
//               _     ______                       _
//              | |   |___  /                      | |
//   __ _   ___ | |_     / /       __ _  _ __    __| |
//  / _` | / _ \| __|   / /       / _` || '_ \  / _` |
// | (_| ||  __/| |_  ./ /    _  | (_| || | | || (_| |
//  \__, | \___| \__| \_/    ( )  \__,_||_| |_| \__,_|
//   __/ |                   |/
//  |___/
//                    _  _    _
//                   | || |  (_)
//  _ __ ___   _   _ | || |_  _  ______
// | '_ ` _ \ | | | || || __|| ||______|
// | | | | | || |_| || || |_ | |
// |_| |_| |_| \__,_||_| \__||_|
//
//
//         _              _                 _
//        | |            (_)               (_)
//  _ __  | |  ___ __  __ _  _ __    __ _   _  ___
// | '_ \ | | / _ \\ \/ /| || '_ \  / _` | | |/ __|
// | |_) || ||  __/ >  < | || | | || (_| | | |\__ \
// | .__/ |_| \___|/_/\_\|_||_| |_| \__, | |_||___/
// | |                               __/ |
// |_|                              |___/
//  _                                                 __
// | |                                               / _|
// | |__   _   _   __ _   __ _  _   _   ___   ___   | |_   __ _  _ __
// | '_ \ | | | | / _` | / _` || | | | / __| / _ \  |  _| / _` || '__|
// | |_) || |_| || (_| || (_| || |_| | \__ \| (_) | | |  | (_| || |    _
// |_.__/  \__,_| \__, | \__, | \__, | |___/ \___/  |_|   \__,_||_|   (_)
//                 __/ |  __/ |  __/ |
//                |___/  |___/  |___/

const char *eventStrings[EVENT_COUNT] = {      // Descriptions of timed events
  "Total cycles",
  "Total instructions",
  "Total load/store instructions",
  "Total floating point instructions",
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
  int index, order, cache_size;
  m_struct matrices;
  void* buffer = NULL;

  init_file();
  eventSet = PAPI_NULL;
  init_papi();
  cache_size = init_cache_buffer(&buffer);
  clear_papi_values();

  for(index = 0; index < INDEX_COUNT; index++) {
    for(order = 0; order < ORDER_COUNT; order++) {
      matrices = init_matrices(indexes[index]);
      clear_cache(buffer, cache_size);

      // TODO: memory fence these 3 operations
      PAPI_start(eventSet);
      multiply_matrices(matrices, order, indexes[index]);
      PAPI_stop(eventSet, values);

      free_matrices(matrices, indexes[index]);
      output_papi_results(indexes[index], order);
      reset_papi_counters();
    }
  }

  end_papi();
  // TODO: Repeat tests using clock_gettime
  end(SUCCESS);
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
     fprintf(file, "%s,", eventStrings[i]);
   }
   fprintf(file, "\n");
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
    (*hw_info).mem_hierarchy.level[0].cache[0].size;
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
 * Initializes PAPI library and event set, adds events listed in eventCode
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
  PAPI_assign_eventset_component(eventSet, 0);
  if((retval = PAPI_multiplex_init()) != PAPI_OK) {
    printf("Error initializing multiplexing\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  if((retval = PAPI_create_eventset(&eventSet)) < PAPI_OK) {
    printf("Error initializing EventSet to PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  if((retval = PAPI_set_multiplex(eventSet)) != PAPI_OK) {
    printf("Error multiplexing EventSet\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  if((retval = PAPI_add_events(eventSet, eventCode, EVENT_COUNT)) < PAPI_OK) {
    printf("Error adding events to PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  for(i = 0; i < EVENT_COUNT; i++) values[i] = 0.0;
}

/*
 * Clears PAPI results array
 */
static inline void clear_papi_values() {
  int i;
  for(i = 0; i < EVENT_COUNT; i++) values[i] = 0.0;
}

/*
 * Output events to screen and write to .csv file for aggregation
 */
static void output_papi_results(int index, int order) {
  int i;
  if(file_open) {
    fprintf(file, "%d,%s,", index, orderStrings[order]);
  }
  printf("\nSize: %d Order: %s\n", index, orderStrings[order]);
  for(i = 0; i < EVENT_COUNT; i++) {
    printf("%s: %lld\n", eventStrings[i], values[i]);
    fprintf(file, "%lld,", values[i]);
  }
  fprintf(file, "\n");
}

/*
 * Resets all counters in event set and sets return value array to 0.0
 */
static void reset_papi_counters() {
  int i, retval;
  if((retval = PAPI_reset(eventSet)) < PAPI_OK) {
    printf("Error resetting event counters\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    end(FAILURE);
  }
  clear_papi_values();
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
  int i, j, k;
  // // TODO: define matrixa, matrixb, mresult
  double** matrixa = matrices.matrixa;
  double** matrixb = matrices.matrixb;
  double** mresult = matrices.mresult;
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
