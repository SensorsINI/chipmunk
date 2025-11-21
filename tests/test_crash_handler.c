/* Test program to verify crash handler functionality */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#ifdef __linux__
#include <execinfo.h>
#endif

static void crash_handler(int sig, siginfo_t *info, void *context)
{
  FILE *crash_log;
  char crash_log_path[512];
  time_t now;
  struct tm *tm_info;
  char timestamp[64];
  
  /* Suppress further signals to avoid recursive crashes */
  signal(SIGSEGV, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  
  /* Get timestamp */
  time(&now);
  tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
  
  /* Write crash log to /tmp for testing */
  snprintf(crash_log_path, sizeof(crash_log_path), "/tmp/chipmunk-crash-test.log");
  crash_log = fopen(crash_log_path, "a");
  
  /* Write crash information */
  fprintf(stderr, "\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "CHIPMUNK CRASH DETECTED (TEST)\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "Time: %s\n", timestamp);
  
  switch (sig) {
    case SIGSEGV:
      fprintf(stderr, "Signal: SIGSEGV (Segmentation Fault)\n");
      fprintf(stderr, "Cause: Memory access violation (invalid pointer, buffer overflow, etc.)\n");
      break;
    case SIGBUS:
      fprintf(stderr, "Signal: SIGBUS (Bus Error)\n");
      fprintf(stderr, "Cause: Invalid memory alignment or access to non-existent memory\n");
      break;
    case SIGABRT:
      fprintf(stderr, "Signal: SIGABRT (Abort)\n");
      fprintf(stderr, "Cause: Program called abort() or assertion failed\n");
      break;
    case SIGFPE:
      fprintf(stderr, "Signal: SIGFPE (Floating Point Exception)\n");
      fprintf(stderr, "Cause: Division by zero or invalid floating point operation\n");
      break;
    default:
      fprintf(stderr, "Signal: %d\n", sig);
      fprintf(stderr, "Cause: Unknown crash\n");
      break;
  }
  
  if (info != NULL) {
    fprintf(stderr, "Fault address: %p\n", info->si_addr);
  }
  
#ifdef __linux__
  {
    void *array[50];
    int size;
    char **strings;
    int i;
    
    size = backtrace(array, 50);
    strings = backtrace_symbols(array, size);
    
    if (strings != NULL) {
      fprintf(stderr, "\nStack trace (%d frames):\n", size);
      for (i = 0; i < size; i++) {
        fprintf(stderr, "  [%2d] %s\n", i, strings[i]);
      }
      free(strings);
    }
  }
#endif
  
  fprintf(stderr, "\nTo help fix this issue, please:\n");
  fprintf(stderr, "1. Note what you were doing when the crash occurred\n");
  fprintf(stderr, "2. Check if you can reproduce the crash\n");
  fprintf(stderr, "3. Report the issue at: https://github.com/sensorsINI/chipmunk/issues\n");
  fprintf(stderr, "4. Include this crash information and the crash log file (if created)\n");
  
  if (crash_log != NULL) {
    fprintf(crash_log, "\n========================================\n");
    fprintf(crash_log, "CHIPMUNK CRASH LOG (TEST)\n");
    fprintf(crash_log, "========================================\n");
    fprintf(crash_log, "Time: %s\n", timestamp);
    fprintf(crash_log, "Signal: %d\n", sig);
    if (info != NULL) {
      fprintf(crash_log, "Fault address: %p\n", info->si_addr);
    }
#ifdef __linux__
    {
      void *array[50];
      int size;
      char **strings;
      int i;
      
      size = backtrace(array, 50);
      strings = backtrace_symbols(array, size);
      
      if (strings != NULL) {
        fprintf(crash_log, "\nStack trace (%d frames):\n", size);
        for (i = 0; i < size; i++) {
          fprintf(crash_log, "  [%2d] %s\n", i, strings[i]);
        }
        free(strings);
      }
    }
#endif
    fprintf(crash_log, "========================================\n\n");
    fclose(crash_log);
    fprintf(stderr, "\nCrash log written to: %s\n", crash_log_path);
  }
  
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "\n");
  
  /* Re-raise signal to get core dump if enabled */
  raise(sig);
}

static void setup_crash_handlers(void)
{
  struct sigaction sa;
  
  sa.sa_sigaction = crash_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
  
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
}

int main(int argc, char *argv[])
{
  int test_type = 0;
  
  if (argc > 1) {
    test_type = atoi(argv[1]);
  }
  
  printf("Setting up crash handlers...\n");
  setup_crash_handlers();
  
  printf("Testing crash handler with different crash types:\n");
  printf("Usage: %s [test_type]\n", argv[0]);
  printf("  0 = SIGSEGV (segmentation fault) - default\n");
  printf("  1 = SIGFPE (division by zero)\n");
  printf("  2 = SIGABRT (abort)\n");
  printf("\n");
  printf("Triggering crash in 2 seconds...\n");
  sleep(2);
  
  switch (test_type) {
    case 1:
      printf("Testing SIGFPE (division by zero)...\n");
      {
        int x = 1;
        int y = 0;
        int z = x / y;  /* This will cause SIGFPE */
        (void)z;  /* Suppress unused variable warning */
      }
      break;
      
    case 2:
      printf("Testing SIGABRT (abort)...\n");
      abort();
      break;
      
    case 0:
    default:
      printf("Testing SIGSEGV (segmentation fault)...\n");
      {
        int *p = NULL;
        *p = 42;  /* This will cause SIGSEGV */
      }
      break;
  }
  
  /* Should never reach here */
  printf("ERROR: Crash handler did not work!\n");
  return 1;
}

