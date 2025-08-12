#ifndef IO_H
#define IO_H 1

#include <stddef.h>

#define ssize_t signed long

typedef enum
{
  FILE_TYPE_UNKN, /* unknown */
  FILE_TYPE_FILE,
  FILE_TYPE_DIR,
  FILE_TYPE_SPEC, /* block / char devices */
  FILE_TYPE_SYM,  /* symbolic link */
  FILE_TYPE_PIPE,
  FILE_TYPE_SOCK
} file_type_t;

typedef enum
{
  FILE_ORDONLY = (1 << 0),
  FILE_OWRONLY = (1 << 1),
  FILE_ORDWR = (1 << 2)
} file_oflags_t;

typedef enum
{
  FILE_SEEK_START,
  FILE_SEEK_CUR,
  FILE_SEEK_END
} file_seek_t;

typedef struct file file_t;

file_t *file_open (const char *name, file_oflags_t flags);
void file_close (file_t *file);
void file_destroy (file_t *file);

int file_get_type (file_t *file, file_type_t *type);
int file_get_size (file_t *file, size_t *size);

int file_seek (file_t *file, size_t off, file_seek_t origin);

ssize_t file_read (file_t *file, void *buf, size_t nbytes);
ssize_t file_sread (file_t *file, size_t off, file_seek_t origin, void *buf,
                    size_t nbytes);

ssize_t file_write (file_t *file, void *buf, size_t nbytes);
ssize_t file_swrite (file_t *file, size_t off, file_seek_t origin, void *buf,
                     size_t nbytes);

#endif