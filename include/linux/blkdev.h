#ifndef __TOOLS_LINUX_BLKDEV_H
#define __TOOLS_LINUX_BLKDEV_H

#include <linux/backing-dev.h>
#include <linux/blk_types.h>
#include <linux/kobject.h>
#include <linux/types.h>

#define MAX_LFS_FILESIZE 	((loff_t)LLONG_MAX)

#define BIO_MAX_VECS	256U

typedef unsigned fmode_t;
typedef __u32 __bitwise blk_opf_t;

struct bio;
struct user_namespace;

#define MINORBITS	20
#define MINORMASK	((1U << MINORBITS) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))

typedef unsigned int __bitwise blk_mode_t;

/* open for reading */
#define BLK_OPEN_READ		((__force blk_mode_t)(1 << 0))
/* open for writing */
#define BLK_OPEN_WRITE		((__force blk_mode_t)(1 << 1))
/* open exclusively (vs other exclusive openers */
#define BLK_OPEN_EXCL		((__force blk_mode_t)(1 << 2))
/* opened with O_NDELAY */
#define BLK_OPEN_NDELAY		((__force blk_mode_t)(1 << 3))
/* open for "writes" only for ioctls (specialy hack for floppy.c) */
#define BLK_OPEN_WRITE_IOCTL	((__force blk_mode_t)(1 << 4))

#define BLK_OPEN_BUFFERED	((__force blk_mode_t)(1 << 5))

struct inode {
	unsigned long		i_ino;
	loff_t			i_size;
	struct super_block	*i_sb;
};

struct file {
	struct inode		*f_inode;
};

static inline struct inode *file_inode(const struct file *f)
{
	return f->f_inode;
}

#define part_to_dev(part)	(part)

void generic_make_request(struct bio *);
int submit_bio_wait(struct bio *);

static inline void submit_bio(struct bio *bio)
{
	generic_make_request(bio);
}

int blkdev_issue_discard(struct block_device *, sector_t, sector_t, gfp_t);
int blkdev_issue_zeroout(struct block_device *, sector_t, sector_t, gfp_t, unsigned);

#define bdev_get_queue(bdev)		(&((bdev)->queue))

#ifndef SECTOR_SHIFT
#define SECTOR_SHIFT 9
#endif
#ifndef SECTOR_SIZE
#define SECTOR_SIZE (1 << SECTOR_SHIFT)
#endif

#define PAGE_SECTORS_SHIFT	(PAGE_SHIFT - SECTOR_SHIFT)
#define PAGE_SECTORS		(1 << PAGE_SECTORS_SHIFT)
#define SECTOR_MASK		(PAGE_SECTORS - 1)

#define bdev_max_discard_sectors(bdev)	((void) (bdev), 0)
#define blk_queue_nonrot(q)		((void) (q), 0)

unsigned bdev_logical_block_size(struct block_device *bdev);
sector_t get_capacity(struct gendisk *disk);

struct blk_holder_ops {
        void (*mark_dead)(struct block_device *bdev);
};

void blkdev_put(struct block_device *bdev, void *holder);
void bdput(struct block_device *bdev);
struct block_device *blkdev_get_by_path(const char *path, blk_mode_t mode,
					void *holder, const struct blk_holder_ops *hop);
int lookup_bdev(const char *path, dev_t *);

struct super_block {
	void			*s_fs_info;
};

/*
 * File types
 *
 * NOTE! These match bits 12..15 of stat.st_mode
 * (ie "(i_mode >> 12) & 15").
 */
#ifndef DT_UNKNOWN
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14
#define DT_MAX		16
#endif

/*
 * This is the "filldir" function type, used by readdir() to let
 * the kernel specify what kind of dirent layout it wants to have.
 * This allows the kernel to read directories into kernel space or
 * to have different dirent layouts depending on the binary type.
 */
struct dir_context;
typedef int (*filldir_t)(struct dir_context *, const char *, int, loff_t, u64,
			 unsigned);

struct dir_context {
	const filldir_t actor;
	u64 pos;
};

/* /sys/fs */
extern struct kobject *fs_kobj;

struct file_operations {
};

static inline int register_chrdev(unsigned int major, const char *name,
				  const struct file_operations *fops)
{
	return 1;
}

static inline void unregister_chrdev(unsigned int major, const char *name)
{
}

static inline const char *bdevname(struct block_device *bdev, char *buf)
{
	snprintf(buf, BDEVNAME_SIZE, "%s", bdev->name);
	return buf;
}

static inline bool op_is_write(unsigned int op)
{
	return op == REQ_OP_READ ? false : true;
}

/*
 * return data direction, READ or WRITE
 */
static inline int bio_data_dir(struct bio *bio)
{
	return op_is_write(bio_op(bio)) ? WRITE : READ;
}

static inline bool dir_emit(struct dir_context *ctx,
			    const char *name, int namelen,
			    u64 ino, unsigned type)
{
	return ctx->actor(ctx, name, namelen, ctx->pos, ino, type) == 0;
}

static inline bool dir_emit_dots(struct file *file, struct dir_context *ctx)
{
	return true;
}

#define capable(cap)		true

int blk_status_to_errno(blk_status_t status);
blk_status_t errno_to_blk_status(int errno);
const char *blk_status_to_str(blk_status_t status);

#endif /* __TOOLS_LINUX_BLKDEV_H */

