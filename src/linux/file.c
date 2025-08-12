#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file.h"

typedef struct file
{
  file_oflags_t oflags;
  int fd;
} file_t;

file_t *
file_open (const char *name, file_oflags_t flags)
{
  int _flags = 0;
  file_t *file = malloc (sizeof (file_t));

  if (file == NULL)
    {
      errno = -ENOMEM;
      return NULL;
    }

  if (flags & FILE_ORDONLY)
    _flags |= O_RDONLY;

  if (flags & FILE_OWRONLY)
    _flags |= O_WRONLY;

  if (flags & FILE_ORDWR)
    _flags |= O_RDWR;

  file->oflags = flags;
  file->fd = open (name, _flags);

  if (file->fd == -1)
    {
      free (file);
      return NULL;
    }

  return file;
}

void
file_close (file_t *file)
{
  if (file->fd == -1)
    return;
  close (file->fd);
  file->fd = -1;
}

void
file_destroy (file_t *file)
{
  file_close (file);
  free (file);
}

int
file_get_type (file_t *file, file_type_t *type)
{
  struct stat buf;
  file_type_t _type = FILE_TYPE_UNKN;

  if (fstat (file->fd, &buf) == -1)
    return -1;

  if (S_ISREG (buf.st_mode))
    {
      _type = FILE_TYPE_FILE;
      goto end;
    }

  if (S_ISDIR (buf.st_mode))
    {
      _type = FILE_TYPE_DIR;
      goto end;
    }

  if (S_ISCHR (buf.st_mode) || S_ISBLK (buf.st_mode))
    {
      _type = FILE_TYPE_SPEC;
      goto end;
    }

  if (S_ISLNK (buf.st_mode))
    {
      _type = FILE_TYPE_SYM;
      goto end;
    }

  if (S_ISFIFO (buf.st_mode))
    {
      _type = FILE_TYPE_PIPE;
      goto end;
    }

  if (S_ISSOCK (buf.st_mode))
    _type = FILE_TYPE_SOCK;

end:
  *type = _type;
  return 0;
}

int
file_get_size (file_t *file, size_t *size)
{
  struct stat buf;
  file_type_t _type = FILE_TYPE_UNKN;

  if (fstat (file->fd, &buf) == -1)
    return -1;

  *size = buf.st_size;
  return 0;
}

int
file_seek (file_t *file, size_t off, file_seek_t origin)
{
  int whence;

  switch (origin)
    {
    case FILE_SEEK_START:
      whence = SEEK_SET;
      break;
    case FILE_SEEK_CUR:
      whence = SEEK_CUR;
      break;
    case FILE_SEEK_END:
      whence = SEEK_END;
      break;
    }

  return lseek (file->fd, off, whence);
}

ssize_t
file_read (file_t *file, void *buf, size_t nbytes)
{
  return read (file->fd, buf, nbytes);
}

ssize_t
file_write (file_t *file, void *buf, size_t nbytes)
{
  return write (file->fd, buf, nbytes);
}