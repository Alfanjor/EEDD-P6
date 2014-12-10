/* 
 * File:   millisleep.h
 * Author: ajrueda
 *
 * Created on 21 de octubre de 2014, 17:22
 */

#if defined(WIN32)
  #include <windows.h>
#elif defined(__linux__) || defined (__APPLE__)
  #include <unistd.h>
#else
#endif

int millisleep(unsigned ms)
{
#if defined(WIN32)
  SetLastError(0);
  Sleep(ms);
  return GetLastError() ?-1 :0;
#elif defined(__linux__) || defined(__APPLE__)
  struct timespec wait;
  wait.tv_sec = ms / 1000;
  wait.tv_nsec = 1000000ul * (ms % 1000); 
  return nanosleep(&wait, 0);
#else
#error ("no milli sleep available for platform")
  return -1;
#endif
}

