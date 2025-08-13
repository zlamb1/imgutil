#include <errno.h>
#include <sys/cdefs.h>
#ifndef IO_H
#define IO_H 1

#include <stddef.h>

#define ssize_t signed long

#ifndef __always_inline
#define __always_inline __attribute__ ((always_inline))
#endif

typedef enum
{
  FILE_TYPE_UNKN, /* unknown */
  FILE_TYPE_FILE,
  FILE_TYPE_DIR,
  FILE_TYPE_CHAR,
  FILE_TYPE_BLOCK,
  FILE_TYPE_SYM, /* symbolic link */
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

typedef struct file
{
  struct dir *(*opendir) (struct file *file);
  int (*get_type) (struct file *file, file_type_t *type);
  int (*get_size) (struct file *file, size_t *size);
  int (*seek) (struct file *file, size_t off, file_seek_t seek);
  ssize_t (*read) (struct file *file, void *buf, size_t nbytes);
  ssize_t (*write) (struct file *file, const void *buf, size_t nbytes);
  void (*close) (struct file *file);
} file_t;

typedef struct dentry
{
  file_type_t type;
  char *name;
} dentry_t;

typedef struct dir
{
  dentry_t *(*readdir) (struct dir *dir);
  void (*rewinddir) (struct dir *dir);
  void (*closedir) (struct dir *dir);
} dir_t;

file_t *file_open (const char *name, file_oflags_t flags);

__always_inline static dir_t *
file_open_dir (file_t *file)
{
  if (file->opendir == NULL)
    {
      errno = -ENOSYS;
      return NULL;
    }
  return file->opendir (file);
}

__always_inline static int
file_get_type (file_t *file, file_type_t *type)
{
  if (file->get_type == NULL)
    {
      errno = -ENOSYS;
      return -1;
    }
  return file->get_type (file, type);
}

__always_inline static int
file_get_size (file_t *file, size_t *size)
{
  if (file->get_size == NULL)
    {
      errno = -ENOSYS;
      return -1;
    }
  return file->get_size (file, size);
}

__always_inline static int
file_seek (file_t *file, size_t off, file_seek_t origin)
{
  if (file->seek == NULL)
    {
      errno = -ENOSYS;
      return -1;
    }
  return file->seek (file, off, origin);
}

__always_inline static ssize_t
file_read (file_t *file, void *buf, size_t nbytes)
{
  if (file->read == NULL)
    {
      errno = -ENOSYS;
      return -1;
    }
  return file->read (file, buf, nbytes);
}

__always_inline static ssize_t
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

__always_inline static ssize_t
file_write (file_t *file, const void *buf, size_t nbytes)
{
  if (file->write == NULL)
    {
      errno = -ENOSYS;
      return -1;
    }

  return file->write (file, buf, nbytes);
}

__always_inline static ssize_t
file_swrite (file_t *file, size_t off, file_seek_t origin, const void *buf,
             size_t nbytes)
{
  ssize_t write;

  if (file_seek (file, off, origin) == -1)
    return -1;

  if ((write = file_write (file, buf, nbytes)) == -1)
    return -2;

  return write;
}

__always_inline static void
file_close (file_t *file)
{
  if (file->close == NULL)
    {
      errno = -ENOSYS;
      return;
    }
  file->close (file);
}

__always_inline static dentry_t *
dir_readdir (dir_t *dir)
{
  if (dir->readdir == NULL)
    {
      errno = -ENOSYS;
      return NULL;
    }
  return dir->readdir (dir);
}

__always_inline static void
dir_rewinddir (dir_t *dir)
{
  if (dir->rewinddir == NULL)
    {
      errno = -ENOSYS;
      return;
    }
  dir->rewinddir (dir);
}

__always_inline static void
dir_closedir (dir_t *dir)
{
  if (dir->closedir == NULL)
    {
      errno = -ENOSYS;
      return;
    }
  dir->closedir (dir);
}

#endif