/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * Todd edited - optimized..
 *
 */

#ifndef __TINKER_RK3288_HEADER__
#define __TINKER_RK3288_HEADER__

#include <asm/arch-rockchip/hardware.h>
#include <linux/sizes.h>

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1)

#define CONFIG_SYS_MMC_ENV_DEV 1

/* Environment */
#define ROCKCHIP_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

/*
 * "rootwait" is not needed unlikely aarch64 booting. It will make regulator firmware loading get failed. 
 */
#define BOOTARGS  \
	"console=ttyS2,115200 root=/dev/mmcblk0p3 ro rootfstype=squashfs"

/* FDT File */
#define FDTFILE   "rk3288-tinker.dtb"

/* /dev/mmc0p1 */
#define BOOTPART  "1:2"

#ifndef CFG_CPUID_OFFSET
#define CFG_CPUID_OFFSET	0x7
#endif

#define BOOT_TARGETS	"mmc1 mmc0"

#define CFG_SYS_HZ_CLOCK		24000000
#define CFG_IRAM_BASE			0xff700000
#define CFG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xfe000000

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x0fffffff" "\0" \
	"initrd_high=0x0fffffff" "\0" \
	"scriptaddr=0x00000000\0" \
	"pxefile_addr_r=0x00100000\0" \
	"hw_conf_addr_r=0x00700000\0" \
	"fdt_overlay_addr_r=0x01e00000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"kernel_addr_r=0x02000000\0" \
	"ramdisk_addr_r=0x04000000\0" \
	"bootargs=" BOOTARGS "\0" \
	"loadkernel=mmc dev 1; fatload mmc " BOOTPART " ${kernel_addr_r} zImage; fatload mmc " BOOTPART  " ${fdt_addr_r} " FDTFILE "\0" \
	"jmpkernel=bootz ${kernel_addr_r} - ${fdt_addr_r}" "\0" \
	"bootcmd_mbsp=run loadkernel; run jmpkernel" "\0" \
	ROCKCHIP_DEVICE_SETTINGS "\0" \
	"\0"

#endif
