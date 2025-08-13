#ifndef FS_H
#define FS_H 1

typedef struct
{
  void *data;
} fs_t;

typedef struct
{
  union
  {
    const char *const_error;
    char *alloc_error;
  };
  char *error;
  int allocated;
} fs_init_error_t;

#endif