#ifndef MINIZIPGLOBAL_H
#define MINIZIPGLOBAL_H

#if defined(WIN32)
#if defined(MINIZIP_LIBRARY)
#  define  __declspec(dllexport)
#else
#  define  __declspec(dllimport)
#endif
#else
#  define 
#endif

#endif // MINIZIPGLOBAL_H
