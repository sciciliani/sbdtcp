#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x80567cab, "module_layout" },
	{ 0x3ec8886f, "param_ops_int" },
	{ 0xc60a49d, "blk_cleanup_queue" },
	{ 0x9d323db1, "put_disk" },
	{ 0x816ca8f3, "del_gendisk" },
	{ 0x96f3110a, "sock_release" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x999e8297, "vfree" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0xb2241487, "add_disk" },
	{ 0xe914e41e, "strcpy" },
	{ 0xff9ff398, "alloc_disk" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0xd89b4cbd, "blk_queue_logical_block_size" },
	{ 0x1d5ec20b, "blk_init_queue" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x1b6314fd, "in_aton" },
	{ 0x743dd655, "sock_create" },
	{ 0xd0d8621b, "strlen" },
	{ 0xbe42cfa3, "__blk_end_request_all" },
	{ 0xb05fd989, "__blk_end_request_cur" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x3b5ae697, "blk_fetch_request" },
	{ 0x73da69a7, "sock_recvmsg" },
	{ 0x558713cf, "sock_sendmsg" },
	{ 0x50eedeb8, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "ED7C4D0EE2FEF0091735E08");
