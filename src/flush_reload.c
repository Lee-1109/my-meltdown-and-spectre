#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[256*4096];
int temp;
unsigned char secret = 94;
/* 假设Cache hit的阈值为80*/
#define CACHE_HIT_THRESHOLD (80)
//设置偏移量DELTA 任意0-4096偏移量都会将其所在的整个page加入Cacheline
#define DELTA 1024

static int scores[256];

void victim()
{
  temp = array[secret*4096 + DELTA];
}

void flushSideChannel()
{
  int i;

  // 向数组中写数据将其带入RAM 防止Cpoy on Write
  for (i = 0; i < 256; i++) array[i*4096 + DELTA] = 1;

  //将数组从Cacheline中清除
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
     time1 = __rdtscp(&junk);//返回时间计数器的值
     junk = *addr;//获取这个地址内存中的值
     time2 = __rdtscp(&junk) - time1;//获取时间差

     //若访问这个地址的时间差小于threshold 则判定cache hit
     if (time2 <= CACHE_HIT_THRESHOLD){
         printf("array[%d*4096 + %d] is in cache.\n",i,DELTA);
         printf("The Secret = %d.\n",i);
     }
  }	
}
void reloadSideChannelImproved()
{
  int i;
  volatile uint8_t *addr;
  register uint64_t time1, time2;
  int junk = 0;
  for (i = 0; i < 256; i++) {
     addr = &array[i * 4096 + DELTA];
     time1 = __rdtscp(&junk);
     junk = *addr;
     time2 = __rdtscp(&junk) - time1;
     if (time2 <= CACHE_HIT_THRESHOLD)
        scores[i]++; /* if cache hit, add 1 for this value */
        
  }
}

int main(int argc, const char **argv) 
{
  flushSideChannel();
  victim();
  reloadSideChannel();
  return (0);
}
