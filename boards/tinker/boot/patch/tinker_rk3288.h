/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * Todd edited - optimized..
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS
#include <configs/rk3288_common.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1)

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 1

/* Environment */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

#define BOOTARGS  \
		"console=ttyS3,115200 root=/dev/mmcblk0p3 ro rootfstype=squashfs earlyprintk rootwait"

/* FDT File */
#define FDTFILE   "rk3288-tinker.dtb"

/* /dev/mmc0p1 */
#define BOOTPART  "1:2"

#undef CONFIG_EXTRA_ENV_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS \
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
