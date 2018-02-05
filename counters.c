//
// TODO: Laura, can you add your EID, please?
//

/*
 * CS 377P: Programming For Performance
 * Assignment 1: Performance Counters
 * Laura Catano, UT EID: XXX
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
  PAPI_L1_DCA,
  PAPI_L1_DCM,
  PAPI_L2_DCA,
  PAPI_L2_DCM
};
const char *eventStrings[EVENT_COUNT] = {      // Descriptions of timed events
  "Total cycles",
  "Total instructions",
  "Total load/store instructions",
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

int main(int argc, char **argv) {
  extern void dummy(void *);

  //
  // TODO: we should dynamically malloc and free these arrays
  //

  double matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];
  int i, j, k;
  for(i = 0; i < INDEX * INDEX; i++) {
    mresult[0][i] = 0.0;
    matrixa[0][i] = matrixb[0][i] = rand() * (double)1.1;
  }

  init_file();
  eventSet = PAPI_NULL;
  if(init_papi() != SUCCESS)
    end(FAILURE);

  //
  // TODO: Clear caches before performing matrix ops
  //

  if(PAPI_start(eventSet) != PAPI_OK) end(FAILURE);

  //
  // TODO: memory fence these operations
  //

  //
  // TODO: incorporate different loop variant orders
  //

  for (i = 0; i < INDEX; i++)
   for(j = 0; j < INDEX; j++)
    for(k = 0; k < INDEX; k++)
      mresult[i][j] = mresult[i][j] + matrixa[i][k] * matrixb[k][j];

  if(PAPI_stop(eventSet, values) != PAPI_OK) end(FAILURE);

  //
  // TODO: time varying sizes of matrix multiplication
  //

  output_papi_results(IJK);             // TODO: include this in size variation

  if(reset_papi_counters() != SUCCESS)
    end(FAILURE);
  if(end_papi() != SUCCESS)
    end(FAILURE);

  //
  // TODO: Repeat tests using clock_gettime
  //

  end(SUCCESS);
}

/*
 * Initializes PAPI library and event set, adds events listed in eventCode
 * to event set, and initializes return value array to -1.0 for error detection
 */
static int init_papi() {
  int i;
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if(retval != PAPI_VER_CURRENT) {
    printf("Error initializing PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    return FAILURE;
  }
  if((retval = PAPI_create_eventset(&eventSet)) < PAPI_OK) {
    printf("Error initializing EventSet to PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    return FAILURE;
  }
  if((retval = PAPI_add_events(eventSet, eventCode, EVENT_COUNT)) < PAPI_OK) {
    printf("Error adding events to PAPI\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    return FAILURE;
  }
  for(i = 0; i < EVENT_COUNT; i++) values[i] = -1.0;
  return SUCCESS;
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
   fprintf(file, "Order,");
   for(i = 0; i < EVENT_COUNT; i++) {
     fprintf(file, "%s,", eventStrings[i]);
   }
   fprintf(file, "\n");
}

/*
 * Resets all counters in event set and sets return value array to -1.0
 */
static int reset_papi_counters() {
  int i, retval;
  if((retval = PAPI_reset(eventSet)) < PAPI_OK) {
    printf("Error resetting event counters\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    return FAILURE;
  }
  for(i = 0; i < EVENT_COUNT; i++) values[i] = -1.0;
  // XXX: This prints zeroes. Curious...
  for(i = 0; i < EVENT_COUNT; i++) printf("%f\n", values[i]);
  return SUCCESS;
}

/*
 * Cleans up event set and once empty, destroys it
 */
static int end_papi() {
  int retval;
  if((retval = PAPI_cleanup_eventset(eventSet)) < PAPI_OK) {
    printf("Error clearing event setn\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    return FAILURE;
  }
  if((retval = PAPI_destroy_eventset(&eventSet)) < PAPI_OK) {
    printf("Error destroying event set\n");
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    return FAILURE;
  }
  eventSet = PAPI_NULL;
  return SUCCESS;
}

/*
 * Output events to screen and write to .csv file for aggregation
 */
static void output_papi_results(int order) {
  int i;
  if(file_open) {
    fprintf(file, "%s,", orderStrings[order]);
  }
  for(i = 0; i < EVENT_COUNT; i++) {
    printf("%s: %f\n", eventStrings[i], values[i]);
    fprintf(file, "%f,", values[i]);
  }
  fprintf(file, "\n");
}

/*
 * End the program
 */
 static void end(int status) {

   //
   // TODO: free any memory that might still by malloc'ed
   //

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
