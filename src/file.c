#include "file.h"

ssize_t
file_sread (file_t *file, size_t off, file_seek_t origin, void *buf,
            size_t nbytes)
{
  ssize_t read;

  if (file_seek (file, off, origin) == -1)
    return -1;

  if ((read = file_read (file, buf, nbytes)) == -1)
    return -2;

  return read;
}

ssize_t
file_swrite (file_t *file, size_t off, file_seek_t origin, void *buf,
             size_t nbytes)
{
  ssize_t write;

  if (file_seek (file, off, origin) == -1)
    return -1;

  if ((write = file_write (file, buf, nbytes)) == -1)
    return -2;

  return write;
}