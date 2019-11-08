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
 
/*MY sys_call_table address*/
void **system_call_table_addr;
 
asmlinkage int (*original_call) (const char __user*, int, mode_t);
 
asmlinkage int our_sys_open(const char __user* filename, int flags, mode_t mode)
{
	printk(KERN_INFO "\n");

	printk(KERN_INFO "%s opened %s\n", current->comm, filename);
	return original_call(filename, flags, mode);
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
	printk(KERN_INFO "Hookopen loaded successfully..\n");
	// sys_call_table address in System.map
	system_call_table_addr = (void*)kallsyms_lookup_name("sys_call_table");
	original_call = system_call_table_addr[__NR_open];
 
	make_rw((unsigned long)system_call_table_addr);
	system_call_table_addr[__NR_open] = our_sys_open;
	return 0;
}
 
static void __exit exit_point(void)
{
	printk(KERN_INFO "Unloaded hookopen successfully\n");
	// Restore the original call
	system_call_table_addr[__NR_open] = original_call;
	make_ro((unsigned long)system_call_table_addr);
}
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("dacrom");
module_init(entry_point);
module_exit(exit_point);

