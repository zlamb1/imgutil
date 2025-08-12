#ifndef FS_H
#define FS_H 1

typedef struct
{
  void *data;
} fs_t;

typedef struct
{
  char *error;
  int allocated;
} fs_init_error_t;

#endif