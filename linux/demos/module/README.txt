Zach Halpern
10/22/17

Part C:

The library function of gettimeofday() returns the same value as the kernel function getnstimeofday, with the one exception that instead of showing us the full nano seconds it only goes down to the micro seconds. It rounds the value in order to increase the accuracy as the user level should not have any reason to need nano second differentials. gettimeofday() also has the ability for a timezone to be included, althought it is depreciated.

current_kernel_time() is a timekeeping measure and only gets updated through timer interrupts. This means the value is dependent on the computer's timer interrupt period, so we cannot use it to measure performance.

getnstimeoftday() is meant for timekeeping as it reads the clock of the computer and is more precise. 

Since these two functions are based on different sources, it is reasonable to expect there to be a difference in their readings. In the case of my program, current_kernel_time() reported back every so slightly faster then getnstimeofday, which can only be seen when you compare down to the nano seconds. clock_gettime() has a performance that is very close to that of the kernel's xtime. It is also based on the system's clock resolution, just like the xtime so it's no surprise that they're similar. When called is when they signal for the result, so that's why one is slightly more then the other. 

When I ran the program with N = 10, The time difference from the first read (3391152) to the last read (3441875) was only 50723 nano seconds.
When I ran the program with N = 100000, The time difference between the first read (3980265) and last read (5616380) increased to 1636115 nano seconds. These readings become more and more spaced as the program runs because of the timer interrupts updating the kernel clock and the actual clock updating the get time of day method.

When changing the return value for init_module() to -1, 
$ insmod mytime.ko
insmod: ERROR: could not insert module mytime.ko: Operation not permitted

If you attempt to insert a module as a non-root/sudo user, you will be greated with this error. Only the root user may insert a new module into the kernel. 


