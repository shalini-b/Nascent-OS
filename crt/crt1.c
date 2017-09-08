#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
//   main(0, NULL, NULL);

  long resp;
  int a;
   __asm__ __volatile__(
                            "movq %%rsp, %0"
                             :"=r" (resp)
                             :
                             :"%rsp");

  a=*((int*)(resp+8));
  char** b=(char**)(resp+16);
  char** c=(char**)(a*8+8+resp+16);
  main(a,b,c);
  exit(0); 
}
