#include <stdio.h>
#include <setjmp.h>
#include <signal.h>

static sigjmp_buf jbuf;

static void catch_segv()
{
  // 回滚回sigsetjmp()设置的检查点
  siglongjmp(jbuf, 1);                         
}

//使用异常处理
void processWithExecptionHandling(){
  unsigned long kernel_data_addr = 0xfb61b000;
  // Register a signal handler
  signal(SIGSEGV, catch_segv);                     

  if (sigsetjmp(jbuf, 1) == 0) {                
     // A SIGSEGV signal will be raised. 
     char kernel_data = *(char*)kernel_data_addr; 
     // 下面这行代码并不会被执行
     printf("在内地址%lu 的数据为: %c\n", kernel_data_addr, kernel_data);
  }
  else {
     printf("内存访问违法!\n");
  }
  printf("进程继续执行.\n");
}

//不使用异常处理
void processWithoutExceptionHandling(){
    unsigned long kernel_data_addr = 0xfb61b000;
    char kernel_data = *(char*)kernel_data_addr;
    printf("内核地址%lu 的数据为: %c\n", kernel_data_addr, kernel_data);
}



int main()
{ 
    //processWithExecptionHandling();
    processWithoutExceptionHandling();
    return 0;
}
