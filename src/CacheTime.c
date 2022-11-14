#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>
/**
 * 该文件检验数据Cache访问和memory访问的 CPU cycle不同
*/
uint8_t array[10*4096];

int main(int argc, const char **argv) {
  int junk=0;
  register uint64_t time1, time2;
  volatile uint8_t *addr;
  int i;
  
  // 初始化arrayy以免内存不分配空间
  for(i=0; i<10; i++) array[i*4096]=1;

  // 将Cache中array的值flush
  for(i=0; i<10; i++) _mm_clflush(&array[i*4096]);

  // 访问一些基本的数组
  array[3*4096] = 100;
  array[7*4096] = 200;
  array[10*4096] = 255;

  for(i=0; i<10; i++) {
    addr = &array[i*4096];
    time1 = __rdtscp(&junk);                
    junk = *addr;
    time2 = __rdtscp(&junk) - time1;       
    printf("array[%d*4096] 访问时间: %d CPU cycles\n",i, (int)time2);
  }
  return 0;
}