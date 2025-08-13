#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file.h"

#define POSIX_FILE(file) posix_file_t *posix_file = (posix_file_t *) (file)
#define POSIX_DIR(dir)   posix_dir_t *posix_dir = (posix_dir_t *) (dir)

typedef struct
{
  file_t base;
  file_oflags_t oflags;
  int fd;
} posix_file_t;

typedef struct
{
  dir_t base;
  DIR *dir;
} posix_dir_t;

static dir_t *posix_file_opendir (file_t *file);
static int posix_file_get_type (file_t *file, file_type_t *type);
static int posix_file_get_size (file_t *file, size_t *size);
static int posix_file_seek (file_t *file, size_t off, file_seek_t origin);
static ssize_t posix_file_read (file_t *file, void *buf, size_t nbytes);
static ssize_t posix_file_write (file_t *file, const void *buf, size_t nbytes);
static void posix_file_close (file_t *file);

static dentry_t *posix_dir_readdir (dir_t *dir);
static void posix_dir_rewinddir (dir_t *dir);
static void posix_dir_closedir (dir_t *dir);

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

  file->base.opendir = posix_file_opendir;
  file->base.get_type = posix_file_get_type;
  file->base.get_size = posix_file_get_size;
  file->base.seek = posix_file_seek;
  file->base.read = posix_file_read;
  file->base.write = posix_file_write;
  file->base.close = posix_file_close;

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

static dir_t *
posix_file_opendir (file_t *file)
{
  POSIX_FILE (file);
  posix_dir_t *dir;
  DIR *pdir;

  dir = malloc (sizeof (posix_dir_t));
  if (dir == NULL)
    {
      errno = -ENOMEM;
      return NULL;
    }

  pdir = fdopendir (posix_file->fd);
  if (pdir == NULL)
    {
      free (dir);
      return NULL;
    }

  dir->base.readdir = posix_dir_readdir;
  dir->base.rewinddir = posix_dir_rewinddir;
  dir->base.closedir = posix_dir_closedir;

  dir->dir = pdir;

  return &dir->base;
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

  if (S_ISCHR (buf.st_mode))
    {
      _type = FILE_TYPE_CHAR;
      goto end;
    }

  if (S_ISBLK (buf.st_mode))
    {
      _type = FILE_TYPE_BLOCK;
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
  if (posix_file->fd > -1)
    close (posix_file->fd);

  free (posix_file);
}

static dentry_t *
posix_dir_readdir (dir_t *dir)
{
  POSIX_DIR (dir);
  struct dirent *dirent = readdir (posix_dir->dir);
  dentry_t *ent = malloc (sizeof (dentry_t));

  if (ent == NULL)
    {
      errno = -ENOMEM;
      return NULL;
    }

  ent->name = dirent->d_name;

  switch (dirent->d_type)
    {
    case DT_REG:
      ent->type = FILE_TYPE_FILE;
      break;
    case DT_DIR:
      ent->type = FILE_TYPE_DIR;
      break;
    case DT_CHR:
      ent->type = FILE_TYPE_CHAR;
      break;
    case DT_BLK:
      ent->type = FILE_TYPE_BLOCK;
      break;
    case DT_LNK:
      ent->type = FILE_TYPE_SYM;
      break;
    case DT_FIFO:
      ent->type = FILE_TYPE_PIPE;
      break;
    case DT_SOCK:
      ent->type = FILE_TYPE_SOCK;
      break;
    default:
      ent->type = FILE_TYPE_UNKN;
      break;
    }

  return ent;
}

static void
posix_dir_rewinddir (dir_t *dir)
{
  POSIX_DIR (dir);
  rewinddir (posix_dir->dir);
}

static void
posix_dir_closedir (dir_t *dir)
{
  POSIX_DIR (dir);
  closedir (posix_dir->dir);
  free (posix_dir);
}