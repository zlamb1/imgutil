#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext2.h"
#include "file.h"

#define ALIGN_UP(X, ALIGN) (((X) + ((ALIGN) - 1)) / (ALIGN) * (ALIGN))

#define ERROR(error, msg)                                                     \
  do                                                                          \
    {                                                                         \
      if ((error) != NULL)                                                    \
        {                                                                     \
          (error)->const_error = (msg);                                       \
          (error)->allocated = 0;                                             \
        }                                                                     \
      goto cleanup;                                                           \
    }                                                                         \
  while (0)

#define ALLOC_ERROR(error, fmt, ...)                                          \
  do                                                                          \
    {                                                                         \
      if ((error) != NULL)                                                    \
        {                                                                     \
          char *buf;                                                          \
          if (asprintf (&buf, (fmt), __VA_ARGS__) == -1)                      \
            {                                                                 \
              (error)->const_error = "out of memory";                         \
              (error)->allocated = 0;                                         \
            }                                                                 \
          else                                                                \
            {                                                                 \
              (error)->alloc_error = buf;                                     \
              (error)->allocated = 1;                                         \
            }                                                                 \
        }                                                                     \
      goto cleanup;                                                           \
    }                                                                         \
  while (0)

static int
ext2_has_extended_sb (ext2_sb_t *sb)
{
  return sb->major_ver >= 1;
}

static int
ext2_read_from_block (ext2_fs_t *fs, size_t block, size_t off, void *buf,
                      size_t nbytes)
{
  if (block >= fs->sb->block_cnt)
    return -3;

  return file_sread (fs->file, block * fs->block_size + off, FILE_SEEK_START,
                     buf, nbytes);
}

static ext2_inode_t *
ext2_read_inode (ext2_fs_t *fs, size_t inode)
{
  size_t block_group = (inode - 1) / fs->sb->inodes_per_group;
  ext2_bgdt_t *bgdt;
  ext2_inode_t *_inode = NULL;
  size_t off;

  if (!inode || inode > fs->sb->inode_cnt
      || block_group >= fs->block_group_cnt)
    {
      errno = -EINVAL;
      return NULL;
    }

  bgdt = fs->bgdt + block_group;
  _inode = malloc (fs->inode_size);

  if (_inode == NULL)
    {
      errno = -ENOMEM;
      goto cleanup;
    }

  off = fs->inode_size * ((inode - 1) % fs->sb->inodes_per_group);
  if (ext2_read_from_block (fs, bgdt->inode_table, off, _inode, fs->inode_size)
      != (ssize_t) fs->inode_size)
    {
      errno = -EINVAL;
      goto cleanup;
    }

  return _inode;

cleanup:
  if (_inode != NULL)
    free (_inode);

  return NULL;
}

fs_t *
ext2_fs_init (file_t *file, fs_init_error_t *error)
{
  ext2_fs_t *fs;
  ext2_sb_t *sb;
  ext2_inode_t *root_inode;
  size_t size;

  if (file_get_size (file, &size) == -1)
    ERROR (error, "failed to read file size");

  if (size < 2048)
    ERROR (error, "image too small for superblock");

  sb = malloc (1024);
  if (sb == NULL)
    ERROR (error, "out of memory");

  if (file_sread (file, 1024, FILE_SEEK_START, sb, 1024) != 1024)
    ERROR (error, "failed to read superblock");

  if (sb->magic != EXT2_MAGIC)
    ERROR (error, "invalid ext2 signature in superblock");

  fs = malloc (sizeof (ext2_fs_t));
  if (fs == NULL)
    ERROR (error, "out of memory");

  memset (fs, 0, sizeof (ext2_fs_t));
  fs->sb = sb;
  fs->file = file;

  /* we only permit block sizes of up to 8192 */
  if (sb->log2_block_size > 3)
    ERROR (error, "invalid block size");

  fs->block_size = 1024 << sb->log2_block_size;

  if (size < sb->block_cnt * fs->block_size)
    ERROR (error, "filesystem size exceeds file size");

  if (sb->blocks_per_group == 0)
    ERROR (error, "invalid blocks per group");

  if (sb->inodes_per_group == 0)
    ERROR (error, "invalid inodes per group");

  fs->block_group_cnt
      = ALIGN_UP (sb->block_cnt, sb->blocks_per_group) / sb->blocks_per_group;

  if (fs->block_group_cnt
      != ALIGN_UP (sb->inode_cnt, sb->inodes_per_group) / sb->inodes_per_group)
    ERROR (error, "inconsistent total block groups between blocks and inodes");

  if (ext2_has_extended_sb (sb))
    {
      fs->inode_size = sb->inode_size;
      if (fs->inode_size < 128)
        ALLOC_ERROR (error, "invalid inode size: %zu", fs->inode_size);

      fs->first_block = sb->sb_block;
    }
  else
    {
      fs->inode_size = 128;
      fs->first_block = 0;
    }

  fs->bgdt_size = fs->block_group_cnt * sizeof (ext2_bgdt_t);
  fs->bgdt = malloc (fs->bgdt_size);
  if (fs->bgdt == NULL)
    ERROR (error, "out of memory");

  if (ext2_read_from_block (fs, fs->block_size == 1024 ? 2 : 1, 0, fs->bgdt,
                            fs->bgdt_size)
      != (ssize_t) fs->bgdt_size)
    ERROR (error, "failed to read block group descriptor table");

  root_inode = ext2_read_inode (fs, EXT2_ROOT_INODE);
  if (root_inode == NULL)
    ERROR (error, "failed to read root inode");

  if (!(root_inode->mode & EXT2_INODE_TYPE_DIR))
    ERROR (error, "root inode is not directory");

  fs->root_inode = root_inode;

  fs->fs.data = fs;
  return &fs->fs;

cleanup:
  if (sb != NULL)
    free (sb);

  if (fs != NULL)
    {
      if (fs->bgdt != NULL)
        free (fs->bgdt);

      if (fs->root_inode != NULL)
        free (fs->root_inode);

      free (fs);
    }

  return NULL;
}

void
ext2_fs_fini (fs_t *_fs)
{
  ext2_fs_t *fs = (ext2_fs_t *) _fs->data;
  free (fs->sb);
  free (fs->bgdt);
  free (fs->root_inode);
  free (fs);
}