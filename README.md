## Table of Contents
1. [Why not Yocto](#whynotyocto)
2. [MicroBSP](#microbsp)
3. [Summary](#summary)
4. [Folder Hieararchy](#folder)
5. [Overlay Root File System](#overlayrootfs)
6. [Setting up Prerequisite Utilities/Libraries](#prereq)
7. [Raspberry PI](#rpi3)
	1. [Poor man's gadgets](#rpi3_component)
	2. [How it looks](#rpi3_look)
	3. [Device operation](#rpi3_operation)
	4. [Booting Shot](#rpi3_boot)
	5. [Procedures](#rpi3_procedures)
		1. [Toolchain](#rpi3_toolchain)
		2. [Libraries, Applications, Extra Applications](#rpi3_library)
		3. [Booting Image](#rpi3_bootimage)
		4. [Creating UI image (Embedded QT - Cross-compilation)](#rpi3_ui) 
		5. [Bootable SD Card](#rpi3_sdcard)
		6. [WLAN Configuration](#rpi3_wlan)
		7. [Available Packages](#rpi3_packages)
8. [MBSP VM](#qemu) 
	1. [Testbed components](#qemu_component)
	2. [Setup](#qemu_setup) 
	3. [Booting Shot](#qemu_boot)
	4. [Procedures](#qemu_procedures)
		1. [Toolchain](#qemu_toolchain)
		2. [Libraries, Applications, Extra Applications](#qemu_library)
		3. [Platform specfic binaries ( HTTP server, currently )](#qemu_http)
		4. [Booting Image Creation](#qemu_bootimage)
		5. [Partitioning external USB disk](#qemu_partition)
		6. [Running/Stopping "output.iso"](#qemu_control)
	5. [Host Network](#qemu_hostnetwork)
		1. [Realtek WLAN configuration](#qemu_realtek)
			1. [802.11a AP](#qemu_80211a_ap)
			2. [802.11g AP](#qemu_80211g_ap)
		2. [Renaming network interfaces](#qemu_renaming_ubuntu)
		3. [NAT network configuration](#qemu_nat)
		4. [DHCP/DNS configuration](#qemu_dns)
		5. [Host Network Setup Script](#qemu_script)
9. [Selective Compilation](#selective_compile)
10. [How Python](#python)
	1. [Setting up PIP](#python_pip)
	2. [Upgrading PIP](#python_upgrade_pip)


## Why not using Yocto  <a name="whynotyocto"></a>


* **Yocto** must be the strongest player in embedded Linux BSP(Board Support Package) tracks. In programmer's viewpoint, I think the followings are amazing characteristics of the Yocto.   
  - The greatest attraction of the Yocto is fully automatic build dependency setup. - Just user simply specify dependency with a keyword "DEPENDS", and the Yocto constructs full dependency tree and tries to build from the bottom nodes of the tree. 
  - QEMU basis cross compilation strategy provides a total environmental separation from host machine, which will not be interfered/conflicted with host machine environment.
  - No occurrence of root access authentication during compilation. - None of CLI commands beginning with "sudo" .
  - Packaging is also supported in various forms; rpm, ipk, .. 
 
* Many detail operations of Yocto are totally implemeted/abstracted from "Python" classes, and it is not easy to fully digest how they work. 

* Final usages of BSP sources for embedded system are usually limited to 2 or 3 such as cross compiler generation, building applications, required utilities and the creation of final image to be downloaded into embedded board. 


## MicroBSP  <a name="microbsp"></a>

* I want to set up simple Linux basis embedded board BSP for both **Raspberry PI 3** and **VM image (based on QEMU)M** - Yocto might be a big cat to come with for catching up these mices under coverage of my private preferences; just for build/compile/...

* **Raspberry PI** is the cheapest embedded board I can purchase easily from Amazon in less than $40, and **MBSP VM** can be easily activated in Ubuntu Linux installed PC. -  For poor system programmer like me, Raspberry PI becomes the right answer. **MBSP VM** is simply working with QEMulator.

* I want the BSP to keep folder/source structure to be easily hacked/manipulated. 

* I don't want fancy desktop environment since I have no extra mouse and keyboard to use with the raspberry pi. HDMI monitor for the PI ? Oh.. no its more than $100 on Amazon. Just simple UART connection with the embedded board will be good for me. 

* **MicroBSP** can be used for 
	- Quick verification of GNU applications in embedded environment 
	- Fast evaluation of embedded application 
	- Fast evaluation of Linux kernel features 

* MicroBSP is to assist the needs for my private individual reason and public share purposes based on its inherent flexibility and simplicity.

## Summary <a name="summary"></a>


* MicroBSP vs. Yocto 

|           | Yocto     |  MicroBSP       |
|-----------|-----------| --------------- |
| Host Cross Toolchain | Yes       |  Yes            |
| Native Cross Toolchain | Yes       |  Yes            |
| ARM CPU   | Yes       |  Yes            |
| Intel CPU | Yes       |  Yes            |
| 32bit     | Yes       |  No             |
| Packages  | rpm,ipk,..  |  No             |
| Versioning| Yes       |  No             |
| Final Images   | Yes |  Yes             |
| Core/Board Separation    | Yes |  No    |
| Kernel/Application Separation    | No   |  No    |

* 32 bit CPU board was not tested with MicroBSP. 
* Architecture(CPU core) and board is not separated in MicroBSP. - Even board A and board B are using the same core type "Cortex-A53", but we need to use 2 arch layers for both boards.  

* MicroBSP is 100% Makefile based raspberry pi software build tree. 
* Building transaction should begin with <strong>GNU cross toolchain construction</strong>.
* Everything has been tested and proved in <strong>Ubuntu 22.04</strong>.
* The followings are used to compose cross toolchain from MicroBSP. 

|  S/W       | Version  |
|------------|----------|
|   GLIBC    |  2.36    |
|   GCC      |  11.2.0  |
|  BINUTILS  |  2.38    |
|   BASH     |  5.1.8   |
|  BUSYBOX   |  1.35.0  |

* Version of each native applications/libraries can be found in corresponding Makefile, and the following simply enumerates a few of those. 

|  S/W       | Version  |
|------------|----------|
| OpenSSL    |  1.1.1c  |
|   GDB      |  11.2    |
|   Perl     |  5.24.1  |
|  Python    |  3.10.8  |

* Linux kernels used for both raspberry pi and MBSP VM are as follows. 

|  Type            | Linux kernel version |
|------------------|----------------------|
| Raspberry PI     |  5.15.x              |
| MBSP VM          |  5.17.7              |

* MicroBSP has **Overlay File System** basis booting policy. 
* Total booting disk image has the following hierarchy. 


## Folder Hierarchy <a name="folder"></a>


The directory hieararchy of MicroBSP is...


```
  # ls -la
  total 260
  drwxrwxr-x 13 todd todd  4096 Nov  6 07:12 .  
  drwxrwxr-x  4 todd todd  4096 Oct 26 13:19 ..
  drwxrwxr-x  7 todd todd  4096 Oct 26 09:18 apps
  drwxrwxr-x  5 todd todd  4096 Oct 23 14:09 arch
  drwxrwxr-x  5 todd todd  4096 Oct 23 14:09 boards
  drwxrwxr-x  2 todd todd  4096 Nov  6 06:46 doc
  drwxrwxr-x 65 todd todd  4096 Nov  2 20:01 exts
  drwxrwxr-x  8 todd todd  4096 Nov  5 21:40 .git
  drwxrwxr-x  8 todd todd  4096 Oct 23 14:11 gnu
  drwxrwxr-x 37 todd todd  4096 Oct 26 09:35 libs
  -rw-rw-r--  1 todd todd 29714 Nov  3 06:51 Makefile
  -rw-rw-r--  1 todd todd 87500 Nov  6 07:12 README.md
  drwxrwxr-x  2 todd todd  4096 Nov  2 22:13 scripts
  drwxrwxr-x 43 todd todd  4096 Oct 25 20:27 uix
  #
```


|  Folders       | Description   |
|----------------|----------|
| **apps/**          | Basic application required for bootstrap disks |
| **libs/**          | Libraries  |
| **exts/**          | Libraries/Applications for high level functions  |
| **uix/**           | Graphic/Multimedia related Libraries/Applications   |


## Overlay Root File Sysyem  <a name="overlayrootfs"></a>


The following shows the structure of root file system constructed by overlay file system mount process. Overlay file system components; lower, upper, work directory is organized with folders in both **_base** and **_stagedir** created after the step **Boot Image Building** .  

|  Dir Type        |  Folders                | 
|------------------|-------------------------|
| Lower dir        |  _base , _stagedir/usr  |
| Upper dir        |  _stagedir/up           |
| Work  dir        |  _stagedir/work         |

- Upper dir and Work dir should be placed in the same disk partition so they are located in the same folder **_stagedir**. 

- **_base** is populated from both libs/ and apps/ in MicroBSP. 
- **_stagedir** is populated from exts/ folder in MicroBSP. 
- Applications in bootstrap image **rootfs.cramfs** is populated from **apps/busybox** . 

![](doc/overlay_hierarchy.png)

- About detail description of **overlay file system** , please refer to [How/What Overlay File System Does](https://www.datalight.com/blog/2016/01/27/explaining-overlayfs-%E2%80%93-what-it-does-and-how-it-works/).

- File system mount is achieved in two steps;
	* First, bootstrap file system is mounted at booting time
	* Second, moves to **/ovr** folder by using **"choot /ovr /etc/rc.init"** for next booting.


## Setting up Prerequisite Utilities/Libraries  <a name="prereq"></a>


- Fundamental libraries/applications will be installed by the following command.
- <strong>This can work in Ubuntu. </strong>

```
  # make installcomps
```

## Raspberry PI  <a name="rpi3"></a>


### Poor man's gadgets <a name="rpi3_component"></a>

* [Raspberry PI 3+](https://www.amazon.com/CanaKit-Raspberry-Complete-Starter-Kit/dp/B01C6Q2GSY/ref=sr_1_6?keywords=raspberry+pi+3%2B&qid=1636520388&qsid=132-7915930-0137541&sr=8-6&sres=B07BCC8PK7%2CB01C6EQNNK%2CB07KKBCXLY%2CB01C6Q2GSY%2CB01LPLPBS8%2CB07BDR5PDW%2CB07BC567TW%2CB07BFH96M3%2CB07N38B86S%2CB0778CZ97B%2CB09HGXHRLQ%2CB08C254F73%2CB08G8QYFCD%2CB01N13X8V1%2CB09HH2BFN4%2CB06W54L7B5)

* [UART Cable for Raspberry PI](https://www.amazon.com/FEANTEEK-TTL232R-Raspberry-Serial-Windows/dp/B08HLSS5T4/ref=sr_1_1_sspa?keywords=raspberry+pi+3%2B+UART+cable&qid=1636520578&sr=8-1-spons&psc=1&spLa=ZW5jcnlwdGVkUXVhbGlmaWVyPUExTEFJTFcwNlNKT0lIJmVuY3J5cHRlZElkPUEwMjc5MjEwTjlUUDBEWEtNTzJTJmVuY3J5cHRlZEFkSWQ9QTA3NDE2NTBMV0xWUDdaRlZFV0gmd2lkZ2V0TmFtZT1zcF9hdGYmYWN0aW9uPWNsaWNrUmVkaXJlY3QmZG9Ob3RMb2dDbGljaz10cnVl )

* [TP-Link USB WiFi Dongle](https://www.amazon.com/TP-Link-TL-WN823N-Wireless-network-Raspberry/dp/B0088TKTY2/ref=sr_1_15?keywords=tp+link+usb+wifi+adapter&qid=1636520657&qsid=132-7915930-0137541&sr=8-15&sres=B08D72GSMS%2CB008IFXQFU%2CB07P6N2TZH%2CB08KHV7H1S%2CB07PB1X4CN%2CB07P5PRK7J%2CB00JBJ6VG8%2CB00YUU3KC6%2CB002SZEOLG%2CB00K11UIV4%2CB0088TKTY2%2CB01MR6M8EC%2CB00HC01KMS%2CB0799C35LV%2CB01NBMJGA9%2CB00A8GVNNY&srpt=NETWORK_INTERFACE_CONTROLLER_ADAPTER)

### How it looks <a name="rpi3_look"></a>

![](doc/rpi.png)


### Device operation <a name="rpi3_operation"></a>

* Home Gateway
* LAN
  - Ethernet : Built-in ethernet
  - WLAN1 : Raspberry PI Built-in WLAN (<strong>Broadcom BRCM43455</strong>)
* WAN
  - WLAN0 : TP-Link USB WiFi Dongle (<strong>Realtek</strong>)
* UART baudrate = 921600bps (not 115200bps)

### Booting Shot <a name="rpi3_boot"></a>

  ```

U-Boot 2022.10-rc3 (Apr 11 2023 - 21:14:47 -0700)

DRAM:  948 MiB
RPI 3 Model B+ (0xa020d3)
Core:  63 devices, 13 uclasses, devicetree: embed
MMC:   mmc@7e202000: 0, mmc@7e300000: 1
Loading Environment from FAT... OK
In:    serial
Out:   serial
Err:   serial
Net:   No ethernet found.
Hit any key to stop autoboot:  0 
switch to partitions #0, OK
mmc0 is current device
26828808 bytes read in 1114 ms (23 MiB/s)
32529 bytes read in 4 ms (7.8 MiB/s)
Moving Image from 0x80000 to 0x200000, end=1c90000
## Flattened Device Tree blob at 02600000
   Booting using the fdt blob at 0x2600000
   Using Device Tree in place at 0000000002600000, end 000000000260af10

Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd034]
[    0.000000] Linux version 5.15.92-v8 (todd@vostro) (aarch64-any-linux-gnu-gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.38) #6 SMP PREEMPT Mon Apr 10 12:59:19 PDT 2023
[    0.000000] Machine model: Raspberry Pi 3 Model B+
[    0.000000] Reserved memory: created CMA memory pool at 0x0000000037400000, size 64 MiB
[    0.000000] OF: reserved mem: initialized node linux,cma, compatible id shared-dma-pool
[    0.000000] Zone ranges:
[    0.000000]   DMA      [mem 0x0000000000000000-0x000000003b3fffff]
[    0.000000]   DMA32    empty
[    0.000000]   Normal   empty
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000000000000-0x000000003b3fffff]
[    0.000000] Initmem setup node 0 [mem 0x0000000000000000-0x000000003b3fffff]
[    0.000000] On node 0, zone DMA: 19456 pages in unavailable ranges
[    0.000000] percpu: Embedded 27 pages/cpu s72600 r8192 d29800 u110592
[    0.000000] Detected VIPT I-cache on CPU0
[    0.000000] CPU features: detected: ARM erratum 843419
[    0.000000] CPU features: detected: ARM erratum 845719
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 238896
[    0.000000] Kernel command line: coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.enable_compat_alsa=0 snd_bcm2835.enable_hdmi=1 video=HDMI-A-1:640x480M@60D vc_mem.mem_base=0x3ec00000 vc_
mem.mem_size=0x40000000  dwg_otg.lpm_enable=0 8250.nr_uarts=1 console=ttyS0,921600 root=/dev/mmcblk0p2 rootfstype=squashfs rootwait
[    0.000000] Dentry cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
[    0.000000] Inode-cache hash table entries: 65536 (order: 7, 524288 bytes, linear)
[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
[    0.000000] Memory: 857568K/970752K available (13056K kernel code, 2072K rwdata, 4104K rodata, 6848K init, 939K bss, 47648K reserved, 65536K cma-reserved)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
[    0.000000] ftrace: allocating 41322 entries in 162 pages
[    0.000000] ftrace: allocated 162 pages with 3 groups
[    0.000000] trace event string verifier disabled
[    0.000000] rcu: Preemptible hierarchical RCU implementation.
[    0.000000] rcu:     RCU event tracing is enabled.
[    0.000000]  Trampoline variant of Tasks RCU enabled.
[    0.000000]  Rude variant of Tasks RCU enabled.
[    0.000000]  Tracing variant of Tasks RCU enabled.
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 100 jiffies.
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
[    0.000000] Root IRQ handler: bcm2836_arm_irqchip_handle_irq
[    0.000000] arch_timer: cp15 timer(s) running at 19.20MHz (phys).
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0x46d987e47, max_idle_ns: 440795202767 ns
[    0.000001] sched_clock: 56 bits at 19MHz, resolution 52ns, wraps every 4398046511078ns
[    0.000357] Console: colour dummy device 80x25
[    0.000438] Calibrating delay loop (skipped), value calculated using timer frequency.. 38.40 BogoMIPS (lpj=19200)
[    0.000476] pid_max: default: 32768 minimum: 301
[    0.000636] LSM: Security Framework initializing
[    0.000929] Mount-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
[    0.000975] Mountpoint-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
[    0.002516] cgroup: Disabling memory control group subsystem
[    0.006290] rcu: Hierarchical SRCU implementation.
[    0.008230] smp: Bringing up secondary CPUs ...
[    0.009700] Detected VIPT I-cache on CPU1
[    0.009781] CPU1: Booted secondary processor 0x0000000001 [0x410fd034]
[    0.011362] Detected VIPT I-cache on CPU2
[    0.011417] CPU2: Booted secondary processor 0x0000000002 [0x410fd034]
[    0.012956] Detected VIPT I-cache on CPU3
[    0.013006] CPU3: Booted secondary processor 0x0000000003 [0x410fd034]
[    0.013202] smp: Brought up 1 node, 4 CPUs
[    0.013263] SMP: Total of 4 processors activated.
[    0.013284] CPU features: detected: 32-bit EL0 Support
[    0.013303] CPU features: detected: CRC32 instructions
[    0.013857] CPU: All CPU(s) started at EL2
[    0.013931] alternatives: patching kernel code
[    0.015426] devtmpfs: initialized
[    0.034702] Enabled cp15_barrier support
[    0.034758] Enabled setend support
[    0.034789] KASLR disabled due to lack of seed
[    0.035044] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 1911260446275000 ns
[    0.035088] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
[    0.038244] pinctrl core: initialized pinctrl subsystem
[    0.040325] NET: Registered PF_NETLINK/PF_ROUTE protocol family
[    0.050312] DMA: preallocated 1024 KiB GFP_KERNEL pool for atomic allocations
[    0.050893] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA pool for atomic allocations
[    0.052246] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA32 pool for atomic allocations
[    0.052480] audit: initializing netlink subsys (disabled)
[    0.052933] audit: type=2000 audit(0.052:1): state=initialized audit_enabled=0 res=1
[    0.053707] thermal_sys: Registered thermal governor 'step_wise'
[    0.053983] cpuidle: using governor menu
[    0.054307] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers.
[    0.054561] ASID allocator initialised with 65536 entries
[    0.054770] Serial: AMBA PL011 UART driver
[    0.064335] bcm2835-mbox 3f00b880.mailbox: mailbox enabled
[    0.079953] raspberrypi-firmware soc:firmware: Attached to firmware from 2023-03-17T10:52:42, variant start
[    0.080965] raspberrypi-firmware soc:firmware: Firmware hash is 82f3750a65fadae9a38077e3c2e217ad158c8d54
[    0.138723] bcm2835-dma 3f007000.dma: DMA legacy API manager, dmachans=0x1
[    0.143700] SCSI subsystem initialized
[    0.144010] usbcore: registered new interface driver usbfs
[    0.144091] usbcore: registered new interface driver hub
[    0.144197] usbcore: registered new device driver usb
[    0.144594] usb_phy_generic phy: supply vcc not found, using dummy regulator
[    0.144898] usb_phy_generic phy: dummy supplies not allowed for exclusive requests
[    0.145238] pps_core: LinuxPPS API ver. 1 registered
[    0.145259] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    0.145296] PTP clock support registered
[    0.147549] clocksource: Switched to clocksource arch_sys_counter
[    0.269262] VFS: Disk quotas dquot_6.6.0
[    0.269388] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    0.269639] FS-Cache: Loaded
[    0.269947] CacheFiles: Loaded
[    0.283446] NET: Registered PF_INET protocol family
[    0.283851] IP idents hash table entries: 16384 (order: 5, 131072 bytes, linear)
[    0.285369] tcp_listen_portaddr_hash hash table entries: 512 (order: 1, 8192 bytes, linear)
[    0.285427] Table-perturb hash table entries: 65536 (order: 6, 262144 bytes, linear)
[    0.285461] TCP established hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.285728] TCP bind hash table entries: 8192 (order: 5, 131072 bytes, linear)
[    0.285939] TCP: Hash tables configured (established 8192 bind 8192)
[    0.286210] UDP hash table entries: 512 (order: 2, 16384 bytes, linear)
[    0.286277] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes, linear)
[    0.286664] NET: Registered PF_UNIX/PF_LOCAL protocol family
[    0.287838] RPC: Registered named UNIX socket transport module.
[    0.287863] RPC: Registered udp transport module.
[    0.287882] RPC: Registered tcp transport module.
[    0.287900] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    0.291483] hw perfevents: enabled with armv8_cortex_a53 PMU driver, 7 counters available
[    1.627895] Initialise system trusted keyrings
[    1.628341] workingset: timestamp_bits=46 max_order=18 bucket_order=0
[    1.638279] zbud: loaded
[    1.640645] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    1.641210] FS-Cache: Netfs 'nfs' registered for caching
[    1.642121] NFS: Registering the id_resolver key type
[    1.642182] Key type id_resolver registered
[    1.642202] Key type id_legacy registered
[    1.642351] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
[    1.642376] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering...
[    1.642446] ntfs: driver 2.1.32 [Flags: R/W].
[    1.642774] ntfs3: Max link count 4000
[    1.643188] fuse: init (API version 7.34)
[    1.718211] Key type asymmetric registered
[    1.718237] Asymmetric key parser 'x509' registered
[    1.718348] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 247)
[    1.718373] io scheduler mq-deadline registered
[    1.718395] io scheduler kyber registered
[    1.719645] crc32: CRC_LE_BITS = 64, CRC_BE BITS = 64
[    1.719668] crc32: self tests passed, processed 225944 bytes in 552656 nsec
[    1.719820] crc32c: CRC_LE_BITS = 64
[    1.719838] crc32c: self tests passed, processed 225944 bytes in 64531 nsec
[    1.756342] crc32_combine: 8373 self tests passed
[    1.792920] crc32c_combine: 8373 self tests passed
[    1.798944] bcm2708_fb soc:fb: FB found 1 display(s)
[    1.812757] Console: switching to colour frame buffer device 80x30
[    1.817998] bcm2708_fb soc:fb: Registered framebuffer for display 0, size 640x480
[    1.905692] Serial: 8250/16550 driver, 1 ports, IRQ sharing enabled
[    1.908391] bcm2835-rng 3f104000.rng: hwrng registered
[    1.909117] vc-mem: phys_addr:0x00000000 mem_base=0x3ec00000 mem_size:0x40000000(1024 MiB)
[    1.910717] gpiomem-bcm2835 3f200000.gpiomem: Initialised: Registers at 0x3f200000
[    1.927875] brd: module loaded
[    1.941096] loop: module loaded
[    1.983870] Loading iSCSI transport class v2.0-870.
[    1.987079] usbcore: registered new interface driver lan78xx
[    1.987171] usbcore: registered new interface driver smsc95xx
[    1.987211] dwc_otg: version 3.00a 10-AUG-2012 (platform bus)
[    2.188058] Core Release: 2.80a
[    2.188087] Setting default values for core params
[    2.188124] Finished setting default values for core params
[    2.388559] Using Buffer DMA mode
[    2.388580] Periodic Transfer Interrupt Enhancement - disabled
[    2.388600] Multiprocessor Interrupt Enhancement - disabled
[    2.388618] OTG VER PARAM: 0, OTG VER FLAG: 0
[    2.388643] Dedicated Tx FIFOs mode
[    2.389437] 
[    2.389451] WARN::dwc_otg_hcd_init:1072: FIQ DMA bounce buffers: virt = ffffffc009ef9000 dma = 0x00000000f7810000 len=9024
[    2.389504] FIQ FSM acceleration enabled for :
[    2.389504] Non-periodic Split Transactions
[    2.389504] Periodic Split Transactions
[    2.389504] High-Speed Isochronous Endpoints
[    2.389504] Interrupt/Control Split Transaction hack enabled
[    2.389615] 
[    2.389627] WARN::hcd_init_fiq:496: MPHI regs_base at ffffffc009acd000
[    2.389684] dwc_otg 3f980000.usb: DWC OTG Controller
[    2.389740] dwc_otg 3f980000.usb: new USB bus registered, assigned bus number 1
[    2.389802] dwc_otg 3f980000.usb: irq 74, io mem 0x00000000
[    2.389865] Init: Port Power? op_state=1
[    2.389883] Init: Power Port (0)
[    2.390362] usb usb1: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 5.15
[    2.390393] usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1
[    2.390419] usb usb1: Product: DWC OTG Controller
[    2.390441] usb usb1: Manufacturer: Linux 5.15.92-v8 dwc_otg_hcd
[    2.390464] usb usb1: SerialNumber: 3f980000.usb
[    2.391406] hub 1-0:1.0: USB hub found
[    2.391504] hub 1-0:1.0: 1 port detected
[    2.393640] usbcore: registered new interface driver usb-storage
[    2.393673] i2c_dev: i2c /dev entries driver
[    2.398261] sdhci: Secure Digital Host Controller Interface driver
[    2.398286] sdhci: Copyright(c) Pierre Ossman
[    2.398867] sdhci-pltfm: SDHCI platform and OF driver helper
[    2.401818] ledtrig-cpu: registered to indicate activity on CPUs
[    2.402355] hid: raw HID events driver (C) Jiri Kosina
[    2.402591] usbcore: registered new interface driver usbhid
[    2.402615] usbhid: USB HID core driver
[    2.403034] ashmem: initialized
[    2.411131] xt_time: kernel timezone is -0000
[    2.411342] ipip: IPv4 and MPLS over IPv4 tunneling driver
[    2.412093] gre: GRE over IPv4 demultiplexor driver
[    2.412234] ipt_CLUSTERIP: ClusterIP Version 0.8 loaded successfully
[    2.412299] Initializing XFRM netlink socket
[    2.413112] NET: Registered PF_INET6 protocol family
[    2.415341] Segment Routing with IPv6
[    2.415428] In-situ OAM (IOAM) with IPv6
[    2.416736] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
[    2.418240] ip6_gre: GRE over IPv6 tunneling driver
[    2.434291] bpfilter: Loaded bpfilter_umh pid 126
[    2.435384] NET: Registered PF_PACKET protocol family
[    2.435490] NET: Registered PF_KEY protocol family
[    2.435660] Bridge firewalling registered
[    2.435800] 8021q: 802.1Q VLAN Support v1.8
[    2.435921] Key type dns_resolver registered
[    2.435971] Key type ceph registered
[    2.436685] libceph: loaded (mon/osd proto 15/24)
[    2.436710] mctp: management component transport protocol core
[    2.436729] NET: Registered PF_MCTP protocol family
[    2.438421] registered taskstats version 1
[    2.438480] Loading compiled-in X.509 certificates
[    2.439242] Key type .fscrypt registered
[    2.439265] Key type fscrypt-provisioning registered
[    2.455853] uart-pl011 3f201000.serial: cts_event_workaround enabled
[    2.456116] 3f201000.serial: ttyAMA0 at MMIO 0x3f201000 (irq = 99, base_baud = 0) is a PL011 rev2
[    2.460361] bcm2835-aux-uart 3f215040.serial: there is not valid maps for state default
[    2.462810] printk: console [ttyS0] disabled
[    2.462967] 3f215040.serial: ttyS0 at MMIO 0x3f215040 (irq = 71, base_baud = 31250000) is a 16550
[    2.497756] Indeed it is in host mode hprt0 = 00021501
[    2.498335] printk: console [ttyS0] enabled
[    2.629648] bcm2835-wdt bcm2835-wdt: Broadcom BCM2835 watchdog timer
[    2.631164] bcm2835-power bcm2835-power: Broadcom BCM2835 power domains driver
[    2.634446] mmc-bcm2835 3f300000.mmcnr: mmc_debug:0 mmc_debug2:0
[    2.635281] mmc-bcm2835 3f300000.mmcnr: DMA channel allocated
[    2.662852] sdhost: log_buf @ (____ptrval____) (c4352000)
[    2.672595] usb 1-1: new high-speed USB device number 2 using dwc_otg
[    2.673764] Indeed it is in host mode hprt0 = 00001101
[    2.734746] mmc0: sdhost-bcm2835 loaded - DMA enabled (>1)
[    2.740031] of_cfs_init
[    2.740609] of_cfs_init: OK
[    2.741397] cfg80211: Loading compiled-in X.509 certificates for regulatory database
[    2.746752] cfg80211: Loaded X.509 cert 'sforshee: 00b28ddf47aef9cea7'
[    2.747951] platform regulatory.0: Direct firmware load for regulatory.db failed with error -2
[    2.749188] cfg80211: failed to load regulatory.db
[    2.753174] Waiting for root device /dev/mmcblk0p2...
[    2.775050] mmc0: host does not support reading read-only switch, assuming write-enable
[    2.779714] mmc0: new high speed SDXC card at address 0001
[    2.782114] mmcblk0: mmc0:0001 GC2QT 59.6 GiB 
[    2.788567] mmc1: new high speed SDIO card at address 0001
[    2.789256]  mmcblk0: p1 p2 p3 p4 < p5 p6 >
[    2.791753] mmcblk0: mmc0:0001 GC2QT 59.6 GiB
[    2.802834] VFS: Mounted root (squashfs filesystem) readonly on device 179:2.
[    2.807448] devtmpfs: mounted
[    2.825429] Freeing unused kernel memory: 6848K
[    2.826258] Run /sbin/init as init process
[    2.861160] usb 1-1: New USB device found, idVendor=0424, idProduct=2514, bcdDevice= b.b3
[    2.862328] usb 1-1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    2.864404] hub 1-1:1.0: USB hub found
[    2.865123] hub 1-1:1.0: 4 ports detected
[    3.149606] usb 1-1.1: new high-speed USB device number 3 using dwc_otg
[    3.239137] usb 1-1.1: New USB device found, idVendor=0424, idProduct=2514, bcdDevice= b.b3
[    3.240313] usb 1-1.1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    3.242341] hub 1-1.1:1.0: USB hub found
[    3.243078] hub 1-1.1:1.0: 3 ports detected
[    3.321706] usb 1-1.2: new high-speed USB device number 4 using dwc_otg
[    3.407024] EXT4-fs (mmcblk0p3): recovery complete
[    3.409361] EXT4-fs (mmcblk0p3): mounted filesystem with ordered data mode. Opts: (null). Quota mode: none.
[    3.413151] usb 1-1.2: New USB device found, idVendor=2357, idProduct=0109, bcdDevice= 2.00
[    3.414345] usb 1-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[    3.415359] usb 1-1.2: Product: 802.11n NIC 
[    3.415975] usb 1-1.2: Manufacturer: Realtek 
[    3.416593] usb 1-1.2: SerialNumber: 00e04c000001
[    3.454984] EXT4-fs (mmcblk0p5): recovery complete
[    3.457313] EXT4-fs (mmcblk0p5): mounted filesystem with ordered data mode. Opts: (null). Quota mode: none.
Mounting...
[    3.476349] overlayfs: "xino" feature enabled using 32 upper inode bits.
Device file system
Change root !!
[    4.046600] usb 1-1.1.1: new high-speed USB device number 5 using dwc_otg
mount: mounting devfs on /dev failed: Device or resource busy
[    4.136417] usb 1-1.1.1: New USB device found, idVendor=0424, idProduct=7800, bcdDevice= 3.00
[    4.137630] usb 1-1.1.1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
chpasswd: password for 'root' changed
[    4.272683] random: crng init done

XCFGD Configurator - wait 5

[    4.330871] EXT4-fs (mmcblk0p6): recovery complete
[    4.332913] EXT4-fs (mmcblk0p6): mounted filesystem with ordered data mode. Opts: (null). Quota mode: none.
Waiting it for getting ready to work..
Daemon Running...
[    4.406886] lan78xx 1-1.1.1:1.0 (unnamed net_device) (uninitialized): No External EEPROM. Setting MAC Speed
[    4.457862] lan78xx 1-1.1.1:1.0 (unnamed net_device) (uninitialized): int urb period 64
DBG:main                :649  TEMPORARY FILE - SYNC : /var/tmp/xcfgXeb6ytm
DBG:main                :673  XML1 : /etc/config.xml
DBG:main                :669  DEV : /config/db
DBG:main                :665  STANDADLONE MODE
DBG:main                :752  TEMPORARY DIRECTORY = (/var/tmp/184)
MSG:xml_storage_open    :1222 /config/db Physical Information = ( 4 x 128 Kbytes + 2 x 1024 Kbytes ) 4 
DBG:xml_storage_open    :1253 /config/db has been opened
MSG:xml_storage_open    :1262 FILE /config/db SIZE=2621440 
DBG:xml_storage_open    :1306 HEADER[ 0 ] FLAGS=0 DIRTY=1 
DBG:xml_storage_open    :1306 HEADER[ 1 ] FLAGS=0 DIRTY=0 
DBG:xml_storage_open    :1306 HEADER[ 2 ] FLAGS=0 DIRTY=0 
DBG:xml_storage_open    :1306 HEADER[ 3 ] FLAGS=0 DIRTY=0 
MSG:xml_storage_open    :1312 xml_storage_open : HEADER ANALSYS [4/4] 
MSG:xml_storage_open    :1320 xml_storage_open : HEADER LOCATION = ( 00000000 00020000 00040000 00060000 )
MSG:xml_storage_open    :1351 DATA[ 0 ] has 8 blocks 
MSG:xml_storage_open    :1351 DATA[ 1 ] has 8 blocks 
MSG:xml_storage_open    :1364 xml_storage_open : DATA LOCATION = ( 00080000 00180000 )
DBG:xml_storage_open    :1386 /config/db configured and prepared before 
DBG:_xml_storage_header_print:862  [[DUMP]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=8fe2 size=936 
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=8e23 size=92c 
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=7833 size=92f 
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=f19a size=933 
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[BROKEN FIXUP]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=8fe2 size=936 
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=8e23 size=92c 
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=7833 size=92f 
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=f19a size=933 
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[DIRTY+ONDIRTY CHECK]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=8fe2 size=936 
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=8e23 size=92c 
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=7833 size=92f 
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=f19a size=933 
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[RECOVER BLOCK PROCESS]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=8fe2 size=936 
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=8e23 size=92c 
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=7833 size=92f 
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=f19a size=933 
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[DIRTY BLOCK PROCESS]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=8fe2 size=936 
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=8e23 size=92c 
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=7833 size=92f 
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=f19a size=933 
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[FINALLY WRITTEN]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=8fe2 size=936 
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=8e23 size=92c 
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=7833 size=92f 
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=f19a size=933 
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_fetch:767  DIRTY HEADER[ 0  ] START = 0x20000 / DATA OFFSET = 524288 FLAG=0000 
MSG:xml_storage_validate:1440 HEADER[0 ]::MAGIC=OK 
MSG:xml_storage_validate:1441 HEADER[0 ]::RSIZE=2358 
MSG:xml_storage_validate:1442 HEADER[0 ]::START=80000 
MSG:xml_storage_validate:1443 HEADER[0 ]::CRC  =8fe2 
MSG:xml_storage_validate:1493 Temporary XML file name : (/var/tmp/184/config.xml) 
DBG:xml_storage_validate:1507 /etc/config.xml write 2358/2358 
MSG:xml_storage_validate:1519 STORAGE VALIDATED [0] 
DBG:config_xml_merge    :401  VERSION (1.0.0.1 vs 1.0.0.1) 
DBG:config_xml_merge    :404  CURRENTLY LATEST VERSION
3
2
1


DATE 

Wed Mar  1 12:00:00 UTC 2023

Logging daemon


USER 

chpasswd: password for 'todd' changed
User [ todd/12345678 ] created

GIT 


PYTHON  


QT Runtime environment


SSH SERVER 


SSH key setup in /root/.ssh 


SSH daemon


RASPBERRY PI SPECIFIC INIT


LPPS 


INIT FILE STARTS ...

640x480x32bpp pitch 2560 type 0 visual 2 colors 16777216 pixtype 8
[WLAN] Loading all WLAN modules...
[   10.672143] 8192eu: loading out-of-tree module taints kernel.
[   10.906227] usbcore: registered new interface driver rtl8192eu
[   11.388502] RTW: module init start
[   11.389024] RTW: rtl88x2bu v5.13.1-20-gbd7c7eb9d.20210702_COEX20210316-18317b7b
[   11.390043] RTW: rtl88x2bu BT-Coex version = COEX20210316-18317b7b
[   11.391098] usbcore: registered new interface driver rtl88x2bu
[   11.391917] RTW: module init ret=0
[WLAN] Building up /var/tmp/wpa_supplicant/1.conf
ifconfig: SIOCSIFHWADDR: Operation not permitted
[WLAN] Running WPA Suppplicant !!
Successfully initialized wpa_supplicant
ioctl[SIOCSIWAP]: Operation not permitted
Waiting Connect .. 0
Waiting Connect .. 1
Waiting Connect .. 2
Waiting Connect .. 3
Waiting Connect .. 4
Waiting Connect .. 5
Waiting Connect .. 6
Waiting Connect .. 7
IPv4 DHCP ...
Waiting IP .. 0
Waiting IP .. 1
Waiting IP .. 2
[   24.641110] IPv6: ADDRCONF(NETDEV_CHANGE): wlan0: link becomes ready
Waiting IP .. 3
Waiting IP .. 4
[WLAN] BRCM Loading modules...
[   28.029417] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   28.031416] usbcore: registered new interface driver brcmfmac
[   28.037468] brcmfmac mmc1:0001:1: Direct firmware load for brcm/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.bin failed with error -2
[   28.254108] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   28.255579] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   28.262019] brcmfmac: brcmf_c_preinit_dcmds: Firmware: BCM4345/6 wl0: Nov  1 2021 00:37:25 version 7.45.241 (1a2f2fa CY) FWID 01-703fd60
[WLAN] Building up /var/tmp/hostapd/1.conf
[ETH] Ethernet and Bridge
[   29.622757] lan78xx 1-1.1.1:1.0 eth0: kevent 4 may have been dropped
[   29.627293] 8021q: adding VLAN 0 to HW filter on device eth0
[   29.636030] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[   29.662683] br0: port 1(eth0) entered blocking state
[   29.663456] br0: port 1(eth0) entered disabled state
[   29.664788] device eth0 entered promiscuous mode
[AP] Adding APs...
[WLAN] Running AP Suppplicant for wlan2
random: Trying to read entropy from /dev/random
Configuration file: /var/tmp/hostapd/0.conf
nl80211: Using driver-based roaming
nl80211: TDLS supported
nl80211: Supported cipher 00-0f-ac:1
nl80211: Supported cipher 00-0f-ac:5
nl80211: Supported cipher 00-0f-ac:2
nl80211: Supported cipher 00-0f-ac:4
nl80211: Supported cipher 00-0f-ac:6
nl80211: Using driver-based off-channel TX
nl80211: Supported vendor command: vendor_id=0x1018 subcmd=1
nl80211: Use separate P2P group interface (driver advertised support)
nl80211: Enable multi-channel concurrent (driver advertised support)
nl80211: use P2P_DEVICE support
nl80211: interface wlan2 in phy phy1
nl80211: Set mode ifindex 10 iftype 3 (AP)
nl80211: Setup AP(wlan2) - device_ap_sme=1 use_monitor=0
nl80211: Subscribe to mgmt frames with AP handle 0x1b463d70 (device SME)
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=04
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=0501
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=0503
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=0504
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=06
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=08
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=09
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=0a
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=11
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=12
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1b463d70 match=7f
nl80211: Register frame type=0xb0 (WLAN_FC_STYPE_AUTH) nl_handle=0x1b463d70 match=
nl80211: Enable Probe Request reporting nl_preq=0x1b463960
nl80211: Register frame type=0x40 (WLAN_FC_STYPE_PROBE_REQ) nl_handle=0x1b463960 match=
rfkill: initial event: idx=1 type=1 op=0 soft=0 hard=0
nl80211: Add own interface ifindex 11 (ifidx_reason 10)
nl80211[   29.993298] br0: port 2(wlan2) entered blocking state
: if_indices[16]: 11(10)[   29.994223] br0: port 2(wlan2) entered disabled state

nl80211: Add own interface ifindex 10 (ifidx_reason -1)
nl80211: if_indices[16]: 11(10) 10(-1[   29.995990] device wlan2 entered promiscuous mode
)
nl80211: Adding interface wlan2 into bridge br0
phy: phy1
BSS count 1, BSSID mask 00:00:00:00:00:00 (0 bits)
wlan2: interface state UNINITIALIZED->COUNTRY_UPDATE
Previous country code 00, new country code US 
Continue interface setup after channel list update
ctrl_iface not configured!
[BRIDGE] Bridge adding wlan2
brctl: bridge br0: Device or resource busy
[WLAN] Running AP Suppplicant for wlan1
random: Trying to read entropy from /dev/random
Configuration file: /var/tmp/hostapd/1.conf
nl80211: Supported cipher 00-0f-ac:1
nl80211: Supported cipher 00-0f-ac:5
nl80211: Supported cipher 00-0f-ac:2
nl80211: Supported cipher 00-0f-ac:4
nl80211: Supported cipher 00-0f-ac:6
nl80211: Supported vendor command: vendor_id=0x1a11 subcmd=4106
nl80211: Supported vendor command: vendor_id=0x1a11 subcmd=4107
nl80211: Use separate P2P group interface (driver advertised support)
nl80211: interface wlan1 in phy phy0
nl80211: Set mode ifindex 9 iftype 3 (AP)
nl80211: Setup AP(wlan1) - device_ap_sme=1 use_monitor=0
nl80211: Subscribe to mgmt frames with AP handle 0x3f2ddd70 (device SME)
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=04
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=0501
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=0503
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=0504
...
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=11
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=12
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3f2ddd70 match=7f
nl80211: Register frame type=0xb0 (WLAN_FC_STYPE_AUTH) nl_handle=0x3f2ddd70 match=
nl80211: Enable Probe Request reporting nl_preq=0x3f2dd960
nl80211: Register frame type=0x40 (WLAN_FC_STYPE_PROBE_REQ) nl_handle=0x3f2dd960 match=
rfkill: initial event: idx=0 type=1 op=0 soft=0 hard=0
nl80211: Add own interface ifindex 11 (ifidx_reason 9)
nl80211: if_indices[16]: 11(9)
nl80211: Add own interface ifindex 9 (ifidx_reason -1)
nl80211: if_indices[16]: 11(9) 9(-1)
nl80211: Adding interface wlan1 into bridge br0
phy: phy0
BSS count 1, BSSID mask 00:00:00:00:00:00 (0 bits)
Using existing control interface directory.
wlan1: interface state UNINITIALIZED->COUNTRY_UPDATE
Previous country code 00, new country code US 
Continue interface setup after channel list update
ctrl_iface not configured!
[BRIDGE] Bridge adding wlan1
brctl: bridge br0: Device or resource busy
[   32.230610] br0: port 1(eth0) entered blocking state
[   32.231330] br0: port 1(eth0) entered forwarding state
[   32.233086] IPv6: ADDRCONF(NETDEV_CHANGE): br0: link becomes ready
[WLAN] WiFi Monitoring Start
[IPV6] START .....
Waiting for My IP6...
Waiting 0
ip: either "dev" is duplicate, or "permanent" is garbage
[   33.249633] cam-dummy-reg: disabling
Waiting 1
Waiting 2
[   35.183937] IPv6: ADDRCONF(NETDEV_CHANGE): wlan2: link becomes ready
[   35.185128] br0: port 2(wlan2) entered blocking state
[   35.185855] br0: port 2(wlan2) entered forwarding state
Waiting 3
[   36.177226] IPv6: ADDRCONF(NETDEV_CHANGE): wlan1: link becomes ready
[   36.178456] br0: port 3(wlan1) entered blocking state
[   36.179217] br0: port 3(wlan1) entered forwarding state
Waiting 4
Waiting 5
+++++++++++++++++++++++++++++++++++++++++++++++++++++
br0       Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet addr:100.5.5.1  Bcast:100.5.5.255  Mask:255.255.255.0
          inet6 addr: fe80::ba27:ebff:feaa:c5b5/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:3 errors:0 dropped:0 overruns:0 frame:0
          TX packets:9 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:318 (318.0 B)  TX bytes:774 (774.0 B)

eth0      Link encap:Ethernet  HWaddr B8:27:EB:FF:90:E0  
          inet6 addr: fe80::ba27:ebff:feff:90e0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:5 errors:0 dropped:0 overruns:0 frame:0
          TX packets:16 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:772 (772.0 B)  TX bytes:1360 (1.3 KiB)

...

wlan0     Link encap:Ethernet  HWaddr D4:6E:0E:13:B9:8C  
          inet addr:10.5.5.36  Bcast:10.5.5.255  Mask:255.255.255.0
          inet6 addr: fe80::d66e:eff:fe13:b98c/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:22 errors:0 dropped:0 overruns:0 frame:0
          TX packets:15 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:3170 (3.0 KiB)  TX bytes:2380 (2.3 KiB)

wlan1     Link encap:Ethernet  HWaddr D6:6E:0E:13:B9:8C  
          inet6 addr: fe80::d46e:eff:fe13:b98c/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:8 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

wlan2     Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet6 addr: fe80::ba27:ebff:feaa:c5b5/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:706 (706.0 B)



nameserver 127.0.0.1
nameserver 10.5.5.1
+++++++++++++++++++++++++++++++++++++++++++++++++++++
QT initialization

Linux Terminal ...


NTP 

Wed, 12 Apr 2023 04:34:52 +0000
 

_____                 _                              _____ _____ 
|  __ \               | |                            |  __ \_   _|
| |__) |__ _ ___ _ __ | |__   ___ _ __ _ __ _   _    | |__) || |  
|  _  // _` / __| '_ \| '_ \ / _ \ '__| '__| | | |   |  ___/ | |  
| | \ \ (_| \__ \ |_) | |_) |  __/ |  | |  | |_| |   | |    _| |_ 
|_|  \_\__,_|___/ .__/|_.__/ \___|_|  |_|   \__, |   |_|   |_____|
                | |                          __/ |              
                |_|                         |___/               



bash-5.1# 
bash-5.1# 
bash-5.1# 

  ```

### Procedures <a name="rpi3_procedures"></a>


#### Toolchain Building <a name="rpi3_toolchain"></a>

- When user want to setup a toolchain under <strong>/opt/rpi3</strong> folder.
```
  # sudo make TBOARD=rpi3 TOOLCHAIN_ROOT=/opt/rpi3 toolchain
```
- Without an option "TOOLCHAIN_ROOT=", the toolchain will be installed under <strong>./gnu/toolchain</strong> folder.
```
  # make TBOARD=rpi3 toolchain
```

 * "TBOARD" indicates type of board which is the name of folders in boards/ .
 * "TOOLCHAIN_ROOT" indicates the location of cross toolchain.


#### Libraries, Applications, Extra Applications Building <a name="rpi3_library"></a>

```
  # make TBOARD=rpi3 lib app ext
```

#### Booting Image Building <a name="rpi3_bootimage"></a>

```
  # make TBOARD=rpi3 board ramdisk extdisk
```

- The following 3 files will be created under <strong>./boards/rpi3/</strong> .
  * boot.tgz
  * rootfs.squashfs
  * image.ext4


- <span style="color:red"> <em> Kernel is referenced multiple times both at toochain building and booting image building. When the toolchain has been built with "sudo" command prefix to locate it under super-user priviledge folder (when you use "TOOLCHAIN_ROOT=" option), access conflict error may happen since the booting image building is accomplished under normal user priviledge.  
To prevent this, just simply clean up <strong>board/rpi3/kernel/build</strong> folder as follows, </em> </span>

```
  # sudo \rm -rf ./boards/rpi3/kernel/build/*
```

#### Creating UI image (Embedded QT - Cross-compilation) <a name="rpi3_ui"></a>


```
  # make TBOARD=rpi3 ui uidisk 
```

- It takes fairly long time to build up all subfolders under /uix.
- The final step is building Embedded QT. 

#### Formatting SD Card <a name="rpi3_sdcard"></a>


- Currently,the following 6 partitions should be used. (<strong>/dev/sde4</strong> is actually the top-level container for partitions; /dev/sde[5,6,7].)

```

Command (m for help): p
Disk /dev/sde: 59.64 GiB, 64021856256 bytes, 125042688 sectors
Disk model: MassStorageClass
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0xb44ddee8

Device     Boot    Start      End  Sectors  Size Id Type
/dev/sde1           2048   264191   262144  128M  c W95 FAT32 (LBA)
/dev/sde2         264192  2361343  2097152    1G 83 Linux
/dev/sde3        2361344 10749951  8388608    4G 83 Linux
/dev/sde4       10749952 27527167 16777216    8G  5 Extended
/dev/sde5       10752000 19140607  8388608    4G 83 Linux
/dev/sde6       19142656 21239807  2097152    1G 83 Linux
/dev/sde7       21241856 23339007  2097152    1G 83 Linux


```
- The following table shows the association between files and partitions.

| Partition  |  File            |
|------------| ---------------- |
|  /dev/sde1 |  boot.tgz        |
|  /dev/sde2 |  rootfs.squashfs |
|  /dev/sde3 |  image.ext4      |
|  /dev/sde5 |  ui.ext4         |
|  /dev/sde6 |  config.ext4     |
|  /dev/sde7 |       X          |

- /dev/sde5 is mounted into /root . /dev/sde7 are overlayed.
- User can copy images into partitions as follows.

```
  # sudo dd if=./board/rpi3/rootfs.squashfs of=/dev/sde2 bs=128M
  # sudo dd if=./board/rpi3/image.ext4      of=/dev/sde3 bs=128M  - It takes long time.
```

- Shell script to make these partitions automatically is available under board/rpi3 as follows.
  If MMC for raspberry PI was mounted on "/dev/sdc",

```
  # cd boards/rpi3
  # sudo ./format_sdcard.sh sdc
```

- Rootfs.squashfs , boot.tgz, image.ext4, config.ext4 are all copied into the disk.
- CAUTION) Existing partitions on the SDcard will be deleted upon running "format_sdcard.sh" .

#### WLAN Configuration <a name="rpi3_wlan"></a>


- User can configure required settings by using a tool **xcfgcli.sh** as follows. 

```
bash-5.1# 
bash-5.1# xcfgcli.sh get wan | jq
{
  "mode": "dhcp",
  "name": "wlan0",
  "mac": "random",
  "ssid": "finemyap",
  "password": "12345678",
  "ip": "192.167.0.162",
  "netmask": "255.255.255.0",
  "gateway": "192.167.0.1",
  "dns": "8.8.8.8",
  "ipv6": {
    "mode": "dhcp",
    "global": "2001:b0d8:2233:1000::",
    "local": "4400::",
    "plen": "64",
    "id": "1"
  },
  "capture": "0"
}
bash-5.1# 
```

- <strong>ssid</strong> and <strong>password</strong> in **wan** section are used for home gateway connection.

```
bash-5.1# 
bash-5.1# 
bash-5.1# xcfgcli.sh put wan/ssid MyHomeNetwork
MyHomeNetwork
bash-5.1# STORAGE UPDATE 

bash-5.1# 
bash-5.1# 
bash-5.1# xcfgcli.sh put wan/password onetwothree
onetwothree
bash-5.1# STORAGE UPDATE 

bash-5.1# reboot -f   
```


- User can see **/config/db** file and it keeps all configuration variable managed through **xcfgcli.sh**, and  **/etc/config.xml** is the XML file maintaining all configurable parameters defined in raspberry PI setup. 


#### Available Packages <a name="rpi3_packages"></a> 

- The following command prints the list of available packages from Microbsp. 
- <strong>MIBC_DEPENDS</strong> section inside of Makefile can choose any of those required . 


```
# make TBOARD=rpi3 pkglist

/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/bash.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/liblzma.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/libpcre2-posix.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/libnl-route-3.0.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/pam_misc.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/zlib.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/libnl-idiag-3.0.pc
/media/todd/work/github/microbsp/boards/rpi3/_install/disk/lib/pkgconfig/openssl.pc
...

/media/todd/work/github/microbsp/boards/rpi3/_stagedir/usr/lib/pkgconfig/libip4tc.pc
/media/todd/work/github/microbsp/boards/rpi3/_stagedir/usr/lib/pkgconfig/libgpiod.pc
/media/todd/work/github/microbsp/boards/rpi3/_stagedir/usr/lib/pkgconfig/gnutls.pc
  ...

```

- When your new pacakage needs both openssl and libz libraries, the following line needs to be added into Makefile. 
  Path should be omitted at <strong>MICB_DEPENDS</strong> line. 

```

  MICB_DEPENDS = openssl zlib 

```

## MBSP VM <a name="qemu"></a>

### Testbed components <a name="qemu_component"></a>


* [QEMU VM  Emulator](https://www.unixmen.com/how-to-install-and-configure-qemu-in-ubuntu/)
* USB Storage stick with "image.ext4"

```
  # cd boards/vm

  # sudo dmesg | grep "sd "
    ..
    [33889.430026] sd 7:0:0:0: [sdd] 30375936 512-byte logical blocks: (15.6 GB/14.5 GiB)
    [33889.431021] sd 7:0:0:0: [sdd] Write Protect is off
    [33889.431027] sd 7:0:0:0: [sdd] Mode Sense: 43 00 00 00
    [33889.432009] sd 7:0:0:0: [sdd] Write cache: disabled, read cache: enabled, doesn't support DPO or FUA
    [33889.458386] sd 7:0:0:0: [sdd] Attached SCSI removable disk
 
    We can know that USB disk is mounted as "sdd"

  # sudo dd if=image.ext4 of=/dev/sdd bs=32M 
 
    <It might take a long time to flash 6G image into USB stick. >

```


### Setup <a name="qemu_setup"></a>

* Just bash shell

### Booting Shot<a name="qemu_boot"></a>


  ```
  USB storage stick should be plugged in...


#
# make TBOARD=vm vmrun


SeaBIOS (version 1.15.0-1)


iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+3FF8B4A0+3FECB4A0 CA00
                                                                               


Booting from Hard Disk...
Boot failed: not a bootable disk

Booting from Floppy...
Boot failed: could not read the boot disk

Booting from DVD/CD...

ISOLINUX 6.04 6.04-pre1 ETCD Copyright (C) 1994-2015 H. Peter Anvin et al
Booting linux kernel....
Loading /boot/vmlinuz-5.17.7... 

....

[    0.000000] Linux version 5.17.7 (todd@vostro) (x86_64-any-linux-gnu-gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.38) #2 SMP PREEMPT Wed Feb 22 07:07:17 PST 2023
[    0.000000] Command line: BOOT_IMAGE=/boot/vmlinuz-5.17.7 console=ttyS0 root=/dev/ram0 ro rootfstype=squashfs rootwait
[    0.000000] KERNEL supported cpus:
[    0.000000]   Intel GenuineIntel
[    0.000000]   AMD AuthenticAMD
[    0.000000]   Hygon HygonGenuine
[    0.000000]   Centaur CentaurHauls
[    0.000000]   zhaoxin   Shanghai  
[    0.000000] x86/fpu: x87 FPU will use FXSAVE
[    0.000000] signal: max sigframe size: 1440
[    0.000000] BIOS-provided physical RAM map:
[    0.000000] BIOS-e820: [mem 0x0000000000000000-0x000000000009fbff] usable
[    0.000000] BIOS-e820: [mem 0x000000000009fc00-0x000000000009ffff] reserved
[    0.000000] BIOS-e820: [mem 0x00000000000f0000-0x00000000000fffff] reserved
[    0.000000] BIOS-e820: [mem 0x0000000000100000-0x00000000bffdffff] usable
[    0.000000] BIOS-e820: [mem 0x00000000bffe0000-0x00000000bfffffff] reserved
[    0.000000] BIOS-e820: [mem 0x00000000fffc0000-0x00000000ffffffff] reserved
[    0.000000] BIOS-e820: [mem 0x0000000100000000-0x000000013fffffff] usable
[    0.000000] NX (Execute Disable) protection: active
[    0.000000] SMBIOS 2.8 present.
[    0.000000] DMI: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.15.0-1 04/01/2014
[    0.000000] AGP: No AGP bridge found
[    0.000000] last_pfn = 0x140000 max_arch_pfn = 0x400000000
[    0.000000] x86/PAT: Configuration [0-7]: WB  WC  UC- UC  WB  WP  UC- WT  
[    0.000000] last_pfn = 0xbffe0 max_arch_pfn = 0x400000000
[    0.000000] found SMP MP-table at [mem 0x000f5ba0-0x000f5baf]
[    0.000000] RAMDISK: [mem 0xffffffff9993c000-0xffffffffa2f4afff]
[    0.000000] ACPI: Early table checksum verification disabled
[    0.000000] ACPI: RSDP 0x00000000000F59C0 000014 (v00 BOCHS )
[    0.000000] ACPI: RSDT 0x00000000BFFE1A1C 000034 (v01 BOCHS  BXPC     00000001 BXPC 00000001)
[    0.000000] ACPI: FACP 0x00000000BFFE18B8 000074 (v01 BOCHS  BXPC     00000001 BXPC 00000001)
[    0.000000] ACPI: DSDT 0x00000000BFFE0040 001878 (v01 BOCHS  BXPC     00000001 BXPC 00000001)
[    0.000000] ACPI: FACS 0x00000000BFFE0000 000040
[    0.000000] ACPI: APIC 0x00000000BFFE192C 000090 (v01 BOCHS  BXPC     00000001 BXPC 00000001)
[    0.000000] ACPI: HPET 0x00000000BFFE19BC 000038 (v01 BOCHS  BXPC     00000001 BXPC 00000001)
[    0.000000] ACPI: WAET 0x00000000BFFE19F4 000028 (v01 BOCHS  BXPC     00000001 BXPC 00000001)
[    0.000000] ACPI: Reserving FACP table memory at [mem 0xbffe18b8-0xbffe192b]
[    0.000000] ACPI: Reserving DSDT table memory at [mem 0xbffe0040-0xbffe18b7]
[    0.000000] ACPI: Reserving FACS table memory at [mem 0xbffe0000-0xbffe003f]
[    0.000000] ACPI: Reserving APIC table memory at [mem 0xbffe192c-0xbffe19bb]
[    0.000000] ACPI: Reserving HPET table memory at [mem 0xbffe19bc-0xbffe19f3]
[    0.000000] ACPI: Reserving WAET table memory at [mem 0xbffe19f4-0xbffe1a1b]
[    0.000000] No NUMA configuration found
[    0.000000] Faking a node at [mem 0x0000000000000000-0x000000013fffffff]
[    0.000000] NODE_DATA(0) allocated [mem 0x13ffd2000-0x13fffcfff]
[    0.000000] Zone ranges:
[    0.000000]   DMA      [mem 0x0000000000001000-0x0000000000ffffff]
[    0.000000]   DMA32    [mem 0x0000000001000000-0x00000000ffffffff]
[    0.000000]   Normal   [mem 0x0000000100000000-0x000000013fffffff]
[    0.000000]   Device   empty
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000000001000-0x000000000009efff]
[    0.000000]   node   0: [mem 0x0000000000100000-0x00000000bffdffff]
[    0.000000]   node   0: [mem 0x0000000100000000-0x000000013fffffff]
[    0.000000] Initmem setup node 0 [mem 0x0000000000001000-0x000000013fffffff]
[    0.000000] On node 0, zone DMA: 1 pages in unavailable ranges
[    0.000000] On node 0, zone DMA: 97 pages in unavailable ranges
[    0.000000] On node 0, zone Normal: 32 pages in unavailable ranges
[    0.000000] ACPI: PM-Timer IO Port: 0x608
[    0.000000] ACPI: LAPIC_NMI (acpi_id[0xff] dfl dfl lint[0x1])
[    0.000000] IOAPIC[0]: apic_id 0, version 32, address 0xfec00000, GSI 0-23
[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 0 global_irq 2 dfl dfl)
[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 5 global_irq 5 high level)
[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 9 global_irq 9 high level)
[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 10 global_irq 10 high level)
[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 11 global_irq 11 high level)
[    0.000000] ACPI: Using ACPI (MADT) for SMP configuration information
[    0.000000] ACPI: HPET id: 0x8086a201 base: 0xfed00000
[    0.000000] smpboot: Allowing 4 CPUs, 0 hotplug CPUs
[    0.000000] [mem 0xc0000000-0xfffbffff] available for PCI devices
[    0.000000] clocksource: refined-jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645519600211568 ns
[    0.000000] setup_percpu: NR_CPUS:8192 nr_cpumask_bits:4 nr_cpu_ids:4 nr_node_ids:1
[    0.000000] percpu: Embedded 53 pages/cpu s178392 r8192 d30504 u524288
[    0.000000] Fallback order for Node 0: 0 
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 1033952
[    0.000000] Policy zone: Normal
[    0.000000] Kernel command line: BOOT_IMAGE=/boot/vmlinuz-5.17.7 console=ttyS0 root=/dev/ram0 ro rootfstype=squashfs rootwait
[    0.000000] Unknown kernel command line parameters "BOOT_IMAGE=/boot/vmlinuz-5.17.7", will be passed to user space.
[    0.000000] Dentry cache hash table entries: 524288 (order: 10, 4194304 bytes, linear)
[    0.000000] Inode-cache hash table entries: 262144 (order: 9, 2097152 bytes, linear)
[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
[    0.000000] AGP: Checking aperture...
[    0.000000] AGP: No AGP bridge found
[    0.000000] Memory: 3877644K/4193784K available (14345K kernel code, 3163K rwdata, 4416K rodata, 2052K init, 2772K bss, 315880K reserved, 0K cma-reserved)
[    0.000000] ftrace: allocating 39521 entries in 155 pages
[    0.000000] ftrace: allocated 155 pages with 5 groups
[    0.000000] Dynamic Preempt: voluntary
[    0.000000] rcu: Preemptible hierarchical RCU implementation.
[    0.000000] rcu: 	RCU restricting CPUs from NR_CPUS=8192 to nr_cpu_ids=4.
[    0.000000] 	Trampoline variant of Tasks RCU enabled.
[    0.000000] 	Rude variant of Tasks RCU enabled.
[    0.000000] 	Tracing variant of Tasks RCU enabled.
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4
[    0.000000] NR_IRQS: 524544, nr_irqs: 456, preallocated irqs: 16
[    0.000000] Console: colour VGA+ 80x25
[    0.000000] printk: console [ttyS0] enabled
[    0.000000] ACPI: Core revision 20211217
[    0.000000] clocksource: hpet: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604467 ns
[    0.008000] APIC: Switch to symmetric I/O mode setup
[    0.020000] ..TIMER: vector=0x30 apic1=0 pin1=2 apic2=-1 pin2=-1
[    0.052000] tsc: Unable to calibrate against PIT
[    0.052000] tsc: using HPET reference calibration
[    0.052000] tsc: Detected 2660.024 MHz processor
[    0.001119] tsc: Marking TSC unstable due to TSCs unsynchronized
[    0.002923] Calibrating delay loop (skipped), value calculated using timer frequency.. 5320.04 BogoMIPS (lpj=10640096)
[    0.005431] pid_max: default: 32768 minimum: 301
[    0.013070] random: get_random_bytes called from setup_net+0x4d/0x290 with crng_init=0
[    0.023967] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.024823] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.073118] process: using AMD E400 aware idle routine
[    0.074011] Last level iTLB entries: 4KB 512, 2MB 255, 4MB 127
[    0.074622] Last level dTLB entries: 4KB 512, 2MB 255, 4MB 127, 1GB 0
[    0.075795] Spectre V1 : Mitigation: usercopy/swapgs barriers and __user pointer sanitization
[    0.077213] Spectre V2 : Mitigation: Retpolines
[    0.077613] Spectre V2 : Spectre v2 / SpectreRSB mitigation: Filling RSB on context switch
[    0.123296] Freeing SMP alternatives memory: 36K
[    0.277375] smpboot: CPU0: AMD QEMU Virtual CPU version 2.5+ (family: 0xf, model: 0x6b, stepping: 0x1)
[    0.307678] cblist_init_generic: Setting adjustable number of callback queues.
[    0.308667] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.311872] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.314058] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.315839] Performance Events: PMU not available due to virtualization, using software events only.
[    0.323800] rcu: Hierarchical SRCU implementation.
[    0.348479] NMI watchdog: Perf NMI watchdog permanently disabled
[    0.364837] smp: Bringing up secondary CPUs ...
[    0.388235] x86: Booting SMP configuration:
[    0.390704] .... node  #0, CPUs:      #1
[    0.000000] calibrate_delay_direct() failed to get a good estimate for loops_per_jiffy.
[    0.000000] Probably due to long platform interrupts. Consider using "lpj=" boot option.
[    0.638503]  #2
[    0.000000] calibrate_delay_direct() dropping min bogoMips estimate 4 = 8257229
[    0.000000] calibrate_delay_direct() dropping max bogoMips estimate 0 = 21306577
[    0.000000] calibrate_delay_direct() failed to get a good estimate for loops_per_jiffy.
[    0.000000] Probably due to long platform interrupts. Consider using "lpj=" boot option.
[    0.769549]  #3
[    0.000000] calibrate_delay_direct() failed to get a good estimate for loops_per_jiffy.
[    0.000000] Probably due to long platform interrupts. Consider using "lpj=" boot option.
[    0.931440] smp: Brought up 1 node, 4 CPUs
[    0.931440] smpboot: Max logical packages: 1
[    0.932020] smpboot: Total of 4 processors activated (8268.11 BogoMIPS)
[    1.015916] devtmpfs: initialized
[    1.032825] x86/mm: Memory block size: 128MB
[    1.079998] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    1.083035] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
[    1.089599] pinctrl core: initialized pinctrl subsystem
[    1.119509] NET: Registered PF_NETLINK/PF_ROUTE protocol family
[    1.136444] audit: initializing netlink subsys (disabled)
[    1.143330] audit: type=2000 audit(1677078859.192:1): state=initialized audit_enabled=0 res=1
[    1.159743] thermal_sys: Registered thermal governor 'step_wise'
[    1.160094] thermal_sys: Registered thermal governor 'user_space'
[    1.162144] cpuidle: using governor menu
[    1.178577] PCI: Using configuration type 1 for base access
[    1.195701] mtrr: your CPUs had inconsistent fixed MTRR settings
[    1.198937] mtrr: your CPUs had inconsistent variable MTRR settings
[    1.199409] mtrr: your CPUs had inconsistent MTRRdefType settings
[    1.199830] mtrr: probably your BIOS does not setup all CPUs.
[    1.200487] mtrr: corrected configuration.
[    1.331225] kprobes: kprobe jump-optimization is enabled. All kprobes are optimized if possible.
[    1.368872] HugeTLB registered 2.00 MiB page size, pre-allocated 0 pages
[    1.399063] cryptd: max_cpu_qlen set to 1000
[    1.428131] ACPI: Added _OSI(Module Device)
[    1.429066] ACPI: Added _OSI(Processor Device)
[    1.429423] ACPI: Added _OSI(3.0 _SCP Extensions)
[    1.429781] ACPI: Added _OSI(Processor Aggregator Device)
[    1.430643] ACPI: Added _OSI(Linux-Dell-Video)
[    1.431311] ACPI: Added _OSI(Linux-Lenovo-NV-HDMI-Audio)
[    1.431731] ACPI: Added _OSI(Linux-HPI-Hybrid-Graphics)
[    1.522627] ACPI: 1 ACPI AML tables successfully acquired and loaded
[    1.581320] ACPI: Interpreter enabled
[    1.583759] ACPI: PM: (supports S0 S5)
[    1.584791] ACPI: Using IOAPIC for interrupt routing
[    1.587484] PCI: Using host bridge windows from ACPI; if necessary, use "pci=nocrs" and report a bug
[    1.598498] ACPI: Enabled 2 GPEs in block 00 to 0F
[    1.795152] ACPI: PCI Root Bridge [PCI0] (domain 0000 [bus 00-ff])
[    1.800029] acpi PNP0A03:00: _OSC: OS supports [ASPM ClockPM Segments MSI HPX-Type3]
[    1.802872] acpi PNP0A03:00: _OSC: not requesting OS control; OS requires [ExtendedConfig ASPM ClockPM MSI]
[    1.814654] acpi PNP0A03:00: fail to add MMCONFIG information, can't access extended PCI configuration space under this bridge.
[    1.830079] PCI host bridge to bus 0000:00
[    1.831172] pci_bus 0000:00: root bus resource [io  0x0000-0x0cf7 window]
[    1.832713] pci_bus 0000:00: root bus resource [io  0x0d00-0xffff window]
[    1.834650] pci_bus 0000:00: root bus resource [mem 0x000a0000-0x000bffff window]
[    1.837532] pci_bus 0000:00: root bus resource [mem 0xc0000000-0xfebfffff window]
[    1.838117] pci_bus 0000:00: root bus resource [mem 0x140000000-0x1bfffffff window]
[    1.839384] pci_bus 0000:00: root bus resource [bus 00-ff]
[    1.845776] pci 0000:00:00.0: [8086:1237] type 00 class 0x060000
[    1.888432] pci 0000:00:01.0: [8086:7000] type 00 class 0x060100
[    1.893100] pci 0000:00:01.1: [8086:7010] type 00 class 0x010180
[    1.903176] pci 0000:00:01.1: reg 0x20: [io  0xc040-0xc04f]
[    1.907537] pci 0000:00:01.1: legacy IDE quirk: reg 0x10: [io  0x01f0-0x01f7]
[    1.908315] pci 0000:00:01.1: legacy IDE quirk: reg 0x14: [io  0x03f6]
[    1.911263] pci 0000:00:01.1: legacy IDE quirk: reg 0x18: [io  0x0170-0x0177]
[    1.911867] pci 0000:00:01.1: legacy IDE quirk: reg 0x1c: [io  0x0376]
[    1.914503] pci 0000:00:01.3: [8086:7113] type 00 class 0x068000
[    1.916439] pci 0000:00:01.3: quirk: [io  0x0600-0x063f] claimed by PIIX4 ACPI
[    1.917124] pci 0000:00:01.3: quirk: [io  0x0700-0x070f] claimed by PIIX4 SMB
[    1.921881] pci 0000:00:02.0: [1234:1111] type 00 class 0x030000
[    1.923752] pci 0000:00:02.0: reg 0x10: [mem 0xfd000000-0xfdffffff pref]
[    1.930652] pci 0000:00:02.0: reg 0x18: [mem 0xfebb0000-0xfebb0fff]
[    1.946503] pci 0000:00:02.0: reg 0x30: [mem 0xfeba0000-0xfebaffff pref]
[    1.947872] pci 0000:00:02.0: Video device with shadowed ROM at [mem 0x000c0000-0x000dffff]
[    1.977022] pci 0000:00:03.0: [8086:100e] type 00 class 0x020000
[    1.980125] pci 0000:00:03.0: reg 0x10: [mem 0xfeb80000-0xfeb9ffff]
[    1.984137] pci 0000:00:03.0: reg 0x14: [io  0xc000-0xc03f]
[    2.005711] pci 0000:00:03.0: reg 0x30: [mem 0xfeb00000-0xfeb7ffff pref]
[    2.080778] ACPI: PCI: Interrupt link LNKA configured for IRQ 10
[    2.088405] ACPI: PCI: Interrupt link LNKB configured for IRQ 10
[    2.097022] ACPI: PCI: Interrupt link LNKC configured for IRQ 11
[    2.103267] ACPI: PCI: Interrupt link LNKD configured for IRQ 11
[    2.105452] ACPI: PCI: Interrupt link LNKS configured for IRQ 9
[    2.155443] SCSI subsystem initialized
[    2.178155] ACPI: bus type USB registered
[    2.183665] usbcore: registered new interface driver usbfs
[    2.188235] usbcore: registered new interface driver hub
[    2.191465] usbcore: registered new device driver usb
[    2.203466] pps_core: LinuxPPS API ver. 1 registered
[    2.206679] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    2.215184] PTP clock support registered
[    2.264748] PCI: Using ACPI for IRQ routing
[    2.274033] hpet: 3 channels of 0 reserved for per-cpu timers
[    2.305337] clocksource: Switched to clocksource hpet
[    3.476114] VFS: Disk quotas dquot_6.6.0
[    3.491625] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    3.506486] FS-Cache: Loaded
[    3.528238] CacheFiles: Loaded
[    3.530200] pnp: PnP ACPI init
[    3.580840] pnp: PnP ACPI: found 6 devices
[    4.086361] clocksource: acpi_pm: mask: 0xffffff max_cycles: 0xffffff, max_idle_ns: 2085701024 ns
[    4.092137] NET: Registered PF_INET protocol family
[    4.098099] IP idents hash table entries: 65536 (order: 7, 524288 bytes, linear)
[    4.134274] tcp_listen_portaddr_hash hash table entries: 2048 (order: 3, 32768 bytes, linear)
[    4.134840] TCP established hash table entries: 32768 (order: 6, 262144 bytes, linear)
[    4.141414] TCP bind hash table entries: 32768 (order: 7, 524288 bytes, linear)
[    4.143879] TCP: Hash tables configured (established 32768 bind 32768)
[    4.168439] UDP hash table entries: 2048 (order: 4, 65536 bytes, linear)
[    4.169978] UDP-Lite hash table entries: 2048 (order: 4, 65536 bytes, linear)
[    4.181709] NET: Registered PF_UNIX/PF_LOCAL protocol family
[    4.200776] pci_bus 0000:00: resource 4 [io  0x0000-0x0cf7 window]
[    4.202313] pci_bus 0000:00: resource 5 [io  0x0d00-0xffff window]
[    4.205392] pci_bus 0000:00: resource 6 [mem 0x000a0000-0x000bffff window]
[    4.207509] pci_bus 0000:00: resource 7 [mem 0xc0000000-0xfebfffff window]
[    4.208715] pci_bus 0000:00: resource 8 [mem 0x140000000-0x1bfffffff window]
[    4.217999] pci 0000:00:01.0: PIIX3: Enabling Passive Release
[    4.219469] pci 0000:00:00.0: Limiting direct PCI/PCI transfers
[    4.220544] pci 0000:00:01.0: Activating ISA DMA hang workarounds
[    4.221819] PCI: CLS 0 bytes, default 64
[    4.239346] PCI-DMA: Using software bounce buffering for IO (SWIOTLB)
[    4.240027] software IO TLB: mapped [mem 0x00000000bbfe0000-0x00000000bffe0000] (64MB)
[    4.247518] kvm: no hardware support for 'kvm_intel'
[    4.316631] Trying to unpack rootfs image as initramfs...
[    4.325769] rootfs image is not initramfs (invalid magic at start of compressed archive); looks like an initrd
[    4.353019] kvm: Nested Virtualization enabled
[    4.361450] SVM: kvm: Nested Paging disabled
[    9.069777] Freeing initrd memory: 153660K
[    9.491056] PCLMULQDQ-NI instructions are not detected.
[    9.515756] Initialise system trusted keyrings
[    9.554796] workingset: timestamp_bits=52 max_order=20 bucket_order=0
[    9.556968] zbud: loaded
[    9.574330] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    9.575882] romfs: ROMFS MTD (C) 2007 Red Hat, Inc.
[    9.582129] fuse: init (API version 7.36)
[    9.688878] Key type asymmetric registered
[    9.692448] Asymmetric key parser 'x509' registered
[    9.697389] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 247)
[    9.702310] io scheduler mq-deadline registered
[    9.746789] crc32: CRC_LE_BITS = 64, CRC_BE BITS = 64
[    9.750182] crc32: self tests passed, processed 225944 bytes in 5542300 nsec
[    9.753091] crc32c: CRC_LE_BITS = 64
[    9.760067] crc32c: self tests passed, processed 225944 bytes in 760420 nsec
[    9.894325] crc32_combine: 8373 self tests passed
[    9.987847] crc32c_combine: 8373 self tests passed
[   10.127245] input: Power Button as /devices/LNXSYSTM:00/LNXPWRBN:00/input/input0
[   10.129662] ACPI: button: Power Button [PWRF]
[   10.243648] Serial: 8250/16550 driver, 32 ports, IRQ sharing enabled
[   10.267500] 00:04: ttyS0 at I/O 0x3f8 (irq = 4, base_baud = 115200) is a 16550A
[   10.755324] brd: module loaded
[   10.982270] loop: module loaded
[   11.555019] zram: Added device: zram0
[   11.621373] scsi host0: ata_piix
[   11.675919] scsi host1: ata_piix
[   11.696326] ata1: PATA max MWDMA2 cmd 0x1f0 ctl 0x3f6 bmdma 0xc040 irq 14
[   11.697274] ata2: PATA max MWDMA2 cmd 0x170 ctl 0x376 bmdma 0xc048 irq 15
[   11.753966] slram: not enough parameters.
[   11.790702] tun: Universal TUN/TAP device driver, 1.6
[   11.802603] igb: Intel(R) Gigabit Ethernet Network Driver
[   11.803147] igb: Copyright (c) 2007-2014 Intel Corporation.
[   11.806291] igbvf: Intel(R) Gigabit Virtual Function Network Driver
[   11.807424] igbvf: Copyright (c) 2009 - 2012 Intel Corporation.
[   11.809720] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
[   11.812464] ehci-pci: EHCI PCI platform driver
[   11.816226] ehci-platform: EHCI generic platform driver
[   11.818538] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
[   11.820485] ohci-pci: OHCI PCI platform driver
[   11.821853] ohci-platform: OHCI generic platform driver
[   11.825386] uhci_hcd: USB Universal Host Controller Interface driver
[   11.850004] usbcore: registered new interface driver uas
[   11.853053] usbcore: registered new interface driver usb-storage
[   11.855237] usbcore: registered new interface driver ums-sddr09
[   11.857207] usbcore: registered new interface driver ums-sddr55
[   11.860413] UDC core: couldn't find an available UDC - added [g_ether] to list of pending drivers
[   11.865209] i8042: PNP: PS/2 Controller [PNP0303:KBD,PNP0f13:MOU] at 0x60,0x64 irq 1,12
[   11.884243] serio: i8042 KBD port at 0x60,0x64 irq 1
[   11.885470] serio: i8042 AUX port at 0x60,0x64 irq 12
[   11.901342] mousedev: PS/2 mouse device common for all mice
[   11.908787] i2c_dev: i2c /dev entries driver
[   11.918176] input: AT Translated Set 2 keyboard as /devices/platform/i8042/serio0/input/input1
[   11.958911] NET: Registered PF_INET6 protocol family
[   12.013800] ata2: found unknown device (class 0)
[   12.049473] ata2.00: ATAPI: QEMU DVD-ROM, 2.5+, max UDMA/100
[   12.081053] Segment Routing with IPv6
[   12.093985] In-situ OAM (IOAM) with IPv6
[   12.128492] NET: Registered PF_PACKET protocol family
[   12.136036] NET: Registered PF_KEY protocol family
[   12.138782] ata1.00: ATA-7: QEMU HARDDISK, 2.5+, max UDMA/100
[   12.165905] 8021q: 802.1Q VLAN Support v1.8
[   12.169051] Key type dns_resolver registered
[   12.171227] IPI shorthand broadcast: enabled
[   12.175815] ata1.00: 12582912 sectors, multi 16: LBA48 
[   12.183430] ata1.01: ATA-7: QEMU HARDDISK, 2.5+, max UDMA/100
[   12.187895] ata1.01: 20480 sectors, multi 16: LBA48 
[   12.220531] registered taskstats version 1
[   12.256450] Loading compiled-in X.509 certificates
[   12.266943] scsi 0:0:0:0: Direct-Access     ATA      QEMU HARDDISK    2.5+ PQ: 0 ANSI: 5
[   12.298138] zswap: loaded using pool lzo/zbud
[   12.311008] scsi 0:0:0:0: Attached scsi generic sg0 type 0
[   12.325703] Key type ._fscrypt registered
[   12.329789] Key type .fscrypt registered
[   12.331151] Key type fscrypt-provisioning registered
[   12.378413] sd 0:0:0:0: [sda] 12582912 512-byte logical blocks: (6.44 GB/6.00 GiB)
[   12.380381] scsi 0:0:1:0: Direct-Access     ATA      QEMU HARDDISK    2.5+ PQ: 0 ANSI: 5
[   12.395334] sd 0:0:0:0: [sda] Write Protect is off
[   12.425012] sd 0:0:0:0: [sda] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
[   12.432849] scsi 0:0:1:0: Attached scsi generic sg1 type 0
[   12.458735] sd 0:0:1:0: [sdb] 20480 512-byte logical blocks: (10.5 MB/10.0 MiB)
[   12.458735] sd 0:0:1:0: [sdb] Write Protect is off
[   12.490260] sd 0:0:1:0: [sdb] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
[   12.523944] scsi 1:0:0:0: CD-ROM            QEMU     QEMU DVD-ROM     2.5+ PQ: 0 ANSI: 5
[   12.618150] sr 1:0:0:0: [sr0] scsi3-mmc drive: 4x/4x cd/rw xa/form2 tray
[   12.631790] cdrom: Uniform CD-ROM driver Revision: 3.20
[   12.938423] sd 0:0:0:0: [sda] Attached SCSI disk
[   12.943377] BIOS EDD facility v0.16 2004-Jun-25, 2 devices found
[   12.959844] sr 1:0:0:0: Attached scsi generic sg2 type 5
[   12.987595] Unstable clock detected, switching default tracing clock to "global"
[   12.987595] If you want to keep using the local clock, then add:
[   12.987595]   "trace_clock=local"
[   12.987595] on the kernel command line
[   13.016793] sd 0:0:1:0: [sdb] Attached SCSI disk
[   13.114384] RAMDISK: squashfs filesystem found at block 0
[   13.115467] RAMDISK: Loading 153659KiB [1 disk] into ram disk... \
[   13.892863] /
[   14.679847] \
[   15.459319] /
[   16.261394] \
[   17.205096] /
[   18.531587] \
[   20.379675] /
[   22.947336] \
[   25.015876] /
[   26.060074] \
[   27.313731] /
[   28.369103] \
[   29.458131] /
[   31.709434] \
[   33.632423] /
[   36.102712] -
[   37.745745] hrtimer: interrupt took 30841560 ns
[   37.765698] |
[   40.489505] -
[   41.905271] done.
[   44.852320] VFS: Mounted root (squashfs filesystem) readonly on device 1:0.
[   44.859630] devtmpfs: mounted
[   45.012534] Freeing unused kernel image (initmem) memory: 2052K
[   45.013851] Write protecting the kernel read-only data: 22528k
[   45.031963] Freeing unused kernel image (text/rodata gap) memory: 2036K
[   45.044961] Freeing unused kernel image (rodata/data gap) memory: 1728K
[   45.609175] x86/mm: Checked W+X mappings: passed, no W+X pages found.
[   45.614217] Run /sbin/init as init process
[   48.373486] EXT4-fs (sda): mounted filesystem with ordered data mode. Quota mode: none.
Mounting...
[   48.809431] overlayfs: "xino" feature enabled using 32 upper inode bits.
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# chroot /ovr/
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# so[   58.015708] random: fast init done
ur[   58.730220] random: crng init done
ce /etc/rc.init


       ___    __      __ _  _     _      _                  
      / _ \  / /     / /| || |   | |    (_)                 
__  _| (_) |/ /_    / /_| || |_  | |     _ _ __  _   ___  __
\ \/ /> _ <| '_ \  | '_ \__   _| | |    | | '_ \| | | \ \/ /
 >  <| (_) | (_) | | (_) | | |   | |____| | | | | |_| |>  < 
/_/\_\\___/ \___/   \___/  |_|   |______|_|_| |_|\__,_/_/\_\
                                                            



chpasswd: password for 'root' changed

XCFGD Configurator - wait 5

[   68.976417] EXT4-fs (sdb): mounted filesystem with ordered data mode. Quota mode: none.
Waiting it for getting ready to work..
Daemon Running...
DBG:main                :649  TEMPORARY FILE - SYNC : /var/tmp/xcfgXZtTOBx
DBG:main                :673  XML1 : /etc/config.xml
DBG:main                :669  DEV : /config/db
DBG:main                :665  STANDADLONE MODE
DBG:main                :752  TEMPORARY DIRECTORY = (/var/tmp/170)
MSG:xml_storage_open    :1222 /config/db Physical Information = ( 4 x 128 Kbytes + 2 x 1024 Kbytes ) 4 
DBG:xml_storage_open    :1236 /config/db has been created
DBG:xml_storage_open    :1246 /config/db zeored 
MSG:xml_storage_open    :1262 FILE /config/db SIZE=2621440 
MSG:xml_storage_open    :1312 xml_storage_open : HEADER ANALSYS [0/4] 
MSG:xml_storage_open    :1320 xml_storage_open : HEADER LOCATION = ( 00000000 00020000 00040000 00060000 )
MSG:xml_storage_open    :1351 DATA[ 0 ] has 8 blocks 
MSG:xml_storage_open    :1351 DATA[ 1 ] has 8 blocks 
MSG:xml_storage_open    :1364 xml_storage_open : DATA LOCATION = ( 00080000 00180000 )
DBG:xml_storage_open    :1374 /config/db first boot up 
DBG:_xml_storage_clean  :652  DATA  [ 0  ] START = 0x180000 / DATA SIZE = 1048576
DBG:_xml_storage_clean  :652  DATA  [ 1  ] START = 0x280000 / DATA SIZE = 1048576
DBG:_xml_storage_clean  :670  Storage Header Size = 131072 
DBG:_xml_storage_clean  :698  HEADER[ 0  ] START = 0x20000 / DATA OFFSET = 524288 (0) / DIRTY = 1 
DBG:_xml_storage_clean  :698  HEADER[ 1  ] START = 0x40000 / DATA OFFSET = 1572864 (1) / DIRTY = 0 
DBG:_xml_storage_clean  :698  HEADER[ 2  ] START = 0x60000 / DATA OFFSET = 524288 (0) / DIRTY = 0 
DBG:_xml_storage_clean  :698  HEADER[ 3  ] START = 0x80000 / DATA OFFSET = 1572864 (1) / DIRTY = 0 
DBG:_xml_storage_header_fetch:767  DIRTY HEADER[ 0  ] START = 0x20000 / DATA OFFSET = 524288 FLAG=0000 
DBG:_xml_storage_initialize:1071 INITIAL DATA COPY FROM /etc/config.xml 
DBG:_xml_storage_initialize:1087 /etc/config.xml read 2183/2183 
DBG:_xml_storage_initialize:1100 SIZE=2183 CRC=0x72df DATA=00080000 
DBG:_xml_storage_header_fetch:767  DIRTY HEADER[ 0  ] START = 0x20000 / DATA OFFSET = 524288 FLAG=0000 
MSG:xml_storage_validate:1440 HEADER[0 ]::MAGIC=OK 
MSG:xml_storage_validate:1441 HEADER[0 ]::RSIZE=2183 
MSG:xml_storage_validate:1442 HEADER[0 ]::START=80000 
MSG:xml_storage_validate:1443 HEADER[0 ]::CRC  =72df 
MSG:xml_storage_validate:1493 Temporary XML file name : (/var/tmp/170/config.xml) 
DBG:xml_storage_validate:1507 /etc/config.xml write 2183/2183 
MSG:xml_storage_validate:1519 STORAGE VALIDATED [0] 
3
DBG:config_xml_merge    :401  VERSION (1.0.0.2 vs 1.0.0.2) 
DBG:config_xml_merge    :404  CURRENTLY LATEST VERSION
2
1


DATE 

Sat Oct 15 12:00:00 UTC 2022

Logging daemon


USER 

chpasswd: password for 'todd' changed
User [ todd/12345678 ] created

GIT 


PYTHON  


Network - VMware should use Host-only interface for this OS

[  101.324824] e1000: Intel(R) PRO/1000 Network Driver
[  101.325525] e1000: Copyright (c) 1999-2006 Intel Corporation.
[  105.215736] ACPI: \_SB_.LNKC: Enabled at IRQ 11
[  105.652463] e1000 0000:00:03.0 eth0: (PCI:33MHz:32-bit) 02:a8:c3:af:78:16
[  105.654483] e1000 0000:00:03.0 eth0: Intel(R) PRO/1000 Network Connection
udhcpc: started, v1.35.0
Setting IP address 0.0.0.0 on eth0
[  111.942569] 8021q: adding VLAN 0 to HW filter on device eth0
[  111.950152] e1000: eth0 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: RX
[  112.024063] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
udhcpc: broadcasting discover
udhcpc: broadcasting discover
udhcpc: broadcasting select for 10.5.5.58, server 10.5.5.1
udhcpc: lease of 10.5.5.58 obtained from 10.5.5.1, lease time 120
Setting IP address 10.5.5.58 on eth0
Deleting routers
route: SIOCDELRT: No such process
Adding router 10.5.5.1
Recreating /etc/resolv.conf
 Adding DNS server 10.5.5.1

Network - Interface Capture


SSH key setup in /root/.ssh 


SSH daemon


NTP 

Wed, 22 Feb 2023 15:16:28 +0000

WEBUI 


SHELL

bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# ifconfig
eth0      Link encap:Ethernet  HWaddr 02:89:C0:96:2C:A0  
          inet addr:10.5.5.58  Bcast:10.5.5.255  Mask:255.255.255.0
          inet6 addr: fe80::89:c0ff:fe96:2ca0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:10 errors:0 dropped:0 overruns:0 frame:0
          TX packets:16 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:1582 (1.5 KiB)  TX bytes:2037 (1.9 KiB)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:192 errors:0 dropped:0 overruns:0 frame:0
          TX packets:192 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:14031 (13.7 KiB)  TX bytes:14031 (13.7 KiB)

bash-5.1# 
bash-5.1# 

  ```

- User can see "eth0" interface is activated in VM, which will be named as "tap0" in host. 

### Procedures <a name="qemu_procedures"></a>


#### Toolchain <a name="qemu_toolchain"></a>

- When user want to setup a toolchain under <strong>/opt/qvm</strong> folder.
```
  # sudo make TBOARD=vm TOOLCHAIN_ROOT=/opt/qvm toolchain
```
- With the following command, the toolchain will be setup under <strong>./gnu/toolchain</strong> folder.
```
  # make TBOARD=vm toolchain
```

 * "TBOARD" indicates type of board which is the name of folders in boards/ .
 * "TOOLCHAIN_ROOT" indicates the location of cross toolchain.

#### Libraries, Applications, Extra Applications <a name="qemu_library"></a>

```
  # make TBOARD=vm lib app ext
```

#### Platform specfic binaries ( HTTP server, currently )  <a name="qemu_http"></a>

```
  # make TBOARD=vm proj 
```

#### Booting Image Creation <a name="qemu_bootimage"></a>

```
  # make TBOARD=vm board ramdisk extdisk 
```


- Unlikely with the case of raspberry PI, we will get "output.iso" file under boards/vm .

#### Partitioning external USB disk  <a name="qemu_partition"></a>


```
  # sudo dd if=boards/vm/image.ext4 of=/dev/sdd1 bs=128M 
  # sudo mkfs.ext4 -F /dev/sdd2
```

- /dev/sdd USB memory stick is partitioned as follows. 

| Partition |  Size           |
|-----------| --------------- |
|  sdd1/    |  10Gbytes       |
|  sdd2/    |  10Mbytes       |

- /dev/sdd1 will be used for main R/W disk space. 
- /dev/sdd2 will be used for configuration data space. 

```
# sudo fdisk /dev/sdd

Welcome to fdisk (util-linux 2.37.2).
...


Command (m for help): p
Disk /dev/sdd: 57.84 GiB, 62109253632 bytes, 121307136 sectors
Disk model: Ultra Fit
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x6fb5f594

Device     Boot    Start      End  Sectors Size Id Type
/dev/sdd1           2048 20973567 20971520  10G 83 Linux
/dev/sdd2       20973568 20994047    20480  10M 83 Linux
```

#### Running/Stopping "output.iso" <a name="qemu_control"></a>

- We can run this file as below.
```
  # make TBOARD=vm vmrun
```

  If you want to explicitly specify USB stick device, use the following option "EXT4HDD" . 

```
  # make TBOARD=vm EXT4HDD=/dev/sde1 vmrun 
```

  If you want to explicitly specify configuration partition device, use the following option "EXT4CFG" . 

```
  # make TBOARD=vm EXT4HDD=/dev/sde1 EXT4CFG=/dev/sdg2 vmrun 
```

- We can stop running QEMU emulator.
```
  # make TBOARD=vm vmstop
```

### Host Network <a name="qemu_hostnetwork"></a>

The following is the example of active network interfaces in Ubuntu 22.04. 
```
# ifconfig
br0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.5.5.1  netmask 255.255.255.0  broadcast 10.5.5.255
        inet6 fe80::f4f8:90ff:fece:b6be  prefixlen 64  scopeid 0x20<link>
        ether f6:f8:90:XX:XX:XX  txqueuelen 1000  (Ethernet)
        RX packets 258316  bytes 18293695 (18.2 MB)
        RX errors 0  dropped 25  overruns 0  frame 0
        TX packets 984870  bytes 3220903936 (3.2 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether f0:4d:a2:XX:XX:XX  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 51  bytes 4403 (4.4 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 51  bytes 4403 (4.4 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

tap0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet6 fe80::2c8c:afff:fec4:d674  prefixlen 64  scopeid 0x20<link>
        ether 2e:8c:af:XX:XX:XX  txqueuelen 1000  (Ethernet)
        RX packets 72  bytes 10241 (10.2 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 94  bytes 12755 (12.7 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlan0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.167.0.111  netmask 255.255.255.0  broadcast 192.167.0.255
        inet6 fe80::1351:a93d:5d08:39aa  prefixlen 64  scopeid 0x20<link>
        ether 90:de:80:XX:XX:XX  txqueuelen 1000  (Ethernet)
        RX packets 2275381  bytes 3428022487 (3.4 GB)
        RX errors 0  dropped 317  overruns 0  frame 0
        TX packets 294001  bytes 38305219 (38.3 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlan240: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether 60:02:b4:XX:XX:XX  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 1608  bytes 114212 (114.2 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlanap0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether 92:de:80:XX:XX:XX  txqueuelen 1000  (Ethernet)
        RX packets 260771  bytes 31296760 (31.2 MB)
        RX errors 0  dropped 30  overruns 0  frame 0
        TX packets 2212617  bytes 3365397554 (3.3 GB)
        TX errors 0  dropped 1556 overruns 0  carrier 0  collisions 0

```

- The network can be illustrated as follows. 

![](doc/host_network.png)


- Each of the network interfaces....
 

| Interface |  Device      |   Description                              |
|-----------| -------------|------------------------------------------- |
|  eth0     |  Laptop      | Built-in ethernet interface                |
|  tap0     |  MBSP VM     | Virtual ethernet interface created by MBSP VM |
|  br0      |  Laptop      | Bridge interface to tie up network interfaces |
|  wlan0    |  Realtek Dongle | 802.11a WLAN Client interface           |
|  wlanap0  |  Realtek Dongle | 802.11g/n WLAN AP interface             |
|  wlan240  |  Laptop(PCI) | 802.11g WLAN AP interface - Atheros PCI card |


- Currently, **eth0** interface is not used. 
 
- **wlan0** 802.11a client interface is created from [Realtek device](https://www.amazon.com/dp/B09KKLCHZZ?psc=1&ref=ppx_yo2ov_dt_b_product_details) dongle for WAN network access. It attaches to a home gateway. **wlanap0** interface is also created from the same Realtek device dongle and it is configured as 802.11a AP. External user devices with 802.11a capability may attach to this AP network. 

- Laptop built-in WiFi is activated as **wlan240** for 2.4GHz 802.11g AP network. My dell vostro laptop has very old "Atheros WiFi SDcard" and it can only supports 802.11g speed.

- **br0** is basically aggregating 2 interfaces; **wlanap0** and **wlan240**. All the devices attached to one of both interfaces will get the same domain of IP address if a DHCP server would be activated on top of **br0** interface. 

- NAT(Network Address Translation) is configured between **wlan0** (WAN interface) and **br0** (LAN interface). 


#### Realtek WLAN configuration <a name="qemu_realtek"></a>


- [External WiFi dongle](https://www.amazon.com/dp/B09KKLCHZZ?psc=1&ref=ppx_yo2ov_dt_b_product_details) is used for Client and AP support at the same time. This product is based on Realtek **8822BU** high speed WiFi chipset. 
- Usually depending on device driver configuration of Realtek driver software, we can easily create both Wireless client and AP interfaces at the same time. 
- You can download and setup 8822BU device driver [here](https://github.com/morrownr/88x2bu-20210702). 
- User needs edit a file in the driver to make enable AP and client modes at the same time as follows. **CONFIG_CONCURRENT_MODE** should be turned on. 

```
~work/88x2bu-20210702# cat Makefile | grep "CONFIG_CONCURRENT_MODE"
..
EXTRA_CFLAGS += -DCONFIG_CONCURRENT_MODE
..

```

- Before heading to the build process, there is a few of utilities needed for building realtek driver. 

```
~work/88x2bu-20210702# sudo apt install -y build-essential dkms git iw
```

- Build command is as follows. 

```
~work/88x2bu-20210702# sudo ./install-driver.sh 
```


- If the build was completed successfully, we can see **/etc/modprobe.d/88x2bu.conf**. To activate AP interface together with client interface, we better choose the following **options** line. 


```
~work/88x2bu-20210702# cat /etc/modprobe.d/88x2bu.conf
# /etc/modprobe.d/88x2bu.conf
#
...
#		wifi: rtw88: Add rtw8822bu chipset support
#
# The following line blacklists (deactivates) the above in-kernel driver.
blacklist rtw88_8822bu
#
# Edit the following line to change, add or delete options:
#options 88x2bu rtw_drv_log_level=1 rtw_led_ctrl=1 rtw_vht_enable=1 rtw_switch_usb_mode=0
#
# Note: To activate USB3 mode, change rtw_switch_usb_mode above to rtw_switch_usb_mode=1
#
# Note: The above `options` line is a good default for managed mode. Below is
# an example for AP mode. Modify as required after reading the documentation:
#options 88x2bu rtw_drv_log_level=1 rtw_led_ctrl=1 rtw_vht_enable=2 rtw_power_mgnt=1 rtw_beamform_cap=1 rtw_switch_usb_mode=1 rtw_dfs_region_domain=1
options 88x2bu rtw_drv_log_level=1 rtw_vht_enable=2 rtw_power_mgnt=1 rtw_beamform_cap=1 rtw_switch_usb_mode=0 rtw_dfs_region_domain=1
#
...

```

##### 802.11a AP <a name=qemu_80211a_ap> </a>

- We can create AP instance as follows.

```
# sudo hostapd -dddd -B ./hostapd_a.conf
```

- **hostapd_a.conf**

```
# cat ./hostapd_a.conf
interface=wlanap0
ctrl_interface=/var/tmp/hostapd/ctrl
bridge=br0
driver=nl80211
ssid=TestAP_Highspeed
channel=36
ieee80211d=1
ieee80211h=1
ieee80211n=1
require_ht=1
ieee80211ac=1
wmm_enabled=1
country_code=US
hw_mode=a
macaddr_acl=0
auth_algs=3
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=12345678
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

```

- User can see a network ID **TestAP_Highspeed** . Passwortd is **12345678**. 

##### 802.11g AP <a name=qemu_80211g_ap> </a>


- We can create AP instance as follows.

```
# sudo hostapd -dddd -B ./hostapd_g.conf
```

- **hostapd_g.conf**

```
# cat hostapd_g.conf
interface=wlan240
ctrl_interface=/var/tmp/hostapd2/ctrl
bridge=br0
driver=nl80211
ssid=TestAP_lowspeed
channel=6
ieee80211d=1
ieee80211h=1
wmm_enabled=1
country_code=US
hw_mode=g
macaddr_acl=0
auth_algs=3
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=12345678
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

```

- User can see a network ID **TestAP_lowspeed** . Passwortd is **12345678**. 

#### Renaming network interfaces <a name="qemu_renaming_ubuntu"></a>

- Aliasing the name of network interface can be done through **/etc/udev/rules.d/70-persistent-net.rules** file as follows. 

```
~work/88x2bu-20210702# cat /etc/udev/rules.d/70-persistent-net.rules 
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{address}=="60:02:b4:XX:XX:XX", ATTR{dev_id}=="0x0", ATTR{type}=="1", NAME="wlan240"
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{address}=="90:de:80:XX:XX:XX", ATTR{dev_id}=="0x0", ATTR{type}=="1", NAME="wlan0"
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{address}=="92:de:80:XX:XX:XX", ATTR{dev_id}=="0x0", ATTR{type}=="1", NAME="wlanap0"
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{address}=="f0:4d:a2:XX:XX:XX", ATTR{dev_id}=="0x0", ATTR{type}=="1", NAME="eth0"
~work/88x2bu-20210702#

```



#### NAT network configuration <a name="qemu_nat"></a>

- **NAT** is needed between **wlan0** (WAN) and **br0** (LAN). 
- Ubuntu 22.04 is based on **ufw** system and it has to be changed to support **IP forwarding** and it can be done through **/etc/default/ufw** 
- **DEFAULT_FORWARD_POLICY** should be **ACCEPT** not **DROP** as follows. 

```
~work/88x2bu-20210702# cat /etc/default/ufw
# /etc/default/ufw
#

# Set to yes to apply rules to support IPv6 (no means only IPv6 on loopback
# accepted). You will need to 'disable' and then 'enable' the firewall for
# the changes to take affect.
IPV6=yes

# Set the default input policy to ACCEPT, DROP, or REJECT. Please note that if
# you change this you will most likely want to adjust your rules.
DEFAULT_INPUT_POLICY="DROP"

# Set the default output policy to ACCEPT, DROP, or REJECT. Please note that if
# you change this you will most likely want to adjust your rules.
DEFAULT_OUTPUT_POLICY="ACCEPT"

#
# Todd : DROP -> ACCEPT 
#
# Set the default forward policy to ACCEPT, DROP or REJECT.  Please note that
# if you change this you will most likely want to adjust your rules
DEFAULT_FORWARD_POLICY="ACCEPT"

# Set the default application policy to ACCEPT, DROP, REJECT or SKIP. Please
# note that setting this to ACCEPT may be a security risk. See 'man ufw' for
...

```

- The following **NAT** masquerading option should be added into **/etc/ufw/before.rules** . We can see the output interface is specified as **wlan0** and the source address of routed packets is "10.5.5.x". This will be the range of LAN IP address. 

```
~work/88x2bu-20210702# sudo cat /etc/ufw/before.rules 
#
# rules.before
#
# Rules that should be run before the ufw command line added rules. Custom
# rules should be added to one of these chains:
#   ufw-before-input
#   ufw-before-output
#   ufw-before-forward
#

# NAT Table rules
*nat
:POSTROUTING ACCEPT [0:0]

# Forward Traffic
-A POSTROUTING -s 10.5.5.0/24 -o wlan0 -j MASQUERADE

COMMIT
...

```

- More detail description is found [here](https://semfionetworks.com/blog/wlan-pi-bridge-wi-fi-hotspot-to-ethernet-interface/)

#### DHCP/DNS configuration <a name="qemu_dns"></a>

- DHCP and DNS service instance is created on top of **br0** interface (1) to distribute private IP addresses to all outstanding devices attached to either **wlanap0** or **wlan240** AP interfaces.
- All ubuntu VM machines created with **tapX** interface which becomes a member of bridge interface **br0** will receive the same domain IP address. 
- Detail information about DHCP/DNS configuration will be found [here](https://computingforgeeks.com/install-and-configure-dnsmasq-on-ubuntu/).

- The following shows how **br0** LAN bridge network works if one Ubuntu VM was created.  


```
# sudo brctl show br0
bridge name     bridge id               STP enabled     interfaces
br0             8000.f6f890ceb6be       no              tap0
                                                        wlan240
                                                        wlanap0
```

- We are using "dnsmasq" utility and it can be downloaded into Ubuntu with a command. 

```
# sudo apt install dnsmasq
```

- **/etc/dnsmasq** configuration file needs to be properly edited as follows. 

```
# You can control how dnsmasq talks to a server: this forces
# queries to 10.1.2.3 to be routed via eth1
# server=10.1.2.3@eth1
server=8.8.8.8@wlan0

# If you want dnsmasq to listen for DHCP and DNS requests only on
# specified interfaces (and the loopback) give the name of the
# interface (eg eth0) here.
# Repeat the line for more than one interface.
interface=br0

# Uncomment this to enable the integrated DHCP server, you need
# to supply the range of addresses available for lease and optionally
# a lease time. If you have more than one network, you will need to
# repeat this for each network on which you want to supply DHCP
# service.
dhcp-range=10.5.5.10,10.5.5.100,12h

dhcp-option=option:router,10.5.5.1
dhcp-option=option:dns-server,10.5.5.1
dhcp-option=option:ntp-server,10.5.5.1
dhcp-option=option:netmask,255.255.255.0
dhcp-leasefile=/var/tmp/dnsmasq.leases

```

- **server=** indicates "where dnsmasq will ask domain name service" . In this case, all the domain name service will be asked to google server(8.8.8.8) through **wlan0** WAN interface.

- **interface=** indicates "which interface will be monitored for DNS/DHCP service".
 
#### Host Network Setup Script <a name="qemu_script"></a>

- Totally automated setup is as follows. 

```
# cat ./setup_network.sh
#!/bin/sh

##
## hostapd_a.conf - 11A mode
## hostapd_g.conf - 11G mode
##
HOSTAP_A_CONF=./hostapd_a.conf
HOSTAP_G_CONF=./hostapd_g.conf

## Applying NAT/IP forward configuration
sudo ufw disable
sudo ufw enable 
sudo ufw disable
sudo iptables -t nat -L -v
sleep 1

sudo brctl addbr br0 

\rm -rf ./hostap*.log

## N/A AP
sudo hostapd -ddddd -B $HOSTAP_A_CONF > ./hostap_a.log

## G AP
sudo hostapd -ddddd -B $HOSTAP_G_CONF > ./hostap_g.log

sleep 1

## bridge interface setup 
sudo ifconfig br0 up
sudo ifconfig br0 10.5.5.1 netmask 255.255.255.0 up

# DNS
# - disabling pre-installed DNS resolver service 
sudo systemctl disable systemd-resolved
sudo systemctl stop systemd-resolved

if [ -s /etc/resolv.conf ]; then
    sudo unlink /etc/resolv.conf
    echo nameserver 8.8.8.8 | sudo tee /etc/resolv.conf
fi

##
## Reactivates new service ... 
sudo systemctl restart dnsmasq

```


## Selective Compilation <a name="selective_compile"></a>


- Each applications/libaries are located under folders names among apps/, libs/, exts/ and they are all associated with specific category names.  

| Folders  |  Category Name   |
|----------| ---------------- |
|  apps/    |  app            |
|  libs/    |  lib            |
|  exts/    |  ext            |


- If you want to build "apps/vim" folder for VIM application, just do the following. "distclean" command removes the build temporary folder.

```
  # make TBOARD=vm SUBDIR=vim distclean app
```


- Building inside of each subfolders has 3 steps; prerpare, all, install. Each substep is triggered through {app|lib|ext}_### as below.  


```
  # make TBOARD=vm SUBDIR=vim app_all  
```
- If you want to build "exts/python" folder,  

```
  # make TBOARD=vm SUBDIR=python distclean ext
```

## How Python <a name="python"></a>

### Setting up PIP <a name="python_pip"></a>


```
bash-5.1# python3 -m ensurepip
Looking in links: /tmp/tmp5p1wvqlw
Processing /var/tmp/tmp/tmp5p1wvqlw/setuptools-63.2.0-py3-none-any.whl
Processing /var/tmp/tmp/tmp5p1wvqlw/pip-22.2.2-py3-none-any.whl
Installing collected packages: setuptools, pip
  WARNING: The scripts pip3 and pip3.10 are installed in '//bin' which is not on PATH.
  Consider adding this directory to PATH or, if you prefer to suppress this warning, use --no-warn-script-location.
Successfully installed pip-22.2.2 setuptools-63.2.0
WARNING: Running pip as the 'root' user can result in broken permissions and conflicting behaviour with the system package manager. It is recommended to use a virtual environment instead: https://pip.pypa.io/warnings/venv
bash-5.1# 
```

### Upgrading PIP <a name="python_upgrade_pip"></a>


```
bash-5.1# python3 -m pip install --upgrade pip
WARNING: The directory '/.cache/pip' or its parent directory is not owned or is not writable by the current user. The cache has been disabled. Check the permissions and owner of that directory. If executing pip with sudo, you should use sudo'.
Requirement already satisfied: pip in /lib/python3.10/site-packages (22.2.2)
Collecting pip
  Downloading pip-22.3-py3-none-any.whl (2.1 MB)
     ???????????????????????????????????????? 2.1/2.1 MB 646.9 kB/s eta 0:00:00
Installing collected packages: pip
  Attempting uninstall: pip
    Found existing installation: pip 22.2.2
    Uninstalling pip-22.2.2:
      Successfully uninstalled pip-22.2.2
  WARNING: The scripts pip, pip3 and pip3.10 are installed in '//bin' which is not on PATH.
  Consider adding this directory to PATH or, if you prefer to suppress this warning, use --no-warn-script-location.
Successfully installed pip-22.3
WARNING: Running pip as the 'root' user can result in broken permissions and conflicting behaviour with the system package manager. It is recommended to use a virtual environment instead: https://pip.pypa.io/warnings/venv
bash-5.1# 
```


