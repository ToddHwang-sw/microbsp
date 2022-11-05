

Why not using Yocto  
------------


* **Yocto** must be the strongest player in embedded Linux BSP(Board Support Package) tracks. In programmer's viewpoint, I think the followings are amazing characteristics of the Yocto.   
  - The greatest attraction of the Yocto is fully automatic build dependency setup. - Just user simply specify dependency with a keyword "DEPENDS", and the Yocto constructs full dependency tree and tries to build from the bottom nodes of the tree. 
  - QEMU basis cross compilation strategy provides a total environmental separation from host machine, which will not be interfered/conflicted with host machine environment.
  - No occurrence of root access authentication during compilation. - None of CLI commands beginning with "sudo" .
  - Packaging is also supported in various forms; rpm, ipk, .. 
 
* Many detail operations of Yocto are totally implemeted/abstracted from "Python" classes, and it is not easy to fully digest how they work. 

* Final usages of BSP sources for embedded system are usually limited to 2 or 3 such as cross compiler generation, building applications, required utilities and the creation of final image to be downloaded into embedded board. 


![](doc/MyfileTest.png)

MicroBSP
------------

* I want to set up simple Linux basis embedded board BSP for both **Raspberry PI 3** and **QEMU VM** - Yocto is too much big cat to catch up these mices only for my private use. 

* **Raspberry PI** is the cheapest embedded board I can purchase easily from Amazon in less than $40, and **QEMU VM** can be easily activated in Ubuntu Linux installed PC. -  For poor system programmer like me, Raspberry PI becomes the right answer.

* I want the BSP to keep folder/source structure to be easily hacked/manipulated. 

* I don't want fancy desktop environment since I have no extra mouse and keyboard to use with the raspberry pi. HDMI monitor for the PI ? Oh.. no its more than $100 on Amazon. Just simple UART connection with the embedded board will be good for me. 

* **MicroBSP** can be used for 
	- Quick verification of GNU applications in embedded environment 
	- Fast evaluation of embedded application 
	- Fast evaluation of Linux kernel features 

* MicroBSP is to assist the needs for my private individual reason and public share purposes based on its inherent flexibility and simplicity.

Summary
------------


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
* CPU core and board is not separated in MicroBSP. - Even board A and board B are using the same core type "Cortex-A53", but we need to use 2 arch layers for both boards.  

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

* Linux kernels used for both raspberry pi and QEMU VM are as follows. 

|  Type            | Linux kernel version |
|------------------|----------------------|
| Raspberry PI     |  5.10.x              |
| QEMU VM          |  5.17.7              |

* MicroBSP has **Overlay File System** basis booting policy. 
* Total booting disk image has the following hierarchy. 


<span style="color:blue; font-size:4em">Setting up Prerequisite Utilities/Libraries</span>
==========================

- Fundamental libraries/applications will be installed by the following command.
- <strong>This can work in Ubuntu. </strong>

```#!/bin/sh

  # make installcomps
```

<span style="color:blue; font-size:4em">Raspberry PI</span>
===============

Testbed Components
---------

