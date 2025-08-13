#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file.h"

#define POSIX_FILE(file) posix_file_t *posix_file = (posix_file_t *) (file)

typedef struct
{
  file_t base;
  file_oflags_t oflags;
  int fd;
} posix_file_t;

static int posix_file_get_type (file_t *file, file_type_t *type);
static int posix_file_get_size (file_t *file, size_t *size);
static int posix_file_seek (file_t *file, size_t off, file_seek_t origin);
static ssize_t posix_file_read (file_t *file, void *buf, size_t nbytes);
static ssize_t posix_file_write (file_t *file, const void *buf, size_t nbytes);
static void posix_file_close (file_t *file);
static void posix_file_destroy (file_t *file);

file_t *
file_open (const char *name, file_oflags_t flags)
{
  int _flags = 0;
  posix_file_t *file = malloc (sizeof (posix_file_t));

  if (file == NULL)
    {
      errno = -ENOMEM;
      return NULL;
    }

  memset (file, 0, sizeof (posix_file_t));

  file->base.get_type = posix_file_get_type;
  file->base.get_size = posix_file_get_size;
  file->base.seek = posix_file_seek;
  file->base.read = posix_file_read;
  file->base.write = posix_file_write;
  file->base.close = posix_file_close;
  file->base.destroy = posix_file_destroy;

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

  return &file->base;
}

static int
posix_file_get_type (file_t *file, file_type_t *type)
{
  POSIX_FILE (file);
  struct stat buf;
  file_type_t _type = FILE_TYPE_UNKN;

  if (fstat (posix_file->fd, &buf) == -1)
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

static int
posix_file_get_size (file_t *file, size_t *size)
{
  POSIX_FILE (file);
  struct stat buf;

  if (fstat (posix_file->fd, &buf) == -1)
    return -1;

  *size = buf.st_size;
  return 0;
}

static int
posix_file_seek (file_t *file, size_t off, file_seek_t origin)
{
  POSIX_FILE (file);
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
    default:
      errno = -EINVAL;
      return -1;
    }

  return lseek (posix_file->fd, off, whence);
}

static ssize_t
posix_file_read (file_t *file, void *buf, size_t nbytes)
{
  POSIX_FILE (file);
  return read (posix_file->fd, buf, nbytes);
}

static ssize_t
posix_file_write (file_t *file, const void *buf, size_t nbytes)
{
  POSIX_FILE (file);
  return write (posix_file->fd, buf, nbytes);
}

static void
posix_file_close (file_t *file)
{
  POSIX_FILE (file);
  if (posix_file->fd == -1)
    return;
  close (posix_file->fd);
  posix_file->fd = -1;
}

static void
posix_file_destroy (file_t *file)
{
  file_close (file);
  free (file);
}