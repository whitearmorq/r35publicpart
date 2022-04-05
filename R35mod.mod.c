#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
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
	{ 0xb344870e, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xa6190f27, __VMLINUX_SYMBOL_STR(put_device) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xc7a0ccb7, __VMLINUX_SYMBOL_STR(kthread_stop) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0xd01f7062, __VMLINUX_SYMBOL_STR(spi_alloc_device) },
	{ 0xaccd8aa0, __VMLINUX_SYMBOL_STR(spi_busnum_to_master) },
	{ 0xfd5b6f97, __VMLINUX_SYMBOL_STR(open_softirq) },
	{ 0x14b94ab8, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0xd5f4eaf7, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0xda9993c1, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0x2f826659, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0xcde224ab, __VMLINUX_SYMBOL_STR(kallsyms_on_each_symbol) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xc7ea8a7a, __VMLINUX_SYMBOL_STR(raise_softirq) },
	{ 0x3bff2ea, __VMLINUX_SYMBOL_STR(spi_sync) },
	{ 0x64f8b539, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0xcd387ee9, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "075503B8D73C172C2B61281");
