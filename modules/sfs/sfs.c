#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/file.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/uaccess.h>

#define SFS_NAME "sfs"
#define SFS_MAGIC 0x3456abcd

/* sfs private structure */
typedef struct {
	u32 _unused;
}__attribute__((packed)) sfs_priv_t;

/* 
 * File Operations Block ....
 */
static unsigned long sfs_mmu_get_unmapped_area(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags)
{
	return current->mm->get_unmapped_area(file, addr, len, pgoff, flags);
}

static const struct file_operations sfs_file_operations = {
	.read_iter	= generic_file_read_iter,
	.write_iter	= generic_file_write_iter,
	.mmap		= generic_file_mmap,
	.fsync		= noop_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.llseek		= generic_file_llseek,
	.get_unmapped_area = sfs_mmu_get_unmapped_area,
};

static const struct inode_operations sfs_file_inode_operations = {
	.setattr	= simple_setattr,
	.getattr	= simple_getattr,
};

static const struct address_space_operations sfs_aops;
static const struct inode_operations sfs_dir_inode_operations;

struct inode *sfs_get_inode(struct super_block *sb,
			const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode * inode = new_inode(sb);

	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, dir, mode);
		inode->i_mapping->a_ops = &sfs_aops;
		mapping_set_gfp_mask(inode->i_mapping, GFP_KERNEL);
		mapping_set_unevictable(inode->i_mapping);
		inode->i_atime = 
		inode->i_mtime = 
		inode->i_ctime = current_time(inode);
		switch (mode & S_IFMT) {
		case S_IFREG:
			inode->i_op = &sfs_file_inode_operations;
			inode->i_fop = &sfs_file_operations;
			break;
		case S_IFDIR:
			inode->i_op = &sfs_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;
			inc_nlink(inode);
			break;
		case S_IFLNK:
			inode->i_op = &page_symlink_inode_operations;
			inode_nohighmem(inode);
			break;
		default:
			init_special_inode(inode, mode, dev);
			break;
		}
	}
	return inode;
}

static int
sfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct inode * inode;
	int error = -ENOSPC;

	inode = sfs_get_inode(dir->i_sb, dir, mode, dev);
	if (inode) {
		d_instantiate(dentry, inode);
		dget(dentry);	/* Extra count - pin the dentry in core */
		error = 0;
		dir->i_mtime = 
		dir->i_ctime = current_time(dir);
	}
	return error;
}

static int 
sfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	return sfs_mknod(dir, dentry, mode | S_IFREG, 0);
}

static int 
sfs_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
{
	int retval;

	retval = sfs_mknod(dir, dentry, mode | S_IFDIR, 0);
	if (!retval)
		inc_nlink(dir);
	return retval;
}

static const struct inode_operations sfs_dir_inode_operations = {
	.create		= sfs_create,
	.lookup		= simple_lookup,
	.link		= simple_link,
	.unlink		= simple_unlink,
	.symlink	= NULL, /*simple_symlink,*/
	.mkdir		= sfs_mkdir,
	.rmdir		= simple_rmdir,
	.mknod		= sfs_mknod,
	.rename		= simple_rename,
};

/* extern int __set_page_dirty_no_writeback(struct page *); */

static int sfs_readpage(struct file *file, struct page *page)
{
	return simple_readpage(file, page);
}

static int sfs_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata)
{
	return simple_write_begin(file, mapping, pos, len, flags, pagep, fsdata);
}

static int sfs_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata)
{
	return simple_write_end(file, mapping, pos, len, copied, page, fsdata);
}

/*
 * Address Operations Block ...
 */
static const struct address_space_operations sfs_aops = {
	.readpage	= sfs_readpage,
	.write_begin	= sfs_write_begin,
	.write_end	= sfs_write_end,
	/* .set_page_dirty	= __set_page_dirty_no_writeback, */
	.set_page_dirty	= set_page_dirty,
};

/* 
 * Super-block Operations Block ....
 */
static const struct super_operations sfs_sops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= NULL,
};

int sfs_fill_super(struct super_block *sb, void *data, int silent)
{
	sfs_priv_t *priv;
	struct inode *inode;

	priv = kzalloc(sizeof(sfs_priv_t), GFP_KERNEL);
	sb->s_fs_info = priv;
	if (!priv)
		return -ENOMEM;

	sb->s_maxbytes		= MAX_LFS_FILESIZE;
	sb->s_blocksize		= PAGE_SIZE;
	sb->s_blocksize_bits	= PAGE_SHIFT;
	sb->s_magic		= SFS_MAGIC;
	sb->s_op		= &sfs_sops;
	sb->s_time_gran		= 1;

	inode = sfs_get_inode(sb, NULL, S_IFDIR | 0755, 0);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

static struct dentry *sfs_mount(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, sfs_fill_super);
}

static void sfs_kill_sb(struct super_block *sb)
{
	if (sb->s_fs_info) 
		kfree(sb->s_fs_info);

	kill_litter_super(sb);
}

static struct file_system_type sfs_fs_type = 
	{
		.owner		= THIS_MODULE,
		.name		= SFS_NAME,
		.mount		= sfs_mount, 
	       	.kill_sb	= sfs_kill_sb,
		.fs_flags	= FS_USERNS_MOUNT,
	};

static int __init init_sfs(void)
{
	int ret = 0;

	ret = register_filesystem(&sfs_fs_type);
	if (ret) 
		printk(KERN_ERR "register_filesystem - error\n");

	return 0;
}

static void __exit exit_sfs(void)
{
	return ;
}

module_init(init_sfs);
module_exit(exit_sfs);

MODULE_ALIAS("SimpleFS");
MODULE_ALIAS_FS("sfs");
MODULE_AUTHOR("Todd Hwang");
MODULE_DESCRIPTION("Simple File System Sample");
MODULE_LICENSE("GPL");
