#include "file.h"
#include "fs.h"
#ifndef EXT4_H
#define EXT4_H 1

#include <stdint.h>

#define EXT2_MAGIC 0xef53

typedef enum
{
  EXT2_FS_STATE_CLEAN = 1,
  EXT2_FS_STATE_ERR = 2
} ext2_fs_state_t;

typedef enum
{
  EXT2_ERR_RES_CONTINUE = 1,
  EXT2_ERR_RES_REMOUNT_AS_RDONLY = 2,
  EXT2_ERR_RES_PANIC = 3
} ext2_err_response_t;

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
  EXT2_RDONLY_FLAG_SPARSE_SB = 0x1,
  EXT2_RDONLY_FLAG_64_BIT_FILE_SIZE = 0x2,
  EXT2_RDONLY_FLAG_DIRS_USE_BINARY_TREE = 0x4,
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
  uint32_t num_inodes;
  uint32_t num_blks;
  uint32_t num_su_blks;
  uint32_t num_free_blks;
  uint32_t num_free_inodes;
  uint32_t start_blk;
  uint32_t log2_blk_size;
  uint32_t log2_frag_size;
  uint32_t blks_per_grp;
  uint32_t frags_per_grp;
  uint32_t inodes_per_grp;
  uint32_t last_mnt_time; /* POSIX time */
  uint32_t last_mod_time; /* POSIX time */
  uint16_t num_mnts_since_fsck;
  uint16_t num_mnts_req_fsck;
  uint16_t ext2_sig;
  uint16_t fs_state;
  uint16_t err_res;
  uint16_t minor_ver;
  uint32_t last_fsck_time; /* POSIX time */
  uint32_t fsck_interval;
  uint32_t os_id;
  uint32_t major_ver;
  uint16_t res_user_id;
  uint16_t res_grp_id;
  /* extended fields if major ver >= 1 */
  uint32_t first_nonres_inode;
  uint16_t inode_size;
  uint16_t blk_grp_of_sb;
  uint32_t opt_flags;    /* optional features of fs impl */
  uint32_t req_flags;    /* required features of fs impl to read or write */
  uint32_t rdonly_flags; /* required features of fs impl to read only */
  uint8_t fs_id[16];
  char volume_name[16];
  char last_mnt_name[64];
  uint32_t cmp_algos;
  uint8_t blks_prealloc_per_file;
  uint8_t blks_prealloc_per_dir;
  uint16_t unused;
  uint8_t journal_id[16];
  uint32_t journal_inode;
  uint32_t journal_dev;
  uint32_t head_orphan_inodes;
} ext2_superblock_t;

typedef struct
{
  uint16_t type_and_perms;
  uint16_t user_id;
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
  uint32_t db_ptrs[12];
  uint32_t singly_indirect_blk_ptr;
  uint32_t doubly_indirect_blk_ptr;
  uint32_t triply_indirect_blk_ptr;
  uint32_t gen_num;
  uint32_t ext_attrib_blk;
  uint32_t nbytes_hi;
  uint32_t frag_blk_addr;
  uint8_t os_res2[12];
} ext2_inode_t;

typedef struct
{
  size_t num_blk_grps;
  size_t blk_size;
  size_t inode_size;
  size_t start_blk;
  ext2_superblock_t *sb;
  fs_t fs;
} ext2_fs_t;

fs_t *ext2_fs_init (file_t *file, fs_init_error_t *error);
void ext2_fs_fini (fs_t *fs);

#endif