#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ext2.h"
#include "file.h"

#define ALIGN_UP(X, ALIGN) (((X) + ((ALIGN) - 1)) / (ALIGN) * (ALIGN))

#define ERROR(error, msg)                                                     \
  do                                                                          \
    {                                                                         \
      if ((error) != NULL)                                                    \
        {                                                                     \
          (error)->error = (msg);                                             \
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
              (error)->error = "out of memory";                               \
              (error)->allocated = 0;                                         \
            }                                                                 \
          else                                                                \
            {                                                                 \
              (error)->error = buf;                                           \
              (error)->allocated = 1;                                         \
            }                                                                 \
        }                                                                     \
      goto cleanup;                                                           \
    }                                                                         \
  while (0)

static int
ext2_has_extended_sb (ext2_superblock_t *sb)
{
  return sb->major_ver >= 1;
}

fs_t *
ext2_fs_init (file_t *file, fs_init_error_t *error)
{
  ext2_fs_t *fs;
  ext2_superblock_t *sb;
  size_t size, blk_size;

  if (file_get_size (file, &size) == -1)
    ERROR (error, "failed to read file size");

  if (size < 2048)
    ERROR (error, "image too small for superblock");

  sb = malloc (1024);
  if (sb == NULL)
    ERROR (error, "out of memory");

  if (file_sread (file, 1024, FILE_SEEK_START, sb, 1024) != 1024)
    ERROR (error, "failed to superblock from file");

  if (sb->ext2_sig != EXT2_MAGIC)
    ERROR (error, "invalid ext2 signature in superblock");

  fs = malloc (sizeof (ext2_fs_t));
  if (fs == NULL)
    ERROR (error, "out of memory");

  fs->sb = sb;

  /* we only permit block sizes of up to 8192 */
  if (sb->log2_blk_size > 3)
    ERROR (error, "invalid block size");

  fs->blk_size = 1024 << sb->log2_blk_size;

  if (sb->blks_per_grp == 0)
    ERROR (error, "invalid blocks per group");

  if (sb->inodes_per_grp == 0)
    ERROR (error, "invalid inodes per group");

  fs->num_blk_grps
      = ALIGN_UP (sb->num_blks, sb->blks_per_grp) / sb->blks_per_grp;

  if (fs->num_blk_grps
      != ALIGN_UP (sb->num_inodes, sb->inodes_per_grp) / sb->inodes_per_grp)
    ERROR (error, "inconsistent total block groups between blocks and inodes");

  if (ext2_has_extended_sb (sb))
    {
      fs->inode_size = sb->inode_size;
      if (fs->inode_size < 128)
        ALLOC_ERROR (error, "invalid inode size: %zu", fs->inode_size);

      fs->start_blk = sb->blk_grp_of_sb;
    }
  else
    {
      fs->inode_size = 128;
      fs->start_blk = 0;
    }

  return &fs->fs;

cleanup:
  if (sb != NULL)
    free (sb);

  if (fs != NULL)
    free (fs);

  return NULL;
}

void
ext2_fs_fini (fs_t *_fs)
{
  ext2_fs_t *fs = (ext2_fs_t *) _fs;
  free (fs->sb);
  free (fs);
}