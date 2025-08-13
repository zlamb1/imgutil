#include "file.h"
#include "fs.h"
#ifndef EXT4_H
#define EXT4_H 1

#include <stdint.h>

#define EXT2_MAGIC      0xef53
#define EXT2_ROOT_INODE 2

typedef enum
{
  EXT2_FS_STATE_VALID = 1,
  EXT2_FS_STATE_ERROR = 2
} ext2_fs_state_t;

typedef enum
{
  EXT2_ERR_CONT = 1,
  EXT2_ERR_RO = 2,
  EXT2_ERR_PANIC = 3
} ext2_err_t;

typedef enum
{
  EXT2_OPT_FLAG_PREALLOC = 1,
  EXT2_OPT_FLAG_AFS_SERVER_INODES = 0x2,
  EXT2_OPT_FLAG_HAS_JOURNAL = 0x4,
  EXT2_OPT_FLAG_INODES_EXT_ATTRIBS = 0x8,
  EXT2_OPT_FLAG_FS_RESIZABLE = 0x10,
  EXT2_OPT_FLAG_DIRS_USE_HASH_IDX = 0x20,
} ext2_opt_flag_t;

typedef enum
{
  EXT2_REQ_FLAG_COMPRESSION_USED = 0x1,
  EXT2_REQ_FLAG_DIR_ENTS_HAVE_TYPE = 0x2,
  EXT2_REQ_FLAG_REPLAY_JOURNAL = 0x4,
  EXT2_REQ_FLAG_USES_JOURNAL = 0x8,
} ext2_req_flag_t;

typedef enum
{
  EXT2_RDO_FLAG_SPARSE_SB = 0x1,
  EXT2_RDO_FLAG_64_BIT_FILE_SIZE = 0x2,
  EXT2_RDO_FLAG_DIRS_USE_BINARY_TREE = 0x4,
} ext2_rdonly_flag_t;

typedef enum
{
  EXT2_INODE_PERM_OEXEC = 0x1,
  EXT2_INODE_PERM_OWRITE = 0x2,
  EXT2_INODE_PERM_OREAD = 0x4,
  EXT2_INODE_PERM_GEXEC = 0x10,
  EXT2_INODE_PERM_GWRITE = 0x20,
  EXT2_INODE_PERM_GREAD = 0x40,
  EXT2_INODE_PERM_UEXEC = 0x100,
  EXT2_INODE_PERM_UWRITE = 0x200,
  EXT2_INODE_PERM_UREAD = 0x400,
  EXT2_INODE_PERM_STICKY_BIT = 0x1000,
  EXT2_INODE_PERM_SETGID = 0x2000,
  EXT2_INODE_PERM_SETUID = 0x4000,
} ext2_inode_perms_t;

typedef enum
{
  EXT2_INODE_FLAG_SYNC = 0x8,
  EXT2_INODE_FLAG_IMMUT = 0x10,
  EXT2_INODE_FLAG_APPEND_ONLY = 0x20,
  EXT2_INODE_FLAG_EXCL_DUMP = 0x40,
  EXT2_INODE_FLAG_NO_UPDATE_LAST_ACC = 0x80,
  EXT2_INODE_FLAG_HASH_IDX_DIR = 0x10000,
  EXT2_INODE_FLAG_AFS_DIR = 0x20000,
  EXT2_INODE_FLAG_JOURNAL_FDATA = 0x40000
} ext2_inode_flags_t;

typedef enum
{
  EXT2_INODE_TYPE_FIFO = 0x1000,
  EXT2_INODE_TYPE_CHR_DEV = 0x2000,
  EXT2_INODE_TYPE_DIR = 0x4000,
  EXT2_INODE_TYPE_BLK_DEV = 0x6000,
  EXT2_INODE_TYPE_REG_FILE = 0x8000,
  EXT2_INODE_TYPE_SYM_LINK = 0xA000,
  EXT2_INODE_TYPE_SOCK = 0xC000
} ext2_inode_type_t;

typedef struct
{
  uint32_t inode_cnt;
  uint32_t block_cnt;
  uint32_t su_block_cnt;
  uint32_t free_block_cnt;
  uint32_t free_inode_cnt;
  uint32_t first_block;
  uint32_t log2_block_size;
  uint32_t log2_frag_size;
  uint32_t blocks_per_group;
  uint32_t frags_per_group;
  uint32_t inodes_per_group;
  uint32_t prev_mnt_time;
  uint32_t prev_mod_time;
  uint16_t mnt_cnt;
  uint16_t max_mnt_cnt;
  uint16_t magic;
  uint16_t state;
  uint16_t err_res;
  uint16_t minor_ver;
  uint32_t prev_fsck_time;
  uint32_t check;
  uint32_t os_id;
  uint32_t major_ver;
  uint16_t res_user_id;
  uint16_t res_group_id;
  uint32_t first_inode;
  uint16_t inode_size;
  uint16_t sb_block;
  uint32_t opt_flags;
  uint32_t req_flags;
  uint32_t rdo_flags;
  uint8_t uuid[16];
  char volume_name[16];
  char prev_mnt_name[64];
  uint32_t cmp_algos;
  uint8_t prealloc_blocks;
  uint8_t dir_prealloc_blocks;
  uint16_t align1;
  uint8_t journal_uuid[16];
  uint32_t journal_inode;
  uint32_t journal_dev;
  uint32_t head_orphan;
  uint32_t hash_seed[4];
  uint8_t def_hash_ver;
  uint8_t align2[3];
  uint32_t def_mnt_opts;
  uint32_t first_meta_bg;
} ext2_sb_t;

typedef struct
{
  uint32_t block_bitmap;
  uint32_t inode_bitmap;
  uint32_t inode_table;
  uint16_t num_free_blks;
  uint16_t num_free_inodes;
  uint16_t pad;
  uint8_t res[12];
} ext2_bgdt_t;

typedef struct
{
  uint16_t mode;
  uint16_t uid;
  uint32_t nbytes_lo;
  uint32_t last_access_time;
  uint32_t creation_time;
  uint32_t last_mod_time;
  uint32_t deletion_time;
  uint16_t grp_id;
  uint16_t num_hard_links;
  uint32_t num_sectors;
  uint32_t flags;
  uint32_t os_res1;
  uint32_t block[15];
  uint32_t gen_num;
  uint32_t ext_attrib_blk;
  uint32_t nbytes_hi;
  uint32_t frag_blk_addr;
  uint8_t os_res2[12];
} ext2_inode_t;

typedef struct
{
  size_t block_group_cnt;
  size_t block_size;
  size_t inode_size;
  size_t first_block;
  ext2_sb_t *sb;
  size_t bgdt_size;
  ext2_bgdt_t *bgdt;
  ext2_inode_t *root_inode;
  file_t *file;
  fs_t fs;
} ext2_fs_t;

fs_t *ext2_fs_init (file_t *file, fs_init_error_t *error);
void ext2_fs_fini (fs_t *fs);

#endif