* [Raspberry PI 3+](https://www.amazon.com/CanaKit-Raspberry-Complete-Starter-Kit/dp/B01C6Q2GSY/ref=sr_1_6?keywords=raspberry+pi+3%2B&qid=1636520388&qsid=132-7915930-0137541&sr=8-6&sres=B07BCC8PK7%2CB01C6EQNNK%2CB07KKBCXLY%2CB01C6Q2GSY%2CB01LPLPBS8%2CB07BDR5PDW%2CB07BC567TW%2CB07BFH96M3%2CB07N38B86S%2CB0778CZ97B%2CB09HGXHRLQ%2CB08C254F73%2CB08G8QYFCD%2CB01N13X8V1%2CB09HH2BFN4%2CB06W54L7B5)

* [UART Cable for Raspberry PI](https://www.amazon.com/FEANTEEK-TTL232R-Raspberry-Serial-Windows/dp/B08HLSS5T4/ref=sr_1_1_sspa?keywords=raspberry+pi+3%2B+UART+cable&qid=1636520578&sr=8-1-spons&psc=1&spLa=ZW5jcnlwdGVkUXVhbGlmaWVyPUExTEFJTFcwNlNKT0lIJmVuY3J5cHRlZElkPUEwMjc5MjEwTjlUUDBEWEtNTzJTJmVuY3J5cHRlZEFkSWQ9QTA3NDE2NTBMV0xWUDdaRlZFV0gmd2lkZ2V0TmFtZT1zcF9hdGYmYWN0aW9uPWNsaWNrUmVkaXJlY3QmZG9Ob3RMb2dDbGljaz10cnVl )

* [TP-Link USB WiFi Dongle](https://www.amazon.com/TP-Link-TL-WN823N-Wireless-network-Raspberry/dp/B0088TKTY2/ref=sr_1_15?keywords=tp+link+usb+wifi+adapter&qid=1636520657&qsid=132-7915930-0137541&sr=8-15&sres=B08D72GSMS%2CB008IFXQFU%2CB07P6N2TZH%2CB08KHV7H1S%2CB07PB1X4CN%2CB07P5PRK7J%2CB00JBJ6VG8%2CB00YUU3KC6%2CB002SZEOLG%2CB00K11UIV4%2CB0088TKTY2%2CB01MR6M8EC%2CB00HC01KMS%2CB0799C35LV%2CB01NBMJGA9%2CB00A8GVNNY&srpt=NETWORK_INTERFACE_CONTROLLER_ADAPTER)

Operation Setup
---------

* Home Gateway
* LAN
  - Ethernet : Built-in ethernet
  - WLAN1 : Raspberry PI Built-in WLAN (<strong>Broadcom BRCM43455</strong>)
* WAN
  - WLAN0 : TP-Link USB WiFi Dongle (<strong>Realtek</strong>)
* UART baudrate = 921600bps (not 115200bps)

Booting Shot
-----------

  ```sh

[    0.000000] Linux version 5.10.110-v8 (todd@vostro) (aarch64-any-linux-gnu-gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.38) #1 SMP PREEMPT Wed Oct 12 21:16:58 PDT 2022
[    0.000000] random: fast init done
[    0.000000] Machine model: Raspberry Pi 3 Model B Plus Rev 1.3
[    0.000000] efi: UEFI not found.
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
[    0.000000] percpu: Embedded 31 pages/cpu s87384 r8192 d31400 u126976
[    0.000000] Detected VIPT I-cache on CPU0
[    0.000000] CPU features: detected: ARM erratum 845719
[    0.000000] CPU features: kernel page table isolation forced ON by KASLR
[    0.000000] CPU features: detected: Kernel page table isolation (KPTI)
[    0.000000] CPU features: detected: ARM erratum 843419
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 238896
[    0.000000] Kernel command line: coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.enable_compat_alsa=0 snd_bcm2835.enable_hdmi=1 bcm2708_fb.fbwidth=640 bcm2708_fb.fbheight=480 bcm2708_fb.fe
[    0.000000] Kernel parameter elevator= does not have any effect anymore.
[    0.000000] Please use sysfs to set IO scheduler for individual devices.
[    0.000000] Dentry cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
[    0.000000] Inode-cache hash table entries: 65536 (order: 7, 524288 bytes, linear)
[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
[    0.000000] Memory: 860712K/970752K available (12288K kernel code, 2004K rwdata, 4440K rodata, 3904K init, 1237K bss, 44504K reserved, 65536K cma-reserved)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
[    0.000000] ftrace: allocating 36784 entries in 144 pages
[    0.000000] ftrace: allocated 144 pages with 2 groups
[    0.000000] rcu: Preemptible hierarchical RCU implementation.
[    0.000000] rcu:     RCU event tracing is enabled.
[    0.000000]  Trampoline variant of Tasks RCU enabled.
[    0.000000]  Rude variant of Tasks RCU enabled.
[    0.000000]  Tracing variant of Tasks RCU enabled.
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 100 jiffies.
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
[    0.000000] random: get_random_bytes called from start_kernel+0x3b4/0x57c with crng_init=1
[    0.000000] arch_timer: cp15 timer(s) running at 19.20MHz (phys).
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0x46d987e47, max_idle_ns: 440795202767 ns
[    0.000007] sched_clock: 56 bits at 19MHz, resolution 52ns, wraps every 4398046511078ns
[    0.000300] Console: colour dummy device 80x25
[    0.000374] Calibrating delay loop (skipped), value calculated using timer frequency.. 38.40 BogoMIPS (lpj=19200)
[    0.000415] pid_max: default: 32768 minimum: 301
[    0.000626] LSM: Security Framework initializing
[    0.000933] Mount-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
[    0.000976] Mountpoint-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
[    0.002579] cgroup: Disabling memory control group subsystem
[    0.005866] rcu: Hierarchical SRCU implementation.
[    0.007151] EFI services will not be available.
[    0.007888] smp: Bringing up secondary CPUs ...
[    0.009325] Detected VIPT I-cache on CPU1
[    0.009410] CPU1: Booted secondary processor 0x0000000001 [0x410fd034]
[    0.011072] Detected VIPT I-cache on CPU2
[    0.011129] CPU2: Booted secondary processor 0x0000000002 [0x410fd034]
[    0.012707] Detected VIPT I-cache on CPU3
[    0.012761] CPU3: Booted secondary processor 0x0000000003 [0x410fd034]
[    0.012978] smp: Brought up 1 node, 4 CPUs
[    0.013043] SMP: Total of 4 processors activated.
[    0.013066] CPU features: detected: 32-bit EL0 Support
[    0.013089] CPU features: detected: CRC32 instructions
[    0.050589] CPU: All CPU(s) started at EL2
[    0.050691] alternatives: patching kernel code
[    0.052715] devtmpfs: initialized
[    0.074228] Enabled cp15_barrier support
[    0.074295] Enabled setend support
[    0.074340] KASLR enabled
[    0.074674] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 1911260446275000 ns
[    0.074717] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
[    0.078257] pinctrl core: initialized pinctrl subsystem
[    0.079693] DMI not present or invalid.
[    0.080210] NET: Registered protocol family 16
[    0.091034] DMA: preallocated 1024 KiB GFP_KERNEL pool for atomic allocations
[    0.091543] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA pool for atomic allocations
[    0.093189] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA32 pool for atomic allocations
[    0.093431] audit: initializing netlink subsys (disabled)
[    0.093915] audit: type=2000 audit(0.092:1): state=initialized audit_enabled=0 res=1
[    0.094878] thermal_sys: Registered thermal governor 'step_wise'
[    0.095223] cpuidle: using governor menu
[    0.095520] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers.
[    0.095787] ASID allocator initialised with 32768 entries
[    0.096035] Serial: AMBA PL011 UART driver
[    0.123899] bcm2835-mbox 3f00b880.mailbox: mailbox enabled
[    0.127340] raspberrypi-firmware soc:firmware: Attached to firmware from 2022-08-26T14:04:36, variant start
[    0.128347] raspberrypi-firmware soc:firmware: Firmware hash is 102f1e848393c2112206fadffaaf86db04e98326
[    2.351857] cryptd: max_cpu_qlen set to 1000
[    2.354375] "cryptomgr_test" (71) uses obsolete ecb(arc4) skcipher
[    2.447719] DRBG: Continuing without Jitter RNG
[    2.475512] bcm2835-dma 3f007000.dma: DMA legacy API manager, dmachans=0x1
[    2.478746] SCSI subsystem initialized
[    2.479190] usbcore: registered new interface driver usbfs
[    2.479290] usbcore: registered new interface driver hub
[    2.479403] usbcore: registered new device driver usb
[    2.482014] clocksource: Switched to clocksource arch_sys_counter
[    4.276842] VFS: Disk quotas dquot_6.6.0
[    4.276977] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    4.277236] FS-Cache: Loaded
[    4.277601] CacheFiles: Loaded
[    4.293983] NET: Registered protocol family 2
[    4.294361] IP idents hash table entries: 16384 (order: 5, 131072 bytes, linear)
[    4.296058] tcp_listen_portaddr_hash hash table entries: 512 (order: 1, 8192 bytes, linear)
[    4.296114] TCP established hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    4.296371] TCP bind hash table entries: 8192 (order: 5, 131072 bytes, linear)
[    4.296578] TCP: Hash tables configured (established 8192 bind 8192)
[    4.296877] UDP hash table entries: 512 (order: 2, 16384 bytes, linear)
[    4.296945] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes, linear)
[    4.297316] NET: Registered protocol family 1
[    4.298512] RPC: Registered named UNIX socket transport module.
[    4.298536] RPC: Registered udp transport module.
[    4.298556] RPC: Registered tcp transport module.
[    4.298576] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    4.302165] hw perfevents: enabled with armv8_cortex_a53 PMU driver, 7 counters available
[    4.304814] Initialise system trusted keyrings
[    4.305346] workingset: timestamp_bits=46 max_order=18 bucket_order=0
[    4.315428] zbud: loaded
[    4.317843] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    4.318268] FS-Cache: Netfs 'nfs' registered for caching
[    4.319238] NFS: Registering the id_resolver key type
[    4.319301] Key type id_resolver registered
[    4.319323] Key type id_legacy registered
[    4.319499] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
[    4.319524] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering...
[    4.319864] fuse: init (API version 7.32)
[    4.391877] NET: Registered protocol family 38
[    4.391912] Key type asymmetric registered
[    4.391935] Asymmetric key parser 'x509' registered
[    4.392074] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 249)
[    4.392099] io scheduler mq-deadline registered
[    4.392121] io scheduler kyber registered
[    4.469852] xz_dec_test: module loaded
[    4.469877] xz_dec_test: Create a device node with 'mknod xz_dec_test c 248 0' and write .xz files to it.
[    4.475219] bcm2708_fb soc:fb: FB found 1 display(s)
[    4.495615] Console: switching to colour frame buffer device 80x30
[    4.500982] bcm2708_fb soc:fb: Registered framebuffer for display 0, size 640x480
[    4.507396] Serial: 8250/16550 driver, 1 ports, IRQ sharing enabled
[    4.508718] bcm2835-aux-uart 3f215040.serial: there is not valid maps for state default
[    4.511484] bcm2835-rng 3f104000.rng: hwrng registered
[    4.512166] vc-mem: phys_addr:0x00000000 mem_base=0x3ec00000 mem_size:0x40000000(1024 MiB)
[    4.514127] gpiomem-bcm2835 3f200000.gpiomem: Initialised: Registers at 0x3f200000
[    4.533863] brd: module loaded
[    4.552715] loop: module loaded
[    4.555507] Loading iSCSI transport class v2.0-870.
[    4.558525] usbcore: registered new interface driver lan78xx
[    4.558640] usbcore: registered new interface driver smsc95xx
[    4.558676] dwc_otg: version 3.00a 10-AUG-2012 (platform bus)
[    5.287342] Core Release: 2.80a
[    5.287369] Setting default values for core params
[    5.287409] Finished setting default values for core params
[    5.487881] Using Buffer DMA mode
[    5.487904] Periodic Transfer Interrupt Enhancement - disabled
[    5.487924] Multiprocessor Interrupt Enhancement - disabled
[    5.487946] OTG VER PARAM: 0, OTG VER FLAG: 0
[    5.487972] Dedicated Tx FIFOs mode
[    5.489036] 
[    5.489060] WARN::dwc_otg_hcd_init:1072: FIQ DMA bounce buffers: virt = ffffffc0106b4000 dma = 0x00000000f7810000 len=9024
[    5.489107] FIQ FSM acceleration enabled for :
[    5.489107] Non-periodic Split Transactions
[    5.489107] Periodic Split Transactions
[    5.489107] High-Speed Isochronous Endpoints
[    5.489107] Interrupt/Control Split Transaction hack enabled
[    5.489181] 
[    5.489198] WARN::hcd_init_fiq:496: MPHI regs_base at ffffffc01006d000
[    5.489256] dwc_otg 3f980000.usb: DWC OTG Controller
[    5.489305] dwc_otg 3f980000.usb: new USB bus registered, assigned bus number 1
[    5.489373] dwc_otg 3f980000.usb: irq 74, io mem 0x00000000
[    5.489436] Init: Port Power? op_state=1
[    5.489456] Init: Power Port (0)
[    5.489963] usb usb1: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 5.10
[    5.490014] usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1
[    5.490039] usb usb1: Product: DWC OTG Controller
[    5.490062] usb usb1: Manufacturer: Linux 5.10.110-v8 dwc_otg_hcd
[    5.490085] usb usb1: SerialNumber: 3f980000.usb
[    5.491243] hub 1-0:1.0: USB hub found
[    5.491336] hub 1-0:1.0: 1 port detected
[    5.494500] usbcore: registered new interface driver usb-storage
[    5.496509] bcm2835-wdt bcm2835-wdt: Broadcom BCM2835 watchdog timer
[    5.500745] sdhci: Secure Digital Host Controller Interface driver
[    5.500767] sdhci: Copyright(c) Pierre Ossman
[    5.501677] mmc-bcm2835 3f300000.mmcnr: could not get clk, deferring probe
[    5.502717] sdhost-bcm2835 3f202000.mmc: could not get clk, deferring probe
[    5.503121] sdhci-pltfm: SDHCI platform and OF driver helper
[    5.505556] ledtrig-cpu: registered to indicate activity on CPUs
[    5.506178] hid: raw HID events driver (C) Jiri Kosina
[    5.506440] usbcore: registered new interface driver usbhid
[    5.506461] usbhid: USB HID core driver
[    5.506920] ashmem: initialized
[    5.514149] ipip: IPv4 and MPLS over IPv4 tunneling driver
[    5.515015] gre: GRE over IPv4 demultiplexor driver
[    5.515067] Initializing XFRM netlink socket
[    5.515975] NET: Registered protocol family 10
[    5.518262] Segment Routing with IPv6
[    5.519512] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
[    5.521272] ip6_gre: GRE over IPv6 tunneling driver
[    5.522190] NET: Registered protocol family 17
[    5.522255] NET: Registered protocol family 15
[    5.522331] bridge: filtering via arp/ip/ip6tables is no longer available by default. Update your scripts to load br_netfilter if you need this.
[    5.522466] 8021q: 802.1Q VLAN Support v1.8
[    5.522558] Key type dns_resolver registered
[    5.523375] registered taskstats version 1
[    5.523428] Loading compiled-in X.509 certificates
[    5.524223] Key type ._fscrypt registered
[    5.524248] Key type .fscrypt registered
[    5.524269] Key type fscrypt-provisioning registered
[    5.544257] uart-pl011 3f201000.serial: cts_event_workaround enabled
[    5.544393] 3f201000.serial: ttyAMA0 at MMIO 0x3f201000 (irq = 99, base_baud = 0) is a PL011 rev2
[    5.547804] bcm2835-aux-uart 3f215040.serial: there is not valid maps for state default
[    5.548748] printk: console [ttyS0] disabled
[    5.549054] 3f215040.serial: ttyS0 at MMIO 0x3f215040 (irq = 71, base_baud = 31250000) is a 16550
[    5.712011] printk: console [ttyS0] enabled
[    5.714379] bcm2835-power bcm2835-power: Broadcom BCM2835 power domains driver
[    5.718095] mmc-bcm2835 3f300000.mmcnr: mmc_debug:0 mmc_debug2:0
[    5.718906] mmc-bcm2835 3f300000.mmcnr: DMA channel allocated
[    5.721775] Indeed it is in host mode hprt0 = 00021501
[    5.745843] sdhost: log_buf @ (____ptrval____) (c3175000)
[    5.816163] mmc0: sdhost-bcm2835 loaded - DMA enabled (>1)
[    5.822248] of_cfs_init
[    5.822806] of_cfs_init: OK
[    5.823529] cfg80211: Loading compiled-in X.509 certificates for regulatory database
[    5.845552] mmc1: queuing unknown CIS tuple 0x80 (2 bytes)
[    5.848121] mmc1: queuing unknown CIS tuple 0x80 (3 bytes)
[    5.850614] mmc1: queuing unknown CIS tuple 0x80 (3 bytes)
[    5.854552] mmc1: queuing unknown CIS tuple 0x80 (7 bytes)
[    5.879667] mmc0: host does not support reading read-only switch, assuming write-enable
[    5.884519] mmc0: new high speed SDXC card at address 0001
[    5.887285] mmcblk0: mmc0:0001 GC2QT 59.6 GiB
[    5.895039]  mmcblk0: p1 p2 p3 p4 < p5 p6 p7 >
[    5.918063] usb 1-1: new high-speed USB device number 2 using dwc_otg
[    5.919181] Indeed it is in host mode hprt0 = 00001101
[    5.942286] mmc1: new high speed SDIO card at address 0001
[    6.101202] cfg80211: Loaded X.509 cert 'sforshee: 00b28ddf47aef9cea7'
[    6.102790] platform regulatory.0: Direct firmware load for regulatory.db failed with error -2
[    6.104040] cfg80211: failed to load regulatory.db
[    6.107583] usb 1-1: New USB device found, idVendor=0424, idProduct=2514, bcdDevice= b.b3
[    6.108948] usb 1-1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    6.111198] hub 1-1:1.0: USB hub found
[    6.111897] hub 1-1:1.0: 4 ports detected
[    6.116151] VFS: Mounted root (squashfs filesystem) readonly on device 179:2.
[    6.121472] devtmpfs: mounted
[    6.136693] Freeing unused kernel memory: 3904K
[    6.137504] Run /sbin/init as init process
[    6.401120] usb 1-1.1: new high-speed USB device number 3 using dwc_otg
[    6.490890] usb 1-1.1: New USB device found, idVendor=0424, idProduct=2514, bcdDevice= b.b3
[    6.492062] usb 1-1.1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    6.494392] hub 1-1.1:1.0: USB hub found
[    6.495114] hub 1-1.1:1.0: 3 ports detected
[    6.574120] usb 1-1.3: new high-speed USB device number 4 using dwc_otg
[    6.669628] usb 1-1.3: New USB device found, idVendor=2357, idProduct=0109, bcdDevice= 2.00
[    6.670792] usb 1-1.3: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[    6.671810] usb 1-1.3: Product: 802.11n NIC 
[    6.672419] usb 1-1.3: Manufacturer: Realtek 
[    6.673037] usb 1-1.3: SerialNumber: 00e04c000001
[    6.827249] EXT4-fs (mmcblk0p3): recovery complete
[    6.830492] EXT4-fs (mmcblk0p3): mounted filesystem with ordered data mode. Opts: (null)
[    6.879456] EXT4-fs (mmcblk0p5): recovery complete
[    6.886415] EXT4-fs (mmcblk0p5): mounted filesystem with ordered data mode. Opts: (null)
[    6.910243] EXT4-fs (mmcblk0p7): recovery complete
[    6.910945] EXT4-fs (mmcblk0p7): mounted filesystem with ordered data mode. Opts: (null)
[    6.912374] random: crng init done
Third lowerdisk is not mounted !!
Mounting...
[    6.935725] overlayfs: "xino" feature enabled using 32 upper inode bits.
Device file system
Change root !!
[    7.297116] usb 1-1.1.1: new high-speed USB device number 5 using dwc_otg
[    7.386932] usb 1-1.1.1: New USB device found, idVendor=0424, idProduct=7800, bcdDevice= 3.00
[    7.388127] usb 1-1.1.1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    7.657455] lan78xx 1-1.1.1:1.0 (unnamed net_device) (uninitialized): No External EEPROM. Setting MAC Speed
[    7.682484] lan78xx 1-1.1.1:1.0 (unnamed net_device) (uninitialized): int urb period 64
chpasswd: password for 'root' changed
Thu Aug  1 12:00:00 UTC 2019


WORK PARTITION ...


[    7.797762] EXT4-fs (mmcblk0p6): recovery complete
[    7.798615] EXT4-fs (mmcblk0p6): mounted filesystem with ordered data mode. Opts: (null)


GIT ... 




PYTHON ... 




 QT Runtime environment




INIT FILE STARTS ...


Syslog Daemon
TELNET daemon
BOOT Partition
[    8.581659] FAT-fs (mmcblk0p1): Volume was not properly unmounted. Some data may be corrupt. Please run fsck.
Python !!
640x480x32bpp pitch 2560 type 0 visual 2 colors 16777216 pixtype 8
[WLAN] RTK Loading modules...
[    9.065431] 8192eu: loading out-of-tree module taints kernel.
[    9.299818] usbcore: registered new interface driver rtl8192eu
grep: wlan0: No such file or directory
[WLAN] Building up /var/tmp/wpa_supplicant/1.conf
[WLAN] Running WPA Suppplicant !!
[   11.458135] device wlan0 entered promiscuous mode
tcpdump: listening on wlan0, link-type EN10MB (Ethernet), capture size 262144 bytes
Successfully initialized wpa_supplicant
ioctl[SIOCSIWENCODEEXT]: Invalid argument
ioctl[SIOCSIWENCODEEXT]: Invalid argument
Waiting Connect .. 0
Waiting Connect .. 1
[   13.736481] IPv6: ADDRCONF(NETDEV_CHANGE): wlan0: link becomes ready
Waiting Connect .. 2
IPv4 DHCP ...
Waiting IP .. 0
Waiting IP .. 1
Waiting IP .. 2
Waiting IP .. 3
Waiting IP .. 4
[WLAN] BRCM Loading modules...
[   20.214306] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   20.216592] usbcore: registered new interface driver brcmfmac
[   20.231037] 8021q: adding VLAN 0 to HW filter on device eth0
ps: Unable to get system boot time
[WLAN] Building up /var/tmp/hostapd/1.conf
[   20.443214] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   20.444773] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[WLAN] Running AP Suppplicant !!
[   20.468609] brcmfmac: brcmf_c_preinit_dcmds: Firmware: BCM4345/6 wl0: Feb 27 2018 03:15:32 version 7.45.154 (r684107 CY) FWID 01-4fbe0b04
random: Trying to read entropy from /dev/random
Configuration file: /var/tmp/hostapd/1.conf
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
nl80211: interface wlan1 in phy phy1
nl80211: Set mode ifindex 9 iftype 3 (AP)
nl80211: Setup AP(wlan1) - device_ap_sme=1 use_monitor=0
nl80211: Subscribe to mgmt frames with AP handle 0x1c276860 (device SME)
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=04
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=0501
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=0503
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=0504
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=06
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=08
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=09
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=0a
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=11
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=12
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x1c276860 match=7f
nl80211: Register frame type=0xb0 (WLAN_FC_STYPE_AUTH) nl_handle=0x1c276860 match=
nl80211: Enable Probe Request reporting nl_preq=0x1c27be50
nl80211: Register frame type=0x40 (WLAN_FC_STYPE_PROBE_REQ) nl_handle=0x1c27be50 match=
rfkill: initial event: idx=1 type=1 op=0 soft=0 hard=0
nl80211: Add own interface ifindex 9 (ifidx_reason -1)
nl80211: if_indices[16]: 9(-1)
nl80211: Add own interface ifindex 10 (ifidx_rea[   20.637626] br0: port 1(wlan1) entered blocking state
son 9)
[   20.638439] br0: port 1(wlan1) entered disabled state
nl80211: if_indices[16]: 9(-1) 10(9)
nl[   20.639625] device wlan1 entered promiscuous mode
80211: Adding interface wlan1 into bridge br0
phy: phy1
BSS count 1, BSSID mask 00:00:00:00:00:00 (0 bits)
wlan1: interface state UNINITIALIZED->COUNTRY_UPDATE
Previous country code 00, new country code US 
Continue interface setup after channel list update
ctrl_iface not configured!
[BRIDGE] Bridge creation....
brctl: bridge br0: File exists
[   21.490549] br0: port 2(eth0) entered blocking state
[   21.491348] br0: port 2(eth0) entered disabled state
[   21.493083] device eth0 entered promiscuous mode
brctl: bridge br0: Device or resource busy
Internet Systems Consortium DHCP Server 4.4.2b1
Copyright 2004-2019 Internet Systems Consortium.
All rights reserved.
For info, please visit https://www.isc.org/software/dhcp/
Config file: /var/tmp/hostapd/dhcpd.conf
Database file: /var/tmp/hostapd/dhcpd.leases
PID file: /var/run/hostapd/dhcpd.pid
Wrote 0 leases to leases file.
Listening on LPF/br0/b8:27:eb:aa:c5:b5/172.145.1.0/24
Sending on   LPF/br0/b8:27:eb:aa:c5:b5/172.145.1.0/24
Sending on   Socket/fallback/fallback-net
Server starting service.
[   22.596131] device br0 entered promiscuous mode
tcpdump: listening on br0, link-type EN10MB (Ethernet), capture size 262144 bytes
[IPV6] START .....
ip: either "dev" is duplicate, or "permanent" is garbage
+++++++++++++++++++++++++++++++++++++++++++++++++++++
br0       Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet addr:172.145.1.1  Bcast:172.145.1.255  Mask:255.255.255.0
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

eth0      Link encap:Ethernet  HWaddr B8:27:EB:FF:90:E0  
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

ip6_vti0  Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  
          NOARP  MTU:1332  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

ip6gre0   Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  
          NOARP  MTU:1448  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

ip6tnl0   Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  
          NOARP  MTU:1452  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

sit0      Link encap:IPv6-in-IPv4  
          NOARP  MTU:1480  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

tunl0     Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-E8-9D-00-00-00-00-00-00-00-00  
          NOARP  MTU:1480  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

wlan0     Link encap:Ethernet  HWaddr D4:6E:0E:13:B9:8C  
          inet addr:192.167.0.206  Bcast:192.167.0.255  Mask:255.255.255.0
          inet6 addr: fe80::d66e:eff:fe13:b98c/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:27 errors:0 dropped:0 overruns:0 frame:0
          TX packets:16 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:3738 (3.6 KiB)  TX bytes:2450 (2.3 KiB)

wlan1     Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)



nameserver 127.0.0.1
nameserver 192.167.0.1
nameserver 8.8.8.8
+++++++++++++++++++++++++++++++++++++++++++++++++++++
[DNS] Masquerading start ...
QT initialization
../../../../source/nftables-0.9.5/src/mnl.c:45: Unable to initialize Netlink socket: Protocol not supported
[   25.880432] IPv6: ADDRCONF(NETDEV_CHANGE): wlan1: link becomes ready
[   25.881715] br0: port 1(wlan1) entered blocking state
[   25.882498] br0: port 1(wlan1) entered forwarding state
[   25.884155] IPv6: ADDRCONF(NETDEV_CHANGE): br0: link becomes ready


CONSOLE ...


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
bash-5.1# 
bash-5.1# ifconfig
br0       Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet addr:172.145.1.1  Bcast:172.145.1.255  Mask:255.255.255.0
          inet6 addr: fe80::ba27:ebff:feaa:c5b5/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:7 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:666 (666.0 B)

eth0      Link encap:Ethernet  HWaddr B8:27:EB:FF:90:E0  
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

wlan0     Link encap:Ethernet  HWaddr D4:6E:0E:13:B9:8C  
          inet addr:192.167.0.206  Bcast:192.167.0.255  Mask:255.255.255.0
          inet6 addr: fe80::d66e:eff:fe13:b98c/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:40 errors:0 dropped:0 overruns:0 frame:0
          TX packets:20 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:5665 (5.5 KiB)  TX bytes:2992 (2.9 KiB)

wlan1     Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet6 addr: fe80::ba27:ebff:feaa:c5b5/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:14 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:1588 (1.5 KiB)

bash-5.1# 
bash-5.1# ping www.google.com
PING www.google.com (142.250.188.228): 56 data bytes
64 bytes from 142.250.188.228: seq=0 ttl=58 time=44.311 ms
64 bytes from 142.250.188.228: seq=1 ttl=58 time=4.887 ms
64 bytes from 142.250.188.228: seq=2 ttl=58 time=7.764 ms
64 bytes from 142.250.188.228: seq=3 ttl=58 time=6.463 ms
64 bytes from 142.250.188.228: seq=4 ttl=58 time=6.087 ms
^C
--- www.google.com ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 4.887/13.902/44.311 ms
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 

  ```
## Procedures


# [1] Toolchain Building

- When user want to setup a toolchain under <strong>/opt/rpi3</strong> folder.
```#!/bin/sh

  # sudo make TBOARD=rpi3 TOOLCHAIN_ROOT=/opt/rpi3 toolchain
```
- Without an option "TOOLCHAIN_ROOT=", the toolchain will be installed under <strong>./gnu/toolchain</strong> folder.
```#!/bin/sh

  # make TBOARD=rpi3 toolchain
```

 * "TBOARD" indicates type of board which is the name of folders in boards/ .
 * "TOOLCHAIN_ROOT" indicates the location of cross toolchain.


# [2] Libraries, Applications, Extra Applications Building

```#!/bin/sh

  # make TBOARD=rpi3 lib app ext
```

# [3] Booting Image Building

```#!/bin/sh

  # make TBOARD=rpi3 board ramdisk extdisk
```

- The following 3 files will be created under <strong>./boards/rpi3/</strong> .
  * boot.tgz
  * rootfs.squashfs
  * image.ext4


- <span style="color:red"> <em> Kernel is referenced multiple times both at toochain building and booting image building. When the toolchain has been built with "sudo" command prefix to locate it under super-user priviledge folder (when you use "TOOLCHAIN_ROOT=" option), access conflict error may happen since the booting image building is accomplished under normal user priviledge.  
To prevent this, just simply clean up <strong>board/rpi3/kernel/build</strong> folder as follows, </em> </span>

```#!/bin/sh

  # sudo \rm -rf ./boards/rpi3/kernel/build/*
```

# [4] Creating UI image (Embedded QT - Cross-compilation)

```#!/bin/sh

  # make TBOARD=rpi3 ui uidisk 
```

- It takes fairly long time to build up all subfolders under /uix.
- The final step is building Embedded QT. 

# [5] Formatting SD Card

- Currently,the following 6 partitions should be used. (<strong>/dev/sde4</strong> is actually the top-level container for partitions; /dev/sde[5,6,7].)

```#!/bin/sh

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
|  /dev/sde6 |       X          |
|  /dev/sde7 |       X          |

- /dev/sde5 is mounted into /root . /dev/sde6 and /dev/sde7 are overlayed.
- User can copy images into partitions as follows.

```#!/bin/sh
  # sudo dd if=./board/rpi3/rootfs.squashfs of=/dev/sde2 bs=128M
  # sudo dd if=./board/rpi3/image.ext4      of=/dev/sde3 bs=128M  - It takes long time.

```

- Shell script to make these partitions automatically is available under board/rpi3 as follows.
  If MMC for raspberry PI was mounted on "/dev/sdc",

```#!/bin/sh

  # cd boards/rpi3
  # sudo ./format_sdcard.sh sdc
```

- Rootfs.squashfs , boot.tgz, image.ext4 are all copied into the disk.
- CAUTION) Existing partitions on the SDcard will be deleted upon running "format_sdcard.sh" .

# [5] WLAN Configuration

- The following is a file <strong>boards/rpi3/firmware/conf.sys</strong> for configuration file.
- Simply change values of <strong>STASSID</strong> and <strong>STAPASSWORD</strong> attributes as defined in your WLAN environment.
By default, <em>LOOSERS</em> and <em>12345678</em> are used.

```#!/bin/sh

##
## WiFi AP
##
USEAP=1
BRCAPTURE=1
BRINTERFACE=br0
ETHINTERFACE=eth0
APINTERFACE=wlan1
APSSID=RPI3
APPASSWORD=12345678

# Only C class
APNET=172.145.1
APNODE=1
APMASK=255.255.255.0
DOMAIN=todd.rpi3.com
DHCPSTART=100
DHCPEND=200

##
## IPv6 RADVD
##
IPV6_PREFIX=2602:9900:1234:6e4c::
IPV6_ID=2000
USE_NAT6=1

##
## WiFi STA
##
USESTA=1
STACAPTURE=1
STAINTERFACE=wlan0
STASSID=LOOSERS
STAPASSWORD=12345678

##
## TL-WN725N : 8188eu (150Mbps)
## TL-WN825N : 8192eu (300Mbps)
##
RTKDRV=8192eu

```

- <strong>APSSID</strong> and <strong>APPASSWORD</strong> are SSID and password used for internal LAN adaptation.
- <strong>APNET></strong> and <strong>APMASK</strong> is IP address ranges distributed to attached stations.


# [6] Available Packages 

- The following command prints the list of available packages from Microbsp. 
- <strong>MIBC_DEPENDS</strong> section inside of Makefile can choose any of those required . 


```#!/bin/sh


todd@vostro:/media/todd/work/github/microbsp$ make TBOARD=rpi3 pkglist

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

```#!/bin/sh

  MICB_DEPENDS = openssl zlib 

```

<span style="color:blue; font-size:4em">Ubuntu QEMU</span>
===============

Testbed Components
---------

* [QEMU VM  Emulator](https://www.unixmen.com/how-to-install-and-configure-qemu-in-ubuntu/)
* USB Storage stick with "image.ext4"

```#!/bin/sh

  # cd boards/vm

  # sudo dmesg | grep "sd "
    ..
    [33889.430026] sd 7:0:0:0: [sdd] 30375936 512-byte logical blocks: (15.6 GB/14.5 GiB)
    [33889.431021] sd 7:0:0:0: [sdd] Write Protect is off
    [33889.431027] sd 7:0:0:0: [sdd] Mode Sense: 43 00 00 00
    [33889.432009] sd 7:0:0:0: [sdd] Write cache: disabled, read cache: enabled, doesn't support DPO or FUA
    [33889.458386] sd 7:0:0:0: [sdd] Attached SCSI removable disk
 
    We can know that USB disk is mounted as "sdd"

  # dd if=image.ext4 of=/dev/sdd bs=32M 
 
    <It might take a long time to flash 6G image into USB stick. >

```



Operation Setup
---------

* Just bash shell

Booting Shot
-----------

  ```sh

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

[    0.000000] Linux version 5.17.7 (todd@vostro) (x86_64-any-linux-gnu-gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.38) #33 SMP PREEMPT Fri Oct 14 21:19:57 PDT 2022
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
[    0.000000] RAMDISK: [mem 0xffffffffb4f48000-0xffffffffbddf2fff]
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
[    0.000000] Memory: 3883788K/4193784K available (14345K kernel code, 3196K rwdata, 4560K rodata, 2068K init, 4148K bss, 309736K reserved, 0K cma-reserved)
[    0.000000] ftrace: allocating 40912 entries in 160 pages
[    0.000000] ftrace: allocated 160 pages with 2 groups
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
[    0.016000] ..TIMER: vector=0x30 apic1=0 pin1=2 apic2=-1 pin2=-1
[    0.048000] tsc: Unable to calibrate against PIT
[    0.052000] tsc: using HPET reference calibration
[    0.052000] tsc: Detected 2655.845 MHz processor
[    0.000962] tsc: Marking TSC unstable due to TSCs unsynchronized
[    0.002464] Calibrating delay loop (skipped), value calculated using timer frequency.. 5311.69 BogoMIPS (lpj=10623380)
[    0.007622] pid_max: default: 32768 minimum: 301
[    0.012250] random: get_random_bytes called from setup_net+0x4d/0x290 with crng_init=0
[    0.018904] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.019669] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.049181] process: using AMD E400 aware idle routine
[    0.050069] Last level iTLB entries: 4KB 512, 2MB 255, 4MB 127
[    0.050554] Last level dTLB entries: 4KB 512, 2MB 255, 4MB 127, 1GB 0
[    0.051688] Spectre V1 : Mitigation: usercopy/swapgs barriers and __user pointer sanitization
[    0.052718] Spectre V2 : Mitigation: Retpolines
[    0.053099] Spectre V2 : Spectre v2 / SpectreRSB mitigation: Filling RSB on context switch
[    0.077145] Freeing SMP alternatives memory: 36K
[    0.209898] smpboot: CPU0: AMD QEMU Virtual CPU version 2.5+ (family: 0xf, model: 0x6b, stepping: 0x1)
[    0.221003] cblist_init_generic: Setting adjustable number of callback queues.
[    0.222080] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.223345] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.224666] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.226141] Performance Events: PMU not available due to virtualization, using software events only.
[    0.230328] rcu: Hierarchical SRCU implementation.
[    0.244653] NMI watchdog: Perf NMI watchdog permanently disabled
[    0.252285] smp: Bringing up secondary CPUs ...
[    0.266492] x86: Booting SMP configuration:
[    0.267019] .... node  #0, CPUs:      #1 #2 #3
[    0.000000] calibrate_delay_direct() dropping min bogoMips estimate 2 = 8257500
[    0.570188] smp: Brought up 1 node, 4 CPUs
[    0.570718] smpboot: Max logical packages: 1
[    0.571712] smpboot: Total of 4 processors activated (21388.95 BogoMIPS)
[    0.612553] devtmpfs: initialized
[    0.620141] x86/mm: Memory block size: 128MB
[    0.643635] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    0.644762] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
[    0.648111] pinctrl core: initialized pinctrl subsystem
[    0.666525] NET: Registered PF_NETLINK/PF_ROUTE protocol family
[    0.675453] audit: initializing netlink subsys (disabled)
[    0.679335] audit: type=2000 audit(1665807946.728:1): state=initialized audit_enabled=0 res=1
[    0.686566] thermal_sys: Registered thermal governor 'step_wise'
[    0.686671] thermal_sys: Registered thermal governor 'user_space'
[    0.687947] cpuidle: using governor menu
[    0.695096] PCI: Using configuration type 1 for base access
[    0.704249] mtrr: your CPUs had inconsistent fixed MTRR settings
[    0.704742] mtrr: your CPUs had inconsistent variable MTRR settings
[    0.705318] mtrr: your CPUs had inconsistent MTRRdefType settings
[    0.705759] mtrr: probably your BIOS does not setup all CPUs.
[    0.706592] mtrr: corrected configuration.
[    0.776028] kprobes: kprobe jump-optimization is enabled. All kprobes are optimized if possible.
[    0.786168] HugeTLB registered 2.00 MiB page size, pre-allocated 0 pages
[    0.796363] cryptd: max_cpu_qlen set to 1000
[    0.807580] ACPI: Added _OSI(Module Device)
[    0.808302] ACPI: Added _OSI(Processor Device)
[    0.808675] ACPI: Added _OSI(3.0 _SCP Extensions)
[    0.809061] ACPI: Added _OSI(Processor Aggregator Device)
[    0.810321] ACPI: Added _OSI(Linux-Dell-Video)
[    0.811028] ACPI: Added _OSI(Linux-Lenovo-NV-HDMI-Audio)
[    0.811675] ACPI: Added _OSI(Linux-HPI-Hybrid-Graphics)
[    0.866177] ACPI: 1 ACPI AML tables successfully acquired and loaded
[    0.906242] ACPI: Interpreter enabled
[    0.908241] ACPI: PM: (supports S0 S5)
[    0.908760] ACPI: Using IOAPIC for interrupt routing
[    0.910841] PCI: Using host bridge windows from ACPI; if necessary, use "pci=nocrs" and report a bug
[    0.915921] ACPI: Enabled 2 GPEs in block 00 to 0F
[    1.030823] ACPI: PCI Root Bridge [PCI0] (domain 0000 [bus 00-ff])
[    1.032562] acpi PNP0A03:00: _OSC: OS supports [ASPM ClockPM Segments MSI HPX-Type3]
[    1.033367] acpi PNP0A03:00: _OSC: not requesting OS control; OS requires [ExtendedConfig ASPM ClockPM MSI]
[    1.035356] acpi PNP0A03:00: fail to add MMCONFIG information, can't access extended PCI configuration space under this bridge.
[    1.042363] PCI host bridge to bus 0000:00
[    1.043315] pci_bus 0000:00: root bus resource [io  0x0000-0x0cf7 window]
[    1.044249] pci_bus 0000:00: root bus resource [io  0x0d00-0xffff window]
[    1.044799] pci_bus 0000:00: root bus resource [mem 0x000a0000-0x000bffff window]
[    1.045821] pci_bus 0000:00: root bus resource [mem 0xc0000000-0xfebfffff window]
[    1.046623] pci_bus 0000:00: root bus resource [mem 0x140000000-0x1bfffffff window]
[    1.047677] pci_bus 0000:00: root bus resource [bus 00-ff]
[    1.050740] pci 0000:00:00.0: [8086:1237] type 00 class 0x060000
[    1.079952] pci 0000:00:01.0: [8086:7000] type 00 class 0x060100
[    1.082467] pci 0000:00:01.1: [8086:7010] type 00 class 0x010180
[    1.090987] pci 0000:00:01.1: reg 0x20: [io  0xc040-0xc04f]
[    1.095127] pci 0000:00:01.1: legacy IDE quirk: reg 0x10: [io  0x01f0-0x01f7]
[    1.095879] pci 0000:00:01.1: legacy IDE quirk: reg 0x14: [io  0x03f6]
[    1.098174] pci 0000:00:01.1: legacy IDE quirk: reg 0x18: [io  0x0170-0x0177]
[    1.098831] pci 0000:00:01.1: legacy IDE quirk: reg 0x1c: [io  0x0376]
[    1.101592] pci 0000:00:01.3: [8086:7113] type 00 class 0x068000
[    1.103131] pci 0000:00:01.3: quirk: [io  0x0600-0x063f] claimed by PIIX4 ACPI
[    1.103859] pci 0000:00:01.3: quirk: [io  0x0700-0x070f] claimed by PIIX4 SMB
[    1.106419] pci 0000:00:02.0: [1234:1111] type 00 class 0x030000
[    1.108949] pci 0000:00:02.0: reg 0x10: [mem 0xfd000000-0xfdffffff pref]
[    1.109748] pci 0000:00:02.0: reg 0x18: [mem 0xfebb0000-0xfebb0fff]
[    1.122819] pci 0000:00:02.0: reg 0x30: [mem 0xfeba0000-0xfebaffff pref]
[    1.124194] pci 0000:00:02.0: Video device with shadowed ROM at [mem 0x000c0000-0x000dffff]
[    1.146047] pci 0000:00:03.0: [8086:100e] type 00 class 0x020000
[    1.148774] pci 0000:00:03.0: reg 0x10: [mem 0xfeb80000-0xfeb9ffff]
[    1.150962] pci 0000:00:03.0: reg 0x14: [io  0xc000-0xc03f]
[    1.162072] pci 0000:00:03.0: reg 0x30: [mem 0xfeb00000-0xfeb7ffff pref]
[    1.199723] ACPI: PCI: Interrupt link LNKA configured for IRQ 10
[    1.203087] ACPI: PCI: Interrupt link LNKB configured for IRQ 10
[    1.205160] ACPI: PCI: Interrupt link LNKC configured for IRQ 11
[    1.207569] ACPI: PCI: Interrupt link LNKD configured for IRQ 11
[    1.208996] ACPI: PCI: Interrupt link LNKS configured for IRQ 9
[    1.223916] SCSI subsystem initialized
[    1.233081] ACPI: bus type USB registered
[    1.235525] usbcore: registered new interface driver usbfs
[    1.236687] usbcore: registered new interface driver hub
[    1.237459] usbcore: registered new device driver usb
[    1.239395] pps_core: LinuxPPS API ver. 1 registered
[    1.239866] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    1.241168] PTP clock support registered
[    1.266232] PCI: Using ACPI for IRQ routing
[    1.269439] hpet: 3 channels of 0 reserved for per-cpu timers
[    1.272800] clocksource: Switched to clocksource hpet
[    1.671026] VFS: Disk quotas dquot_6.6.0
[    1.671916] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    1.676193] FS-Cache: Loaded
[    1.685890] CacheFiles: Loaded
[    1.687575] pnp: PnP ACPI init
[    1.705332] pnp: PnP ACPI: found 6 devices
[    1.815455] clocksource: acpi_pm: mask: 0xffffff max_cycles: 0xffffff, max_idle_ns: 2085701024 ns
[    1.817890] NET: Registered PF_INET protocol family
[    1.822592] IP idents hash table entries: 65536 (order: 7, 524288 bytes, linear)
[    1.837210] tcp_listen_portaddr_hash hash table entries: 2048 (order: 3, 32768 bytes, linear)
[    1.838431] TCP established hash table entries: 32768 (order: 6, 262144 bytes, linear)
[    1.839788] TCP bind hash table entries: 32768 (order: 7, 524288 bytes, linear)
[    1.841220] TCP: Hash tables configured (established 32768 bind 32768)
[    1.846807] UDP hash table entries: 2048 (order: 4, 65536 bytes, linear)
[    1.848473] UDP-Lite hash table entries: 2048 (order: 4, 65536 bytes, linear)
[    1.855458] NET: Registered PF_UNIX/PF_LOCAL protocol family
[    1.858940] pci_bus 0000:00: resource 4 [io  0x0000-0x0cf7 window]
[    1.859502] pci_bus 0000:00: resource 5 [io  0x0d00-0xffff window]
[    1.859962] pci_bus 0000:00: resource 6 [mem 0x000a0000-0x000bffff window]
[    1.860515] pci_bus 0000:00: resource 7 [mem 0xc0000000-0xfebfffff window]
[    1.861027] pci_bus 0000:00: resource 8 [mem 0x140000000-0x1bfffffff window]
[    1.862944] pci 0000:00:01.0: PIIX3: Enabling Passive Release
[    1.863512] pci 0000:00:00.0: Limiting direct PCI/PCI transfers
[    1.864109] pci 0000:00:01.0: Activating ISA DMA hang workarounds
[    1.864784] PCI: CLS 0 bytes, default 64
[    1.866352] PCI-DMA: Using software bounce buffering for IO (SWIOTLB)
[    1.867226] software IO TLB: mapped [mem 0x00000000bbfe0000-0x00000000bffe0000] (64MB)
[    1.875567] kvm: no hardware support for 'kvm_intel'
[    1.886663] Trying to unpack rootfs image as initramfs...
[    1.888480] kvm: Nested Virtualization enabled
[    1.892947] SVM: kvm: Nested Paging disabled
[    1.895680] rootfs image is not initramfs (invalid magic at start of compressed archive); looks like an initrd
[    2.426388] PCLMULQDQ-NI instructions are not detected.
[    2.444817] Initialise system trusted keyrings
[    2.449406] workingset: timestamp_bits=52 max_order=20 bucket_order=0
[    2.451564] zbud: loaded
[    2.454793] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    2.454793] romfs: ROMFS MTD (C) 2007 Red Hat, Inc.
[    2.464031] fuse: init (API version 7.36)
[    2.529143] Key type asymmetric registered
[    2.529896] Asymmetric key parser 'x509' registered
[    2.531779] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 247)
[    2.532734] io scheduler mq-deadline registered
[    2.543276] crc32: CRC_LE_BITS = 64, CRC_BE BITS = 64
[    2.543730] crc32: self tests passed, processed 225944 bytes in 1604400 nsec
[    2.547370] crc32c: CRC_LE_BITS = 64
[    2.548533] crc32c: self tests passed, processed 225944 bytes in 1540360 nsec
[    2.632072] crc32_combine: 8373 self tests passed
[    2.702773] crc32c_combine: 8373 self tests passed
[    2.743003] input: Power Button as /devices/LNXSYSTM:00/LNXPWRBN:00/input/input0
[    2.745332] ACPI: button: Power Button [PWRF]
[    2.777033] Serial: 8250/16550 driver, 32 ports, IRQ sharing enabled
[    2.788038] 00:04: ttyS0 at I/O 0x3f8 (irq = 4, base_baud = 115200) is a 16550A
[    3.025344] brd: module loaded
[    3.145849] loop: module loaded
[    3.433397] zram: Added device: zram0
[    3.474066] scsi host0: ata_piix
[    3.484561] scsi host1: ata_piix
[    3.487302] ata1: PATA max MWDMA2 cmd 0x1f0 ctl 0x3f6 bmdma 0xc040 irq 14
[    3.487983] ata2: PATA max MWDMA2 cmd 0x170 ctl 0x376 bmdma 0xc048 irq 15
[    3.499272] slram: not enough parameters.
[    3.515527] tun: Universal TUN/TAP device driver, 1.6
[    3.521817] e100: Intel(R) PRO/100 Network Driver
[    3.525905] e100: Copyright(c) 1999-2006 Intel Corporation
[    3.530938] e1000: Intel(R) PRO/1000 Network Driver
[    3.531443] e1000: Copyright (c) 1999-2006 Intel Corporation.
[    3.547794] Freeing initrd memory: 146092K
[    3.675872] ata2: found unknown device (class 0)
[    3.682722] ata1: found unknown device (class 0)
[    3.691271] ata2.00: ATAPI: QEMU DVD-ROM, 2.5+, max UDMA/100
[    3.694969] ata1.00: ATA-7: QEMU HARDDISK, 2.5+, max UDMA/100
[    3.696181] ata1.00: 30375936 sectors, multi 16: LBA48 
[    3.729775] scsi 0:0:0:0: Direct-Access     ATA      QEMU HARDDISK    2.5+ PQ: 0 ANSI: 5
[    3.749830] sd 0:0:0:0: Attached scsi generic sg0 type 0
[    3.765697] sd 0:0:0:0: [sda] 30375936 512-byte logical blocks: (15.6 GB/14.5 GiB)
[    3.769772] scsi 1:0:0:0: CD-ROM            QEMU     QEMU DVD-ROM     2.5+ PQ: 0 ANSI: 5
[    3.780856] sd 0:0:0:0: [sda] Write Protect is off
[    3.786345] sd 0:0:0:0: [sda] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
[    3.799366] sr 1:0:0:0: [sr0] scsi3-mmc drive: 4x/4x cd/rw xa/form2 tray
[    3.801136] cdrom: Uniform CD-ROM driver Revision: 3.20
[    3.903292] sr 1:0:0:0: Attached scsi generic sg1 type 5
[    3.928546] sd 0:0:0:0: [sda] Attached SCSI disk
[    4.996799] ACPI: \_SB_.LNKC: Enabled at IRQ 11
[    5.339554] e1000 0000:00:03.0 eth0: (PCI:33MHz:32-bit) 00:0a:0b:0c:0d:0e
[    5.341515] e1000 0000:00:03.0 eth0: Intel(R) PRO/1000 Network Connection
[    5.344098] e1000e: Intel(R) PRO/1000 Network Driver
[    5.344613] e1000e: Copyright(c) 1999 - 2015 Intel Corporation.
[    5.345370] igb: Intel(R) Gigabit Ethernet Network Driver
[    5.345775] igb: Copyright (c) 2007-2014 Intel Corporation.
[    5.346600] igbvf: Intel(R) Gigabit Virtual Function Network Driver
[    5.347080] igbvf: Copyright (c) 2009 - 2012 Intel Corporation.
[    5.347796] ixgbe: Intel(R) 10 Gigabit PCI Express Network Driver
[    5.348317] ixgbe: Copyright (c) 1999-2016 Intel Corporation.
[    5.351605] ixgbevf: Intel(R) 10 Gigabit PCI Express Virtual Function Network Driver
[    5.352287] ixgbevf: Copyright (c) 2009 - 2018 Intel Corporation.
[    5.355757] ixgb: Intel(R) PRO/10GbE Network Driver
[    5.356383] ixgb: Copyright (c) 1999-2008 Intel Corporation.
[    5.358601] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
[    5.359876] ehci-pci: EHCI PCI platform driver
[    5.361349] ehci-platform: EHCI generic platform driver
[    5.362331] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
[    5.363098] ohci-pci: OHCI PCI platform driver
[    5.364135] ohci-platform: OHCI generic platform driver
[    5.364912] uhci_hcd: USB Universal Host Controller Interface driver
[    5.371890] usbcore: registered new interface driver uas
[    5.373423] usbcore: registered new interface driver usb-storage
[    5.374968] usbcore: registered new interface driver ums-sddr09
[    5.376364] usbcore: registered new interface driver ums-sddr55
[    5.378381] UDC core: couldn't find an available UDC - added [g_ether] to list of pending drivers
[    5.383628] i8042: PNP: PS/2 Controller [PNP0303:KBD,PNP0f13:MOU] at 0x60,0x64 irq 1,12
[    5.391925] serio: i8042 KBD port at 0x60,0x64 irq 1
[    5.392993] serio: i8042 AUX port at 0x60,0x64 irq 12
[    5.398400] mousedev: PS/2 mouse device common for all mice
[    5.401802] i2c_dev: i2c /dev entries driver
[    5.416908] input: AT Translated Set 2 keyboard as /devices/platform/i8042/serio0/input/input1
[    5.425988] NET: Registered PF_INET6 protocol family
[    5.453651] Segment Routing with IPv6
[    5.456143] In-situ OAM (IOAM) with IPv6
[    5.460581] NET: Registered PF_PACKET protocol family
[    5.461803] NET: Registered PF_KEY protocol family
[    5.464929] 8021q: 802.1Q VLAN Support v1.8
[    5.467807] Key type dns_resolver registered
[    5.468847] IPI shorthand broadcast: enabled
[    5.482366] registered taskstats version 1
[    5.484861] Loading compiled-in X.509 certificates
[    5.491896] zswap: loaded using pool lzo/zbud
[    5.501705] Key type ._fscrypt registered
[    5.502462] Key type .fscrypt registered
[    5.504682] Key type fscrypt-provisioning registered
[    5.575075] BIOS EDD facility v0.16 2004-Jun-25, 1 devices found
[    5.579880] Unstable clock detected, switching default tracing clock to "global"
[    5.579880] If you want to keep using the local clock, then add:
[    5.579880]   "trace_clock=local"
[    5.579880] on the kernel command line
[    5.600689] RAMDISK: squashfs filesystem found at block 0
[    5.601664] RAMDISK: Loading 146090KiB [1 disk] into ram disk... \
[    5.946808] /
[    6.303228] \
[    6.684704] /
[    7.078032] \
[    7.461895] /
[    7.909972] \
[    8.353924] /
[    8.768551] \
[    9.209369] /
[    9.661694] \
[   10.123825] /
[   10.595679] \
[   11.233174] /
[   11.751274] \
[   12.347755] /
[   12.877518] \
[   13.462036] /
[   14.036029] done.
[   17.536428] VFS: Mounted root (squashfs filesystem) readonly on device 1:0.
[   17.541307] devtmpfs: mounted
[   17.643565] Freeing unused kernel image (initmem) memory: 2068K
[   17.644640] Write protecting the kernel read-only data: 22528k
[   17.652902] Freeing unused kernel image (text/rodata gap) memory: 2036K
[   17.661348] Freeing unused kernel image (rodata/data gap) memory: 1584K
[   18.024088] x86/mm: Checked W+X mappings: passed, no W+X pages found.
[   18.025634] Run /sbin/init as init process
[   19.377855] EXT4-fs (sda): recovery complete
[   19.383278] EXT4-fs (sda): mounted filesystem with ordered data mode. Quota mode: none.
Mounting...
[   19.571530] overlayfs: "xino" feature enabled using 32 upper inode bits.
bash-5.1# ifconfig -a
eth0      Link encap:Ethernet  HWaddr 00:0A:0B:0C:0D:0E  
          BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          LOOPBACK  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# ls -la
total 12
drwxrwxr-x   18 0        0              277 Oct 15 04:18 .
drwxrwxr-x   18 0        0              277 Oct 15 04:18 ..
drwxrwxr-x    2 0        0              952 Oct 15 04:16 bin
drwxr-xr-x    5 0        0             3600 Oct 15 04:25 dev
drwxrwxr-x   20 0        0              328 Oct 15 04:18 disk
drwxrwxr-x    6 0        0               88 Oct 15 04:16 etc
drwxrwxr-x    3 0        0               27 Oct 13 05:35 include
drwxrwxr-x    4 0        0              240 Oct 15 04:18 lib
drwxrwxr-x    2 0        0               43 Oct 15 04:18 lib64
lrwxrwxrwx    1 0        0               11 Oct 15 04:18 linuxrc -> bin/busybox
drwxrwxr-x    2 0        0                3 Oct 13 03:15 low2
drwxr-xr-x    5 0        0             4096 Oct 15 00:50 mnt
drwxrwxr-x    1 1000     1000          4096 Oct 12 15:47 ovr
dr-xr-xr-x  144 0        0                0 Oct 15 04:26 proc
lrwxrwxrwx    1 0        0                9 Oct 15 04:18 root -> /var/root
drwxrwxr-x    2 0        0              610 Oct 15 04:16 sbin
[   39.051432] random: fast init done
drwxrwxr-x    5 0        0               49 Oct 13 05:35 share
dr-xr-xr-x   12 0        0                0 Oct 15 04:26 sys
lrwxrwxrwx    1 0        0                8 Oct 15 04:18 tmp -> /var/tmp
drwxrwxr-x    4 0        0               38 Oct 13 03:17 usr
drwxrwxrwt    6 0        0              120 Oct 15 04:26 var
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# 
bash-5.1# chroot ovr/
sh-5.1# 
sh-5.1#
sh-5.1# source /etc/rc.init
       ___    __      __ _  _     _      _
      / _ \  / /     / /| || |   | |    (_)
__  _| (_) |/ /_    / /_| || |_  | |     _ _ __  _   ___  __
\ \/ /> _ <| '_ \  | '_ \__   _| | |    | | '_ \| | | \ \/ /
 >  <| (_) | (_) | | (_) | | |   | |____| | | | | |_| |>  <
/_/\_\\___/ \___/   \___/  |_|   |______|_|_| |_|\__,_/_/\_\



chpasswd: password for 'root' changed

XCFGD Configurator - wait 5

[   93.018198] EXT4-fs (sdb): recovery complete
[   93.109111] EXT4-fs (sdb): mounted filesystem with ordered data mode. Quota mode: none.
Waiting it for getting ready to work..
Daemon Running...
DBG:main                :649  TEMPORARY FILE - SYNC : /var/tmp/xcfgXQxtEgu
DBG:main                :673  XML1 : /etc/config.xml
DBG:main                :669  DEV : /config/db
DBG:main                :665  STANDADLONE MODE
DBG:main                :752  TEMPORARY DIRECTORY = (/var/tmp/170)
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
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=3f76 size=836
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[BROKEN FIXUP]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=3f76 size=836
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[DIRTY+ONDIRTY CHECK]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=3f76 size=836
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[RECOVER BLOCK PROCESS]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=3f76 size=836
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[DIRTY BLOCK PROCESS]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=3f76 size=836
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_print:862  [[FINALLY WRITTEN]]
DBG:_xml_storage_header_print:867  INFO HEAD[ 0 ]=0x00000000 dirty=1 flags=0 crc=3f76 size=836
DBG:_xml_storage_header_print:867  INFO HEAD[ 1 ]=0x00020000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 2 ]=0x00040000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:867  INFO HEAD[ 3 ]=0x00060000 dirty=0 flags=0 crc=0 size=0
DBG:_xml_storage_header_print:870  ===========
DBG:_xml_storage_header_fetch:767  DIRTY HEADER[ 0  ] START = 0x20000 / DATA OFFSET = 524288 FLAG=0000
MSG:xml_storage_validate:1440 HEADER[0 ]::MAGIC=OK
MSG:xml_storage_validate:1441 HEADER[0 ]::RSIZE=2102
MSG:xml_storage_validate:1442 HEADER[0 ]::START=80000
MSG:xml_storage_validate:1443 HEADER[0 ]::CRC  =3f76
MSG:xml_storage_validate:1493 Temporary XML file name : (/var/tmp/170/config.xml)
DBG:xml_storage_validate:1507 /etc/config.xml write 2102/2102
MSG:xml_storage_validate:1519 STORAGE VALIDATED [0]
DBG:config_xml_merge    :401  VERSION (1.0.0.1 vs 1.0.0.1)
DBG:config_xml_merge    :404  CURRENTLY LATEST VERSION
3
2
1


DATE

Sat Oct 15 12:00:00 UTC 2022

Logging daemon


USER


GIT


PYTHON


Network - VMware should use Host-only interface for this OS

[  110.227389] e1000: Intel(R) PRO/1000 Network Driver
[  110.228007] e1000: Copyright (c) 1999-2006 Intel Corporation.
[  111.308984] ACPI: \_SB_.LNKC: Enabled at IRQ 11
[  111.635095] e1000 0000:00:03.0 eth0: (PCI:33MHz:32-bit) 00:0a:0b:0c:0d:0e
[  111.636504] e1000 0000:00:03.0 eth0: Intel(R) PRO/1000 Network Connection
udhcpc: started, v1.35.0
Setting IP address 0.0.0.0 on eth0
[  113.094337] 8021q: adding VLAN 0 to HW filter on device eth0
[  113.101170] e1000: eth0 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: RX
[  113.102432] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
udhcpc: broadcasting discover
udhcpc: broadcasting select for 192.167.0.162, server 192.167.0.1
udhcpc: lease of 192.167.0.162 obtained from 192.167.0.1, lease time 7200
Setting IP address 192.167.0.162 on eth0
Deleting routers
route: SIOCDELRT: No such process
Adding router 192.167.0.1
Recreating /etc/resolv.conf
 Adding DNS server 192.167.0.1
 Adding DNS server 8.8.8.8

SSH key setup in /root/.ssh


SSH daemon


WEBUI

killall: lighttpd: no process killed

Shell
bash-5.1#
bash-5.1#
bash-5.1# ifconfig
eth0      Link encap:Ethernet  HWaddr 00:0A:0B:0C:0D:0E  
          inet addr:192.167.0.162  Bcast:192.167.0.255  Mask:255.255.255.0
          inet6 addr: fe80::20a:bff:fe0c:d0e/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:103 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:8857 (8.6 KiB)  TX bytes:1200 (1.1 KiB)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

  ```

- User can see "eth0" interface is activated in VM, which will be named as "tap0" in host. 

## Procedures


# [1] Toolchain Building

- When user want to setup a toolchain under <strong>/opt/qvm</strong> folder.
```#!/bin/sh

  # sudo make TBOARD=vm TOOLCHAIN_ROOT=/opt/qvm toolchain
```
- With the following command, the toolchain will be setup under <strong>./gnu/toolchain</strong> folder.
```#!/bin/sh

  # make TBOARD=vm toolchain
```

 * "TBOARD" indicates type of board which is the name of folders in boards/ .
 * "TOOLCHAIN_ROOT" indicates the location of cross toolchain.


# [2] Libraries, Applications, Extra Applications Building

```#!/bin/sh

  # make TBOARD=vm lib app ext
```

# [3] Platform specfic binaries Building ( HTTP server, currently ) 

```#!/bin/sh

  # make TBOARD=vm proj 
```

# [4] Booting Image Building

```#!/bin/sh

  # make TBOARD=vm board ramdisk extdisk 
```


- Unlikely with the case of raspberry PI, we will get "output.iso" file under boards/vm .


# [5] Burning external USB disk partition 

```#!/bin/sh

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

```#!/bin/sh

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

# [6] Running/Stopping "output.iso"

- We can run this file as below.
```#!/bin/sh

  # make TBOARD=vm vmrun


  If you want to explicitly specify USB stick device, use the following option "EXT4HDD" . 


  # make TBOARD=vm EXT4HDD=/dev/sde1 vmrun 


  If you want to explicitly specify configuration partition device, use the following option "EXT4CFG" . 


  # make TBOARD=vm EXT4HDD=/dev/sde1 EXT4CFG=/dev/sdg2 vmrun 

```

- We can stop running QEMU emulator.
```#!/bin/sh

  # make TBOARD=vm vmstop
```

Host Network 
===============================

- VM machine is connected to host machine through "TAP" interface. 
- The following shows how host network is composed to support an isolated interface between VM and the host . 


```#!/bin/sh

#
# ifconfig 
br0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.167.0.120  netmask 255.255.255.0  broadcast 192.167.0.255
        inet6 ---------------------  prefixlen 64  scopeid 0x20<link>
        ether ---------------------  txqueuelen 1000  (Ethernet)
        RX packets 52600  bytes 61025479 (61.0 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 10051  bytes 1690332 (1.6 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

enp19s0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether ---------------------  txqueuelen 1000  (Ethernet)
        RX packets 166615  bytes 186468648 (186.4 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 93667  bytes 66442891 (66.4 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 6895  bytes 688217 (688.2 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 6895  bytes 688217 (688.2 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

tap0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet6 fe80::2c8c:afff:fec4:d674  prefixlen 64  scopeid 0x20<link>
        ether 2e:8c:af:c4:d6:74  txqueuelen 1000  (Ethernet)
        RX packets 29  bytes 3034 (3.0 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 277  bytes 52212 (52.2 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

#
#
# brctl show 
bridge name	bridge id		STP enabled	interfaces
br0		8000.f6f890ceb6be	no          enp19s0
                                        tap0


```

| Interface |  Description                              |
|-----------| ----------------------------------------- |
|  enp19s0  |  Ethernet interface of the host (laptop)  |
|  tap0     |  Virtual name of "eth0" in  VM            |


Partial Build
===============================

- Each applications/libaries are located under folders names among apps/, libs/, exts/ and they are all associated with specific category names.  

| Folders  |  Category Name   |
|----------| ---------------- |
|  apps/    |  app            |
|  libs/    |  lib            |
|  exts/    |  ext            |


- If you want to build "apps/vim" folder for VIM application, just do the following. "distclean" command removes the build temporary folder.

```#!/bin/sh

  # make TBOARD=vm SUBDIR=vim distclean app
```


- Building inside of each subfolders has 3 steps; prerpare, all, install. Each substep is triggered through {app|lib|ext}_### as below.  


```#!/bin/sh

  # make TBOARD=vm SUBDIR=vim app_all  
```
- If you want to build "exts/python" folder,  

```#!/bin/sh

  # make TBOARD=vm SUBDIR=python distclean ext
```

<span style="color:blue; font-size:4em">How Python</span>
===============

Setting up PIP 
---------

```#!/bin/sh

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

Upgrading PIP
---------

```#!/bin/sh

bash-5.1# python3 -m pip install --upgrade pip
WARNING: The directory '/.cache/pip' or its parent directory is not owned or is not writable by the current user. The cache has been disabled. Check the permissions and owner of that directory. If executing pip with sudo, you should use sudo'.
Requirement already satisfied: pip in /lib/python3.10/site-packages (22.2.2)
Collecting pip
  Downloading pip-22.3-py3-none-any.whl (2.1 MB)
     ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 2.1/2.1 MB 646.9 kB/s eta 0:00:00
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



