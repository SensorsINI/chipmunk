#ifndef MARKRELEASE_H
#define MARKRELEASE_H

#ifdef MARKRELEASE_G
# define vextern
#else
# define vextern extern
#endif

struct record;
extern void mark(struct record **state);
extern void release(struct record **state);
extern char *fakemalloc(long size);

#define malloc(x) fakemalloc(x)

#undef vextern

#endif
