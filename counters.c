//
// TODO: Laura, can you add your EID, please?
//

/*
 * CS 377P: Programming For Performance
 * Assignment 1: Performance Counters
 * Laura Catano, UT EID: XXX
 * George Ellis, UT EID: ghe88
 */

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
#define INDEX 100
#define FAILURE -1
#define SUCCESS 0

// Globals
int eventSet = PAPI_NULL;                     // Event set for PAPI
int eventCount = 7;                           // Number of events to time
long long values[eventCount];                 // Return values for timed events
int eventCode[eventCount] = {                 // PAPI defined CONSTANTS
  PAPI_TOT_CYC_idx,
  PAPI_TOT_INS_idx,
  PAPI_LST_INS_idx,
  PAPI_L1_DCA_idx,
  PAPI_L1_DCM_idx,
  PAPI_L2_DCA_idx,
  PAPI_L2_DCM_idx
};
const char *eventStrings[eventCount] = {      // Descriptions of timed events
  "Total cyles",
  "Total instructions",
  "Total load/store instructions",
  "L1 data cache accesses",
  "L1 data cache misses",
  "L2 data cache accesses",
  "L2 data cache misses"
};

// Function headers
static int init_papi();
static void init_arrays(double *a, double *b, double *r, int index);
static int time_multiply(double *a, double *b, double *r, int index);
static void output_results();
static int reset_papi();
static int end_papi();
static void end(int status) __attribute__((noreturn));

int main(int argc, char **argv) {
  extern void dummy(void *);

  //
  // TODO: we should dynamically malloc and free these arrays
  //

  double matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];

  eventSet = PAPI_NULL;
  if(init_papi() != SUCCESS)
    end(FAILURE);

  //
  // TODO: Clear caches before performing matrix ops
  // XXX: Programatically read cache sizes, or just define arbitrarily large
  // XXX: array?
  //

  //
  // TODO: time varying sizes of matrix multiplication
  //

  if(time_multiply(&matrixa, &matrixb, &mresult, INDEX) != SUCCESS)
    end(FAILURE);
  output_results();
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
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if(retval != PAPI_VER_CURRENT) {
    printf("Error initializing PAPI\n");
    return FAILURE;
  }
  if(PAPI_create_eventset(&eventSet) != PAPI_OK) {
    printf("Error initializing EventSet to PAPI\n");
    return FAILURE;
  }
  if(PAPI_add_events(eventSet, &eventCode, eventCount) != PAPI_OK) {
    printf("Error adding events to PAPI\n");
    return FAILURE;
  }
  for(i = 0; i < eventCount; i++) values[i] = -1.0;
  return SUCCESS;
}

/*
 * Populates matrices with random doubles, initializes result matrix to zeroes
 */
static void init_arrays(double *a, double *b, double *r, int index) {
  int i;
  for(i = 0; i < index * index; i++) {
    *r[0][i] = 0.0;
    *a[0][i] = *b[0][i] = rand() * (double)1.1;
  }
}

/*
 * Resets all counters in event set and sets return value array to -1.0
 */
static int reset_papi_counters() {
  if(PAPI_reset(eventSet) != PAPI_OK) {
    printf("Error resetting event counters\n");
    return FAILURE;
  }
  for(i = 0; i < eventCount; i++) values[i] = -1.0;
  return SUCCESS;
}

/*
 * Cleans up event set and once empty, destroys it
 */
static int end_papi() {
  if(PAPI_cleanup_eventset(eventSet) != PAPI_OK) {
    printf("Error clearing event setn\n");
    return FAILURE;
  }
  if(PAPI_destroy_eventset(eventSet) != PAPI_OK) {
    printf("Error destroying event set\n");
    return FAILURE;
  }
  eventSet = PAPI_NULL;
  return SUCCESS;
}

/*
 * Output events to screen and write to .csv file for aggregation
 */
static void output_results() {
  int i;
  for(i = 0; i < eventCount; i++)
    printf("%s: %f\n", eventStrings[i], values[i]);

  //
  // TODO: write to .csv file
  //

}

/*
 * Start PAPI, perform matrix multiply, stop PAPI
 */
static int time_multiply(float *a, float *b, float *r, int index) {
  int i, j, k;
  if(PAPI_start(eventSet) != PAPI_OK) {
    printf("Error starting event counters\n");
    return FAILURE;
  }

  //
  // TODO: memory fence these operations
  //

  //
  // TODO: incorporate different loop variant orders
  //

  for (i = 0; i < index; i++)
   for(j = 0; j < index; j++)
    for(k = 0; k < index; k++)
      *r[i][j] = *r[i][j] + *a[i][k] * *b[k][j];

  if(PAPI_stop(eventSet, &values) != PAPI_OK) {
    printf("Error stopping event counters\n");
    return FAILURE;
  }
  return SUCCESS;
}

/*
 * End the program
 */
 static void end(int status) __attribute__((noreturn)) {

   //
   // TODO: free any memory that might still by malloc'ed
   //

   if(status == FAILURE) {
     printf("\n\t*** Irrecoverable failure. Exiting. ***\n\n");
     exit(-1);
   }

   printf("\n\t*** Success. Exiting. ***\n\n");
   exit(0);
   __builtin_unreachable();
 }
