#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable_types.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <linux/security.h>
#include <linux/fdtable.h>
#include <linux/string.h>
 
/*MY sys_call_table address*/
void **system_call_table_addr;
 
asmlinkage size_t (*original_call) (unsigned int, const char *, size_t);
 
asmlinkage size_t our_sys_write(unsigned int fd, const char *buf, size_t nbytes)
{
	size_t wrotebytes = original_call(fd, buf, nbytes);

	char fileName[256];
	int fileDesc = fd;
	struct files_struct *files = current->files;
	char *tmp;
	char *pathname;
	struct file *file;
	struct path *path;

	spin_lock(&files->file_lock);
	file = fcheck_files(files, fileDesc);
	if (!file) {
		spin_unlock(&files->file_lock);
		return -ENOENT;
	}
	path = &file->f_path;
	path_get(path);
	spin_unlock(&files->file_lock);
	tmp = (char *)__get_free_page(GFP_KERNEL);
	if (!tmp) {
		path_put(path);
		return -ENOMEM;
	}
	pathname = d_path(path, tmp, PAGE_SIZE);
	path_put(path);
	if (IS_ERR(pathname)) {
		free_page((unsigned long)tmp);
		return PTR_ERR(pathname);
	}
	strcpy(fileName, pathname);
	free_page((unsigned long)tmp);

//	if (strcmp(current->comm, "test_hookwrite") == 0)
//	if (fd == 3)
	{
		printk(KERN_INFO "\n");

		printk(KERN_INFO "%s opened %s wrote %zu byte(s)\n", current->comm, fileName, wrotebytes);
	}
	return wrotebytes;
}
 
/*Make page writeable*/
int make_rw(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~_PAGE_RW){
		pte->pte |=_PAGE_RW;
	}
	return 0;
}
 
/* Make the page write protected */
int make_ro(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~_PAGE_RW;
	return 0;
}
 
static int __init entry_point(void)
{
	printk(KERN_INFO "Hookwrite loaded successfully\n");
	// sys_call_table address in System.map
	system_call_table_addr = (void*)kallsyms_lookup_name("sys_call_table");
	original_call = system_call_table_addr[__NR_write];
 
	make_rw((unsigned long)system_call_table_addr);
	system_call_table_addr[__NR_write] = our_sys_write;
	return 0;
}
 
static void __exit exit_point(void)
{
	printk(KERN_INFO "Unloaded hookwrite successfully\n");
	// Restore the original call
	system_call_table_addr[__NR_write] = original_call;
	make_ro((unsigned long)system_call_table_addr);
}
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("dacrom");
module_init(entry_point);
module_exit(exit_point);

