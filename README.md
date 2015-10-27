# Memory-Leak-Detector-For-C
Valgrind works really well... if you are using Linux and have gotten it set up.  So, if we can avoid that, lets take the simpler approach and just drop in some headers!

# Overview
This is a simple memory leak detector for C90 and up!  Simple because it will only catch
uses of malloc, calloc, realloc, and free.  Depending on what is used in your project,
you may want to extend the list of leaky functions to look for, say... fopen and fclose.

# Strange Errors?
If you get any strange errors, the first thing you should try is renaming the #defines to a unique name
(my_malloc) and use those.  Some standard libraries, like Cygwin stdlib.h, mark these memory functions as
'no_throw'.  But, with modern IDEs you should be able to replace all 'free', for example, with 'my_free', quite
easily.  If you really want to override the original names (in this case for Cygwin) you will have to delete the
no_throw definitions in Cygwin's stdlib.h and import malloc from malloc.h instead of stdlib.h.

# Usage
1. include memory_leak_test.h and memory_leak_test.c in your project
2. put 'include "memory_leak_test.h"' as the last include of every source file
3. Whenever you want a status report, run 'leak_check_run_and_print()'
4. run your program!

# Example
    ```C
    /* other.h */
    void memory_thieving_function(void);

    /* other.c */
    #include "leaky_function.h"
    #include <stdlib.h>
    #include <stdio.h>
    #include "memory_leak_test.h"
    
    void memory_thieving_function(void) {
      char* forgotten_buffer = malloc(sizeof(char) * 50);
      printf("Whoops!\n");
    }

    /* main.c */
    #include "other.h"
    #include "memory_leak_test.h"
    
    int main() {
      memory_thieving_function();
      int* even_better = malloc(sizeof(int) * 9000);
      leak_check_run_and_print();
    }
    ```
    
    /* console output */
    Whoops!
    WARNING: leaked '50' bytes at address '0x600010378' in file 'main.c' on line '40' after 0.01s
    WARNING: leaked '36000' bytes at address '0x600048488' in file 'main.c' on line '46' after 0.01s
    MEMORY REPORT: '2' alloc(s) made.  '2' not freed (total of '36050' bytes leaked).
    Process finished with exit code 0
