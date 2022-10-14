
WHY MicroBSP
==========


* Everyone is now happy with diverse uses and mega-tons of free resources of raspberry pi.
* How about embedded system system programer ?? - Still they are happy too?
* None of source-level software distribution for raspberry PI is disclosed on the Internet. It's very scarce.
* Just downloading and flashing images into SD card cannot make system programmer happy who is strongly eager to dig into system bottom level.
* Poor system programmer like me just can only spend less than \$50 for an embedded platform. - Raspberry PI becomes the answer.
* <strong>Yocto</strong> - a bunch set of meta-layers could be stressful to a system programmer exteremely familiar with old-dated build trees like uClinux.
* As a consequence, I simply ran to UART console based raspberry pi embedded board. I don't want fancy desktop environment, even I don't have extra mouse and keyboard to manipulate graphical widgets on HDMI monitor. HDMI monitor? Oh.. its more than $150 on Amazon.

MicroBSP
------------
1. To do something various in raspberry PI, the need of specific board support packaage was risen from a few of possible cases; (1) Free application running/testing/validation  (2) Probing/verifiying Linux kernel functios (3) Systematic distribution/management of software stacks .
2. MicroBSP was intended to assist the needs for my private individual reason and public share purposes based on its inherent flexibility and simplicity.

SHORTAGES
==========


* Since MicroBSP is based on classic Makefile, I could not find a way to automatically resolve <strong>build dependency</strong>.
* User should not change the order of folders listed in a environment <strong>SUBDIR</strong> in top-level Makefile.
* Python based build system like "Yocto" can generate <strong>RPM</strong> packages but MicroBSP cannot construct RPM packages yet.
* MicroBSP does not use QEMU VM system based cross compilation like "Yocto".


NOTICE
==========

* MicroBSP is 100% Makefile based raspberry pi software build tree.
* Software building begins with <strong>GNU toolchain construction</strong>.
* Everything has been tested and proved in <strong>Ubuntu 20.04</strong>.
* MicroBSP kicks at the starting line in cross compiler build to QT graphic system.
* Raspberry PI 3+ has been used for coding for MicroBSP. (boards/rpi3)
* X86_64 platform has been tested through QEMU emulator in ubuntu.

July/17/2022
* Development branch (dev1) has been modifed for <strong>Ubuntu LTS22.04 </strong>.
* "QT" is not supported. - How to build ?? 

Oct/11/2022
* GLIBC version is now 2.36
* GCC   version is now 11.2.0
* BINUTILS version is now 2.38

Required Softwares
===============================

- Fundamental libraries/applications will be installed by the following command.
- <strong>This can work in Ubuntu. </strong>

```#!/bin/sh

  # make installcomps
```


<span style="color:blue; font-size:2em">Raspberry PI</span>
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
* IPv6 support
* UART baudrate = 921600bps (not 115200bps)

Booting Shot
-----------

  ```sh

[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd034]
[    0.000000] Linux version 5.10.77-v8 (todd@vostro) (aarch64-linux-gcc (GCC) 8.3.0, GNU ld (GNU Binutils) 2.32) #6 SMP PREEMPT Tue Nov 9 09:14:34 PST 2021
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
[    0.000000] Kernel command line: coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.enable_compat_alsa=0 snd_bcm2835.enable_hdmi=1 bcm2708_fb.fbwidth=640 bcm2708_fb.fbheight=480 bcm2708_fb.fbswap=1 vc_mem.mem_
base=0x3ec00000 vc_mem.mem_size=0x40000000  dwg_otg.lpm_enable=0 console=ttyS0,921600 earlyprintk root=/dev/mmcblk0p2 rootfstype=squashfs rootwait elevator=deadline
[    0.000000] Kernel parameter elevator= does not have any effect anymore.
[    0.000000] Please use sysfs to set IO scheduler for individual devices.
[    0.000000] Dentry cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
[    0.000000] Inode-cache hash table entries: 65536 (order: 7, 524288 bytes, linear)
[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
[    0.000000] Memory: 860856K/970752K available (12096K kernel code, 2002K rwdata, 4456K rodata, 3968K init, 1236K bss, 44360K reserved, 65536K cma-reserved)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
[    0.000000] ftrace: allocating 37926 entries in 149 pages
[    0.000000] ftrace: allocated 149 pages with 4 groups
[    0.000000] rcu: Preemptible hierarchical RCU implementation.
[    0.000000] rcu:     RCU event tracing is enabled.
[    0.000000]  Trampoline variant of Tasks RCU enabled.
[    0.000000]  Rude variant of Tasks RCU enabled.
[    0.000000]  Tracing variant of Tasks RCU enabled.
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 100 jiffies.
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
[    0.000000] random: get_random_bytes called from start_kernel+0x3b0/0x570 with crng_init=1
[    0.000000] arch_timer: cp15 timer(s) running at 19.20MHz (phys).
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0x46d987e47, max_idle_ns: 440795202767 ns
[    0.000007] sched_clock: 56 bits at 19MHz, resolution 52ns, wraps every 4398046511078ns
[    0.000294] Console: colour dummy device 80x25
[    0.000370] Calibrating delay loop (skipped), value calculated using timer frequency.. 38.40 BogoMIPS (lpj=19200)
[    0.000411] pid_max: default: 32768 minimum: 301
[    0.000647] LSM: Security Framework initializing
[    0.000980] Mount-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
[    0.001025] Mountpoint-cache hash table entries: 2048 (order: 2, 16384 bytes, linear)
[    0.002695] cgroup: Disabling memory control group subsystem
[    0.006025] rcu: Hierarchical SRCU implementation.
[    0.007303] EFI services will not be available.
[    0.008082] smp: Bringing up secondary CPUs ...
[    0.009477] Detected VIPT I-cache on CPU1
[    0.009561] CPU1: Booted secondary processor 0x0000000001 [0x410fd034]
[    0.011218] Detected VIPT I-cache on CPU2
[    0.011276] CPU2: Booted secondary processor 0x0000000002 [0x410fd034]
[    0.012845] Detected VIPT I-cache on CPU3
[    0.012900] CPU3: Booted secondary processor 0x0000000003 [0x410fd034]
[    0.013136] smp: Brought up 1 node, 4 CPUs
[    0.013202] SMP: Total of 4 processors activated.
[    0.013227] CPU features: detected: 32-bit EL0 Support
[    0.013250] CPU features: detected: CRC32 instructions
[    0.047824] CPU: All CPU(s) started at EL2
[    0.047925] alternatives: patching kernel code
[    0.049938] devtmpfs: initialized
[    0.072295] Enabled cp15_barrier support
[    0.072351] Enabled setend support
[    0.072397] KASLR enabled
[    0.072718] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 1911260446275000 ns
[    0.072760] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
[    0.076337] pinctrl core: initialized pinctrl subsystem
[    0.077801] DMI not present or invalid.
[    0.078342] NET: Registered protocol family 16
[    0.089836] DMA: preallocated 1024 KiB GFP_KERNEL pool for atomic allocations
[    0.090365] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA pool for atomic allocations
[    0.091962] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA32 pool for atomic allocations
[    0.092226] audit: initializing netlink subsys (disabled)
[    0.092704] audit: type=2000 audit(0.091:1): state=initialized audit_enabled=0 res=1
[    0.093698] thermal_sys: Registered thermal governor 'step_wise'
[    0.094060] cpuidle: using governor menu
[    0.094687] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers.
[    0.094953] ASID allocator initialised with 32768 entries
[    0.095182] Serial: AMBA PL011 UART driver
[    0.123332] bcm2835-mbox 3f00b880.mailbox: mailbox enabled
[    0.126757] raspberrypi-firmware soc:firmware: Attached to firmware from 2021-10-29T10:49:08, variant start
[    0.127767] raspberrypi-firmware soc:firmware: Firmware hash is b8a114e5a9877e91ca8f26d1a5ce904b2ad3cf13
[    2.379563] cryptd: max_cpu_qlen set to 1000
[    2.382051] "cryptomgr_test" (71) uses obsolete ecb(arc4) skcipher
[    2.477310] DRBG: Continuing without Jitter RNG
[    2.505484] bcm2835-dma 3f007000.dma: DMA legacy API manager, dmachans=0x1
[    2.508793] SCSI subsystem initialized
[    2.509228] usbcore: registered new interface driver usbfs
[    2.509347] usbcore: registered new interface driver hub
[    2.509528] usbcore: registered new device driver usb
[    2.512570] clocksource: Switched to clocksource arch_sys_counter
[    4.188250] VFS: Disk quotas dquot_6.6.0
[    4.188384] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    4.188650] FS-Cache: Loaded
[    4.189020] CacheFiles: Loaded
[    4.206117] NET: Registered protocol family 2
[    4.206455] IP idents hash table entries: 16384 (order: 5, 131072 bytes, linear)
[    4.208184] tcp_listen_portaddr_hash hash table entries: 512 (order: 1, 8192 bytes, linear)
[    4.208242] TCP established hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    4.208530] TCP bind hash table entries: 8192 (order: 5, 131072 bytes, linear)
[    4.208739] TCP: Hash tables configured (established 8192 bind 8192)
[    4.209043] UDP hash table entries: 512 (order: 2, 16384 bytes, linear)
[    4.209113] UDP-Lite hash table entries: 512 (order: 2, 16384 bytes, linear)
[    4.209501] NET: Registered protocol family 1
[    4.210705] RPC: Registered named UNIX socket transport module.
[    4.210730] RPC: Registered udp transport module.
[    4.210752] RPC: Registered tcp transport module.
[    4.210773] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    4.214825] hw perfevents: enabled with armv8_cortex_a53 PMU driver, 7 counters available
[    4.217658] Initialise system trusted keyrings
[    4.218079] workingset: timestamp_bits=46 max_order=18 bucket_order=0
[    4.228898] zbud: loaded
[    4.231469] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    4.231904] FS-Cache: Netfs 'nfs' registered for caching
[    4.232957] NFS: Registering the id_resolver key type
[    4.233015] Key type id_resolver registered
[    4.233038] Key type id_legacy registered
[    4.233225] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
[    4.233251] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering...
[    4.233666] fuse: init (API version 7.32)
[    4.307399] NET: Registered protocol family 38
[    4.307437] Key type asymmetric registered
[    4.307461] Asymmetric key parser 'x509' registered
[    4.307588] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 249)
[    4.307614] io scheduler mq-deadline registered
[    4.307638] io scheduler kyber registered
[    4.385284] xz_dec_test: module loaded
[    4.385310] xz_dec_test: Create a device node with 'mknod xz_dec_test c 248 0' and write .xz files to it.
[    4.390740] bcm2708_fb soc:fb: FB found 1 display(s)
[    4.410801] Console: switching to colour frame buffer device 80x30
[    4.416171] bcm2708_fb soc:fb: Registered framebuffer for display 0, size 640x480
[    4.423009] Serial: 8250/16550 driver, 1 ports, IRQ sharing enabled
[    4.424357] bcm2835-aux-uart 3f215040.serial: there is not valid maps for state default
[    4.427199] bcm2835-rng 3f104000.rng: hwrng registered
[    4.427887] vc-mem: phys_addr:0x00000000 mem_base=0x3ec00000 mem_size:0x40000000(1024 MiB)
[    4.429946] gpiomem-bcm2835 3f200000.gpiomem: Initialised: Registers at 0x3f200000
[    4.430500] cacheinfo: Unable to detect cache hierarchy for CPU 0
[    4.448731] brd: module loaded
[    4.467787] loop: module loaded
[    4.470428] Loading iSCSI transport class v2.0-870.
[    4.473121] libphy: Fixed MDIO Bus: probed
[    4.473700] usbcore: registered new interface driver lan78xx
[    4.473809] usbcore: registered new interface driver smsc95xx
[    4.473846] dwc_otg: version 3.00a 10-AUG-2012 (platform bus)
[    5.202481] Core Release: 2.80a
[    5.202536] Setting default values for core params
[    5.202578] Finished setting default values for core params
[    5.403061] Using Buffer DMA mode
[    5.403086] Periodic Transfer Interrupt Enhancement - disabled
[    5.403107] Multiprocessor Interrupt Enhancement - disabled
[    5.403130] OTG VER PARAM: 0, OTG VER FLAG: 0
[    5.403156] Dedicated Tx FIFOs mode
[    5.404058]
[    5.404082] WARN::dwc_otg_hcd_init:1074: FIQ DMA bounce buffers: virt = ffffffc010694000 dma = 0x00000000f7810000 len=9024
[    5.404130] FIQ FSM acceleration enabled for :
[    5.404130] Non-periodic Split Transactions
[    5.404130] Periodic Split Transactions
[    5.404130] High-Speed Isochronous Endpoints
[    5.404130] Interrupt/Control Split Transaction hack enabled
[    5.404209]
[    5.404226] WARN::hcd_init_fiq:497: MPHI regs_base at ffffffc01006d000
[    5.404286] dwc_otg 3f980000.usb: DWC OTG Controller
[    5.404335] dwc_otg 3f980000.usb: new USB bus registered, assigned bus number 1
[    5.404405] dwc_otg 3f980000.usb: irq 74, io mem 0x00000000
[    5.404468] Init: Port Power? op_state=1
[    5.404489] Init: Power Port (0)
[    5.405006] usb usb1: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 5.10
[    5.405034] usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1
[    5.405060] usb usb1: Product: DWC OTG Controller
[    5.405085] usb usb1: Manufacturer: Linux 5.10.77-v8 dwc_otg_hcd
[    5.405108] usb usb1: SerialNumber: 3f980000.usb
[    5.406335] hub 1-0:1.0: USB hub found
[    5.406441] hub 1-0:1.0: 1 port detected
[    5.409495] usbcore: registered new interface driver usb-storage
[    5.411779] bcm2835-wdt bcm2835-wdt: Broadcom BCM2835 watchdog timer
[    5.416080] sdhci: Secure Digital Host Controller Interface driver
[    5.416102] sdhci: Copyright(c) Pierre Ossman
[    5.417043] mmc-bcm2835 3f300000.mmcnr: could not get clk, deferring probe
[    5.418089] sdhost-bcm2835 3f202000.mmc: could not get clk, deferring probe
[    5.418467] sdhci-pltfm: SDHCI platform and OF driver helper
[    5.420917] ledtrig-cpu: registered to indicate activity on CPUs
[    5.421569] hid: raw HID events driver (C) Jiri Kosina
[    5.421828] usbcore: registered new interface driver usbhid
[    5.421850] usbhid: USB HID core driver
[    5.422320] ashmem: initialized
[    5.429715] ipip: IPv4 and MPLS over IPv4 tunneling driver
[    5.430617] gre: GRE over IPv4 demultiplexor driver
[    5.430669] Initializing XFRM netlink socket
[    5.431676] NET: Registered protocol family 10
[    5.433907] Segment Routing with IPv6
[    5.435220] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
[    5.437058] ip6_gre: GRE over IPv6 tunneling driver
[    5.437984] NET: Registered protocol family 17
[    5.438050] NET: Registered protocol family 15
[    5.438126] bridge: filtering via arp/ip/ip6tables is no longer available by default. Update your scripts to load br_netfilter if you need this.
[    5.438249] 8021q: 802.1Q VLAN Support v1.8
[    5.438341] Key type dns_resolver registered
[    5.439379] registered taskstats version 1
[    5.439421] Loading compiled-in X.509 certificates
[    5.440281] Key type ._fscrypt registered
[    5.440306] Key type .fscrypt registered
[    5.440329] Key type fscrypt-provisioning registered
[    5.459739] uart-pl011 3f201000.serial: cts_event_workaround enabled
[    5.459891] 3f201000.serial: ttyAMA0 at MMIO 0x3f201000 (irq = 99, base_baud = 0) is a PL011 rev2
[    5.463357] bcm2835-aux-uart 3f215040.serial: there is not valid maps for state default
[    5.464323] printk: console [ttyS0] disabled
[    5.464655] 3f215040.serial: ttyS0 at MMIO 0x3f215040 (irq = 71, base_baud = 31250000) is a 16550
[    5.628958] printk: console [ttyS0] enabled
[    5.631346] bcm2835-power bcm2835-power: Broadcom BCM2835 power domains driver
[    5.635083] mmc-bcm2835 3f300000.mmcnr: mmc_debug:0 mmc_debug2:0
[    5.635920] mmc-bcm2835 3f300000.mmcnr: DMA channel allocated
[    5.638792] Indeed it is in host mode hprt0 = 00021501
[    5.663381] sdhost: log_buf @ (____ptrval____) (c313a000)
[    5.732602] mmc0: sdhost-bcm2835 loaded - DMA enabled (>1)
[    5.738658] of_cfs_init
[    5.739181] of_cfs_init: OK
[    5.739875] cfg80211: Loading compiled-in X.509 certificates for regulatory database
[    5.762174] mmc1: queuing unknown CIS tuple 0x80 (2 bytes)
[    5.764747] mmc1: queuing unknown CIS tuple 0x80 (3 bytes)
[    5.767239] mmc1: queuing unknown CIS tuple 0x80 (3 bytes)
[    5.771155] mmc1: queuing unknown CIS tuple 0x80 (7 bytes)
[    5.796103] mmc0: host does not support reading read-only switch, assuming write-enable
[    5.800692] mmc0: new high speed SDXC card at address 0001
[    5.803400] mmcblk0: mmc0:0001 GC2QT 59.6 GiB
[    5.811217]  mmcblk0: p1 p2 p3 p4 < p5 p6 p7 >
[    5.834595] usb 1-1: new high-speed USB device number 2 using dwc_otg
[    5.835749] Indeed it is in host mode hprt0 = 00001101
[    5.857743] mmc1: new high speed SDIO card at address 0001
[    6.021055] cfg80211: Loaded X.509 cert 'sforshee: 00b28ddf47aef9cea7'
[    6.022860] platform regulatory.0: Direct firmware load for regulatory.db failed with error -2
[    6.024066] cfg80211: failed to load regulatory.db
[    6.025311] usb 1-1: New USB device found, idVendor=0424, idProduct=2514, bcdDevice= b.b3
[    6.026461] usb 1-1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    6.028759] hub 1-1:1.0: USB hub found
[    6.029475] hub 1-1:1.0: 4 ports detected
[    6.035754] VFS: Mounted root (squashfs filesystem) readonly on device 179:2.
[    6.040424] devtmpfs: mounted
[    6.053152] Freeing unused kernel memory: 3968K
[    6.053957] Run /sbin/init as init process
[    6.315631] usb 1-1.1: new high-speed USB device number 3 using dwc_otg
[    6.405178] usb 1-1.1: New USB device found, idVendor=0424, idProduct=2514, bcdDevice= b.b3
[    6.406631] usb 1-1.1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
[    6.408993] hub 1-1.1:1.0: USB hub found
[    6.409733] hub 1-1.1:1.0: 3 ports detected
[    6.486750] EXT4-fs (mmcblk0p3): recovery complete
[    6.488605] usb 1-1.2: new high-speed USB device number 4 using dwc_otg
[    6.489790] EXT4-fs (mmcblk0p3): mounted filesystem with ordered data mode. Opts: (null)
[    6.508175] EXT4-fs (mmcblk0p6): recovery complete
[    6.508914] EXT4-fs (mmcblk0p6): mounted filesystem with ordered data mode. Opts: (null)
[    6.543297] EXT4-fs (mmcblk0p7): recovery complete
[    6.544039] EXT4-fs (mmcblk0p7): mounted filesystem with ordered data mode. Opts: (null)
Mounting...
[    6.563444] overlayfs: "xino" feature enabled using 32 upper inode bits.
Device file system
Change root !!
[    6.581912] usb 1-1.2: New USB device found, idVendor=2357, idProduct=0109, bcdDevice= 2.00
[    6.583104] usb 1-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[    6.584116] usb 1-1.2: SerialNumber: 00e04c000001
[    6.814484] random: crng init done
[    7.214605] usb 1-1.1.1: new high-speed USB device number 5 using dwc_otg
[    7.304569] usb 1-1.1.1: New USB device found, idVendor=0424, idProduct=7800, bcdDevice= 3.00
[    7.305773] usb 1-1.1.1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
chpasswd: password for 'root' changed
Thu Aug  1 12:00:00 UTC 2019


INIT FILE STARTS ...


Syslog Daemon
TELNET daemon
BOOT Partition
[    7.410742] FAT-fs (mmcblk0p1): Volume was not properly unmounted. Some data may be corrupt. Please run fsck.
Python !!
[    7.575178] lan78xx 1-1.1.1:1.0 (unnamed net_device) (uninitialized): No External EEPROM. Setting MAC Speed
[    7.579258] libphy: lan78xx-mdiobus: probed
[WLAN] RTK Loading modules...
[    8.290304] 8192eu: loading out-of-tree module taints kernel.
[    8.531811] usbcore: registered new interface driver rtl8192eu
grep: wlan0: No such file or directory
[WLAN] Building up /var/tmp/wpa_supplicant/1.conf
[WLAN] Running WPA Suppplicant !!
[   10.280649] device wlan0 entered promiscuous mode
tcpdump: listening on wlan0, link-type EN10MB (Ethernet), capture size 262144 bytes
Successfully initialized wpa_supplicant
[   12.064555] lan78xx 1-1.1.1:1.0 (unnamed net_device) (uninitialized): int urb period 64
ioctl[SIOCSIWENCODEEXT]: Invalid argument
ioctl[SIOCSIWENCODEEXT]: Invalid argument
Waiting Connect .. 0
Waiting Connect .. 1
[   16.022858] IPv6: ADDRCONF(NETDEV_CHANGE): wlan0: link becomes ready
Waiting Connect .. 2
IPv4 DHCP ...
Waiting IP .. 0
Waiting IP .. 1
Waiting IP .. 2
Waiting IP .. 3
Waiting IP .. 4
[WLAN] BRCM Loading modules...
[   22.686434] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   22.688738] usbcore: registered new interface driver brcmfmac
[   22.704273] 8021q: adding VLAN 0 to HW filter on device eth0
[   22.871589] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   22.873309] brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6
[   22.893577] brcmfmac: brcmf_c_preinit_dcmds: Firmware: BCM4345/6 wl0: Feb 27 2018 03:15:32 version 7.45.154 (r684107 CY) FWID 01-4fbe0b04
[WLAN] Building up /var/tmp/hostapd/1.conf
[WLAN] Running AP Suppplicant !!
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
nl80211: Subscribe to mgmt frames with AP handle 0x3bcb3790 (device SME)
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=04
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=0501
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=0503
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=0504
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=06
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=08
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=09
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=0a
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=11
nl80211: Register frame type=0xd0 (WLAN_FC_STYPE_ACTION) nl_handle=0x3bcb3790 match=7f
nl80211: Register frame type=0xb0 (WLAN_FC_STYPE_AUTH) nl_handle=0x3bcb3790 match=
nl80211: Enable Probe Request reporting nl_preq=0x3bcb8df0
nl80211: Register frame type=0x40 (WLAN_FC_STYPE_PROBE_REQ) nl_handle=0x3bcb8df0 match=
rfkill: initial event: idx=1 type=1 op=0 soft=0 hard=0
[BRIDGE] Bridge creation....
[   23.957513] br0: port 1(eth0) entered blocking state
[   23.958257] br0: port 1(eth0) entered disabled state
[   23.960217] device eth0 entered promiscuous mode
[   23.973671] br0: port 2(wlan1) entered blocking state
[   23.974441] br0: port 2(wlan1) entered disabled state
[   23.975888] device wlan1 entered promiscuous mode
Internet Systems Consortium DHCP Server 4.1.2
Copyright 2004-2010 Internet Systems Consortium.
All rights reserved.
For info, please visit https://www.isc.org/software/dhcp/
Wrote 0 leases to leases file.
Listening on LPF/br0/b8:27:eb:aa:c5:b5/172.145.1.0/24
Sending on   LPF/br0/b8:27:eb:aa:c5:b5/172.145.1.0/24
Sending on   Socket/fallback/fallback-net
[   25.058719] device br0 entered promiscuous mode
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

tunl0     Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-B0-74-00-00-00-00-00-00-00-00  
          NOARP  MTU:1480  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

wlan0     Link encap:Ethernet  HWaddr D4:6E:0E:13:B9:8C  
          inet addr:192.167.0.107  Bcast:192.167.0.255  Mask:255.255.255.0
          inet6 addr: fe80::d66e:eff:fe13:b98c/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:25 errors:0 dropped:0 overruns:0 frame:0
          TX packets:11 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:4476 (4.3 KiB)  TX bytes:1868 (1.8 KiB)

wlan1     Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)



nameserver 127.0.0.1
nameserver 192.167.0.1
+++++++++++++++++++++++++++++++++++++++++++++++++++++
[DNS] Masquerading start ...
nl80211: Add own interface ifindex 10 (ifidx_reason 9)
nl80211: if_indices[16]: 10(9)
nl80211: Add own interface ifindex 9 (ifidx_reason -1)
nl80211: if_indices[16]: 10(9) 9(-1)
phy: phy1
BSS count 1, BSSID mask 00:00:00:00:00:00 (0 bits)
wlan1: interface state UNINITIALIZED->COUNTRY_UPDATE
Previous country code 00, new country code US
Continue interface setup after channel list update
ctrl_iface not configured!
[   31.775406] IPv6: ADDRCONF(NETDEV_CHANGE): wlan1: link becomes ready
[   31.776724] br0: port 2(wlan1) entered blocking state
[   31.777430] br0: port 2(wlan1) entered forwarding state
[   31.778636] IPv6: ADDRCONF(NETDEV_CHANGE): br0: link becomes ready


ROOT PARTITION ...


[   42.655894] EXT4-fs (mmcblk0p5): recovery complete
[   42.656633] EXT4-fs (mmcblk0p5): mounted filesystem with ordered data mode. Opts: (null)


CONSOLE ...


 _____                 _                              _____ _____
|  __ \               | |                            |  __ \_   _|
| |__) |__ _ ___ _ __ | |__   ___ _ __ _ __ _   _    | |__) || |  
|  _  // _` / __| '_ \| '_ \ / _ \ '__| '__| | | |   |  ___/ | |  
| | \ \ (_| \__ \ |_) | |_) |  __/ |  | |  | |_| |   | |    _| |_
|_|  \_\__,_|___/ .__/|_.__/ \___|_|  |_|   \__, |   |_|   |_____|
                | |                          __/ |              
                |_|                         |___/               

bash-5.1# ifconfig
br0       Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet addr:172.145.1.1  Bcast:172.145.1.255  Mask:255.255.255.0
          inet6 addr: fe80::ba27:ebff:feaa:c5b5/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:736 (736.0 B)

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
          inet addr:192.167.0.107  Bcast:192.167.0.255  Mask:255.255.255.0
          inet6 addr: fe80::d66e:eff:fe13:b98c/64 Scope:Link
          inet6 addr: 2600:8802:1700:f8f::1001/64 Scope:Global
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:68 errors:0 dropped:0 overruns:0 frame:0
          TX packets:17 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:17225 (16.8 KiB)  TX bytes:2668 (2.6 KiB)

wlan1     Link encap:Ethernet  HWaddr B8:27:EB:AA:C5:B5  
          inet6 addr: fe80::ba27:ebff:feaa:c5b5/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:16 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:1776 (1.7 KiB)

bash-5.1#
bash-5.1#
bash-5.1#
bash-5.1# ping6 www.google.com
PING www.google.com (2607:f8b0:4007:814::2004): 56 data bytes
64 bytes from 2607:f8b0:4007:814::2004: seq=0 ttl=117 time=87.560 ms
64 bytes from 2607:f8b0:4007:814::2004: seq=1 ttl=117 time=165.328 ms
64 bytes from 2607:f8b0:4007:814::2004: seq=2 ttl=117 time=65.691 ms
64 bytes from 2607:f8b0:4007:814::2004: seq=3 ttl=117 time=295.226 ms
64 bytes from 2607:f8b0:4007:814::2004: seq=4 ttl=117 time=187.865 ms
64 bytes from 2607:f8b0:4007:814::2004: seq=5 ttl=117 time=70.300 ms
64 bytes from 2607:f8b0:4007:814::2004: seq=6 ttl=117 time=199.497 ms
^C
--- www.google.com ping statistics ---
7 packets transmitted, 7 packets received, 0% packet loss
round-trip min/avg/max = 65.691/153.066/295.226 ms
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

<span style="color:blue; font-size:2em">Ubuntu QEMU</span>
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

[    0.000000] Linux version 5.17.7 (todd@vostro) (x86_64-any-linux-gnu-gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.38) #15 SMP PREEMPT Thu Oct 13 22:04:02 PDT 2022
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
[    0.000000] RAMDISK: [mem 0xffffffffb3113000-0xffffffffbce9cfff]
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
[    0.000000] percpu: Embedded 53 pages/cpu s178264 r8192 d30632 u524288
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
[    0.000000] Memory: 3873548K/4193784K available (12295K kernel code, 3052K rwdata, 4092K rodata, 2016K init, 3468K bss, 319976K reserved, 0K cma-reserved)
[    0.000000] ftrace: allocating 36903 entries in 145 pages
[    0.000000] ftrace: allocated 145 pages with 3 groups
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
[    0.012000] APIC: Switch to symmetric I/O mode setup
[    0.020000] ..TIMER: vector=0x30 apic1=0 pin1=2 apic2=-1 pin2=-1
[    0.052000] tsc: Unable to calibrate against PIT
[    0.056000] tsc: using HPET reference calibration
[    0.056000] tsc: Detected 2659.980 MHz processor
[    0.000644] tsc: Marking TSC unstable due to TSCs unsynchronized
[    0.001843] Calibrating delay loop (skipped), value calculated using timer frequency.. 5319.96 BogoMIPS (lpj=10639920)
[    0.006718] pid_max: default: 32768 minimum: 301
[    0.011198] random: get_random_bytes called from setup_net+0x4d/0x290 with crng_init=0
[    0.017728] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.018388] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.050999] process: using AMD E400 aware idle routine
[    0.051745] Last level iTLB entries: 4KB 512, 2MB 255, 4MB 127
[    0.052179] Last level dTLB entries: 4KB 512, 2MB 255, 4MB 127, 1GB 0
[    0.053088] Spectre V1 : Mitigation: usercopy/swapgs barriers and __user pointer sanitization
[    0.053938] Spectre V2 : Mitigation: Retpolines
[    0.054312] Spectre V2 : Spectre v2 / SpectreRSB mitigation: Filling RSB on context switch
[    0.080265] Freeing SMP alternatives memory: 32K
[    0.218767] smpboot: CPU0: AMD QEMU Virtual CPU version 2.5+ (family: 0xf, model: 0x6b, stepping: 0x1)
[    0.235726] cblist_init_generic: Setting adjustable number of callback queues.
[    0.236536] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.238025] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.239338] cblist_init_generic: Setting shift to 2 and lim to 1.
[    0.240280] Performance Events: PMU not available due to virtualization, using software events only.
[    0.245874] rcu: Hierarchical SRCU implementation.
[    0.262456] NMI watchdog: Perf NMI watchdog permanently disabled
[    0.269476] smp: Bringing up secondary CPUs ...
[    0.286332] x86: Booting SMP configuration:
[    0.286854] .... node  #0, CPUs:      #1 #2
[    0.000000] calibrate_delay_direct() dropping min bogoMips estimate 1 = 8970226
[    0.492161]  #3
[    0.000000] calibrate_delay_direct() dropping max bogoMips estimate 3 = 12695327
[    0.570929] smp: Brought up 1 node, 4 CPUs
[    0.573541] smpboot: Max logical packages: 1
[    0.573989] smpboot: Total of 4 processors activated (21253.50 BogoMIPS)
[    0.613707] devtmpfs: initialized
[    0.623605] x86/mm: Memory block size: 128MB
[    0.653523] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    0.654617] futex hash table entries: 1024 (order: 4, 65536 bytes, linear)
[    0.658183] pinctrl core: initialized pinctrl subsystem
[    0.680141] NET: Registered PF_NETLINK/PF_ROUTE protocol family
[    0.686578] audit: initializing netlink subsys (disabled)
[    0.690721] audit: type=2000 audit(1665724073.744:1): state=initialized audit_enabled=0 res=1
[    0.704661] thermal_sys: Registered thermal governor 'step_wise'
[    0.704892] thermal_sys: Registered thermal governor 'user_space'
[    0.710183] cpuidle: using governor menu
[    0.716443] PCI: Using configuration type 1 for base access
[    0.725150] mtrr: your CPUs had inconsistent fixed MTRR settings
[    0.725577] mtrr: your CPUs had inconsistent variable MTRR settings
[    0.726089] mtrr: your CPUs had inconsistent MTRRdefType settings
[    0.726522] mtrr: probably your BIOS does not setup all CPUs.
[    0.727024] mtrr: corrected configuration.
[    0.788057] kprobes: kprobe jump-optimization is enabled. All kprobes are optimized if possible.
[    0.800274] HugeTLB registered 2.00 MiB page size, pre-allocated 0 pages
[    0.813925] cryptd: max_cpu_qlen set to 1000
[    0.820579] ACPI: Added _OSI(Module Device)
[    0.820999] ACPI: Added _OSI(Processor Device)
[    0.821327] ACPI: Added _OSI(3.0 _SCP Extensions)
[    0.821553] ACPI: Added _OSI(Processor Aggregator Device)
[    0.822625] ACPI: Added _OSI(Linux-Dell-Video)
[    0.823333] ACPI: Added _OSI(Linux-Lenovo-NV-HDMI-Audio)
[    0.823976] ACPI: Added _OSI(Linux-HPI-Hybrid-Graphics)
[    0.873967] ACPI: 1 ACPI AML tables successfully acquired and loaded
[    0.914851] ACPI: Interpreter enabled
[    0.916275] ACPI: PM: (supports S0 S5)
[    0.916640] ACPI: Using IOAPIC for interrupt routing
[    0.918180] PCI: Using host bridge windows from ACPI; if necessary, use "pci=nocrs" and report a bug
[    0.922810] ACPI: Enabled 2 GPEs in block 00 to 0F
[    1.050095] ACPI: PCI Root Bridge [PCI0] (domain 0000 [bus 00-ff])
[    1.052946] acpi PNP0A03:00: _OSC: OS supports [ASPM ClockPM Segments MSI HPX-Type3]
[    1.054132] acpi PNP0A03:00: _OSC: not requesting OS control; OS requires [ExtendedConfig ASPM ClockPM MSI]
[    1.056350] acpi PNP0A03:00: fail to add MMCONFIG information, can't access extended PCI configuration space under this bridge.
[    1.067135] PCI host bridge to bus 0000:00
[    1.068031] pci_bus 0000:00: root bus resource [io  0x0000-0x0cf7 window]
[    1.068769] pci_bus 0000:00: root bus resource [io  0x0d00-0xffff window]
[    1.069262] pci_bus 0000:00: root bus resource [mem 0x000a0000-0x000bffff window]
[    1.069969] pci_bus 0000:00: root bus resource [mem 0xc0000000-0xfebfffff window]
[    1.070551] pci_bus 0000:00: root bus resource [mem 0x140000000-0x1bfffffff window]
[    1.071669] pci_bus 0000:00: root bus resource [bus 00-ff]
[    1.076006] pci 0000:00:00.0: [8086:1237] type 00 class 0x060000
[    1.095756] pci 0000:00:01.0: [8086:7000] type 00 class 0x060100
[    1.098249] pci 0000:00:01.1: [8086:7010] type 00 class 0x010180
[    1.107337] pci 0000:00:01.1: reg 0x20: [io  0xc040-0xc04f]
[    1.110672] pci 0000:00:01.1: legacy IDE quirk: reg 0x10: [io  0x01f0-0x01f7]
[    1.111566] pci 0000:00:01.1: legacy IDE quirk: reg 0x14: [io  0x03f6]
[    1.112397] pci 0000:00:01.1: legacy IDE quirk: reg 0x18: [io  0x0170-0x0177]
[    1.113175] pci 0000:00:01.1: legacy IDE quirk: reg 0x1c: [io  0x0376]
[    1.118037] pci 0000:00:01.3: [8086:7113] type 00 class 0x068000
[    1.119852] pci 0000:00:01.3: quirk: [io  0x0600-0x063f] claimed by PIIX4 ACPI
[    1.120547] pci 0000:00:01.3: quirk: [io  0x0700-0x070f] claimed by PIIX4 SMB
[    1.122950] pci 0000:00:02.0: [1234:1111] type 00 class 0x030000
[    1.125278] pci 0000:00:02.0: reg 0x10: [mem 0xfd000000-0xfdffffff pref]
[    1.129416] pci 0000:00:02.0: reg 0x18: [mem 0xfebb0000-0xfebb0fff]
[    1.142987] pci 0000:00:02.0: reg 0x30: [mem 0xfeba0000-0xfebaffff pref]
[    1.145693] pci 0000:00:02.0: Video device with shadowed ROM at [mem 0x000c0000-0x000dffff]
[    1.161340] pci 0000:00:03.0: [8086:100e] type 00 class 0x020000
[    1.164836] pci 0000:00:03.0: reg 0x10: [mem 0xfeb80000-0xfeb9ffff]
[    1.165416] pci 0000:00:03.0: reg 0x14: [io  0xc000-0xc03f]
[    1.183296] pci 0000:00:03.0: reg 0x30: [mem 0xfeb00000-0xfeb7ffff pref]
[    1.245166] ACPI: PCI: Interrupt link LNKA configured for IRQ 10
[    1.258042] ACPI: PCI: Interrupt link LNKB configured for IRQ 10
[    1.263346] ACPI: PCI: Interrupt link LNKC configured for IRQ 11
[    1.274022] ACPI: PCI: Interrupt link LNKD configured for IRQ 11
[    1.276298] ACPI: PCI: Interrupt link LNKS configured for IRQ 9
[    1.298910] SCSI subsystem initialized
[    1.310747] ACPI: bus type USB registered
[    1.311059] usbcore: registered new interface driver usbfs
[    1.313692] usbcore: registered new interface driver hub
[    1.315816] usbcore: registered new device driver usb
[    1.317416] pps_core: LinuxPPS API ver. 1 registered
[    1.317416] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    1.333580] PTP clock support registered
[    1.380144] PCI: Using ACPI for IRQ routing
[    1.389553] hpet: 3 channels of 0 reserved for per-cpu timers
[    1.441316] clocksource: Switched to clocksource hpet
[    1.887907] VFS: Disk quotas dquot_6.6.0
[    1.888753] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    1.893286] FS-Cache: Loaded
[    1.905159] CacheFiles: Loaded
[    1.908036] pnp: PnP ACPI init
[    1.929583] pnp: PnP ACPI: found 6 devices
[    2.041342] clocksource: acpi_pm: mask: 0xffffff max_cycles: 0xffffff, max_idle_ns: 2085701024 ns
[    2.043967] NET: Registered PF_INET protocol family
[    2.049262] IP idents hash table entries: 65536 (order: 7, 524288 bytes, linear)
[    2.066384] tcp_listen_portaddr_hash hash table entries: 2048 (order: 3, 32768 bytes, linear)
[    2.067375] TCP established hash table entries: 32768 (order: 6, 262144 bytes, linear)
[    2.071769] TCP bind hash table entries: 32768 (order: 7, 524288 bytes, linear)
[    2.075078] TCP: Hash tables configured (established 32768 bind 32768)
[    2.079224] UDP hash table entries: 2048 (order: 4, 65536 bytes, linear)
[    2.082723] UDP-Lite hash table entries: 2048 (order: 4, 65536 bytes, linear)
[    2.088447] NET: Registered PF_UNIX/PF_LOCAL protocol family
[    2.091230] pci_bus 0000:00: resource 4 [io  0x0000-0x0cf7 window]
[    2.091741] pci_bus 0000:00: resource 5 [io  0x0d00-0xffff window]
[    2.092189] pci_bus 0000:00: resource 6 [mem 0x000a0000-0x000bffff window]
[    2.092675] pci_bus 0000:00: resource 7 [mem 0xc0000000-0xfebfffff window]
[    2.093163] pci_bus 0000:00: resource 8 [mem 0x140000000-0x1bfffffff window]
[    2.099444] pci 0000:00:01.0: PIIX3: Enabling Passive Release
[    2.100338] pci 0000:00:00.0: Limiting direct PCI/PCI transfers
[    2.101204] pci 0000:00:01.0: Activating ISA DMA hang workarounds
[    2.101928] PCI: CLS 0 bytes, default 64
[    2.105240] PCI-DMA: Using software bounce buffering for IO (SWIOTLB)
[    2.106429] software IO TLB: mapped [mem 0x00000000bbfe0000-0x00000000bffe0000] (64MB)
[    2.113578] Trying to unpack rootfs image as initramfs...
[    2.113610] PCLMULQDQ-NI instructions are not detected.
[    2.120124] rootfs image is not initramfs (invalid magic at start of compressed archive); looks like an initrd
[    2.132707] Initialise system trusted keyrings
[    2.140859] workingset: timestamp_bits=52 max_order=20 bucket_order=0
[    2.153427] zbud: loaded
[    2.167137] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    2.168771] romfs: ROMFS MTD (C) 2007 Red Hat, Inc.
[    2.178348] fuse: init (API version 7.36)
[    2.254008] Key type asymmetric registered
[    2.255176] Asymmetric key parser 'x509' registered
[    2.257879] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 247)
[    2.259819] io scheduler mq-deadline registered
[    2.273403] crc32: CRC_LE_BITS = 64, CRC_BE BITS = 64
[    2.274357] crc32: self tests passed, processed 225944 bytes in 1110370 nsec
[    2.277259] crc32c: CRC_LE_BITS = 64
[    2.277929] crc32c: self tests passed, processed 225944 bytes in 771860 nsec
[    2.372622] crc32_combine: 8373 self tests passed
[    2.442695] crc32c_combine: 8373 self tests passed
[    2.475950] input: Power Button as /devices/LNXSYSTM:00/LNXPWRBN:00/input/input0
[    2.479088] ACPI: button: Power Button [PWRF]
[    2.513667] Serial: 8250/16550 driver, 32 ports, IRQ sharing enabled
[    2.523733] 00:04: ttyS0 at I/O 0x3f8 (irq = 4, base_baud = 115200) is a 16550A
[    2.742848] brd: module loaded
[    2.840636] loop: module loaded
[    3.090316] zram: Added device: zram0
[    3.135075] scsi host0: ata_piix
[    3.145005] scsi host1: ata_piix
[    3.153243] ata1: PATA max MWDMA2 cmd 0x1f0 ctl 0x3f6 bmdma 0xc040 irq 14
[    3.154507] ata2: PATA max MWDMA2 cmd 0x170 ctl 0x376 bmdma 0xc048 irq 15
[    3.165671] slram: not enough parameters.
[    3.175117] tun: Universal TUN/TAP device driver, 1.6
[    3.179578] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
[    3.184148] ehci-pci: EHCI PCI platform driver
[    3.187431] ehci-platform: EHCI generic platform driver
[    3.189183] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
[    3.191192] ohci-pci: OHCI PCI platform driver
[    3.193852] ohci-platform: OHCI generic platform driver
[    3.195178] uhci_hcd: USB Universal Host Controller Interface driver
[    3.203794] usbcore: registered new interface driver uas
[    3.204830] usbcore: registered new interface driver usb-storage
[    3.205790] usbcore: registered new interface driver ums-sddr09
[    3.206689] usbcore: registered new interface driver ums-sddr55
[    3.209246] UDC core: couldn't find an available UDC - added [g_ether] to list of pending drivers
[    3.212440] i8042: PNP: PS/2 Controller [PNP0303:KBD,PNP0f13:MOU] at 0x60,0x64 irq 1,12
[    3.227048] serio: i8042 KBD port at 0x60,0x64 irq 1
[    3.228371] serio: i8042 AUX port at 0x60,0x64 irq 12
[    3.240385] mousedev: PS/2 mouse device common for all mice
[    3.244422] i2c_dev: i2c /dev entries driver
[    3.265365] input: AT Translated Set 2 keyboard as /devices/platform/i8042/serio0/input/input1
[    3.275084] NET: Registered PF_INET6 protocol family
[    3.305805] Segment Routing with IPv6
[    3.307341] In-situ OAM (IOAM) with IPv6
[    3.310571] NET: Registered PF_PACKET protocol family
[    3.311994] NET: Registered PF_KEY protocol family
[    3.322883] 8021q: 802.1Q VLAN Support v1.8
[    3.326163] Key type dns_resolver registered
[    3.327276] ata1: found unknown device (class 0)
[    3.329634] IPI shorthand broadcast: enabled
[    3.338215] ata2: found unknown device (class 0)
[    3.345803] registered taskstats version 1
[    3.360321] ata1.00: ATA-7: QEMU HARDDISK, 2.5+, max UDMA/100
[    3.365434] ata1.00: 30375936 sectors, multi 16: LBA48 
[    3.370638] ata2.00: ATAPI: QEMU DVD-ROM, 2.5+, max UDMA/100
[    3.371540] Loading compiled-in X.509 certificates
[    3.379584] zswap: loaded using pool lzo/zbud
[    3.394922] Key type ._fscrypt registered
[    3.397636] Key type .fscrypt registered
[    3.398993] Key type fscrypt-provisioning registered
[    3.424098] scsi 0:0:0:0: Direct-Access     ATA      QEMU HARDDISK    2.5+ PQ: 0 ANSI: 5
[    3.448995] sd 0:0:0:0: Attached scsi generic sg0 type 0
[    3.466112] scsi 1:0:0:0: CD-ROM            QEMU     QEMU DVD-ROM     2.5+ PQ: 0 ANSI: 5
[    3.479417] sd 0:0:0:0: [sda] 30375936 512-byte logical blocks: (15.6 GB/14.5 GiB)
[    3.487988] sd 0:0:0:0: [sda] Write Protect is off
[    3.491319] sd 0:0:0:0: [sda] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
[    3.505128] sr 1:0:0:0: [sr0] scsi3-mmc drive: 4x/4x cd/rw xa/form2 tray
[    3.522272] cdrom: Uniform CD-ROM driver Revision: 3.20
[    3.570125] BIOS EDD facility v0.16 2004-Jun-25, 1 devices found
[    3.577768] Unstable clock detected, switching default tracing clock to "global"
[    3.577768] If you want to keep using the local clock, then add:
[    3.577768]   "trace_clock=local"
[    3.577768] on the kernel command line
[    3.632955] sr 1:0:0:0: Attached scsi generic sg1 type 5
[    3.668905] sd 0:0:0:0: [sda] Attached SCSI disk
[    4.104736] Freeing initrd memory: 161320K
[    4.122876] RAMDISK: squashfs filesystem found at block 0
[    4.123640] RAMDISK: Loading 161317KiB [1 disk] into ram disk... \
[    4.513943] /
[    4.929405] \
[    5.359663] /
[    5.887955] \
[    6.348914] /
[    6.830649] \
[    7.327574] /
[    7.840102] \
[    8.370751] /
[    8.907909] \
[    9.590564] /
[   10.225339] \
[   10.945615] /
[   11.483640] \
[   12.123357] /
[   12.780667] \
[   13.491963] /
[   14.219982] \
[   14.921094] /
[   15.674160] done.
[   21.989080] VFS: Mounted root (squashfs filesystem) readonly on device 1:0.
[   21.992757] devtmpfs: mounted
[   22.091705] Freeing unused kernel image (initmem) memory: 2016K
[   22.092702] Write protecting the kernel read-only data: 18432k
[   22.102664] Freeing unused kernel image (text/rodata gap) memory: 2040K
[   22.105285] Freeing unused kernel image (rodata/data gap) memory: 4K
[   22.460359] x86/mm: Checked W+X mappings: passed, no W+X pages found.
[   22.461313] Run /sbin/init as init process
[   23.896602] EXT4-fs (sda): recovery complete
[   23.902251] EXT4-fs (sda): mounted filesystem with ordered data mode. Quota mode: none.
Mounting...
[   24.094486] overlayfs: "xino" feature enabled using 32 upper inode bits.
Device file system
Change root !!
sh: cannot set terminal process group (124): Inappropriate ioctl for device
sh: no job control in this shell
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# [   29.881244] random: fast init done

sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# 
sh-5.1# ls
bin                   lib64                 sbin
boot                  libexec               share
dev                   linuxrc               sys
etc                   man                   tmp
home                  mnt                   usr
include               proc                  var
lib                   root                  x86_64-any-linux-gnu
sh-5.1#  
sh-5.1# 
sh-5.1# /etc/rc.init
       ___    __      __ _  _     _      _                  
      / _ \  / /     / /| || |   | |    (_)                 
__  _| (_) |/ /_    / /_| || |_  | |     _ _ __  _   ___  __
\ \/ /> _ <| '_ \  | '_ \__   _| | |    | | '_ \| | | \ \/ /
 >  <| (_) | (_) | | (_) | | |   | |____| | | | | |_| |>  < 
/_/\_\\___/ \___/   \___/  |_|   |______|_|_| |_|\__,_/_/\_\
                                                            


Network - VMware should use Host-only interface for this OS

[   46.620670] e1000: Intel(R) PRO/1000 Network Driver
[   46.621136] e1000: Copyright (c) 1999-2006 Intel Corporation.
[   47.896230] ACPI: \_SB_.LNKC: Enabled at IRQ 11
[   48.229071] e1000 0000:00:03.0 eth0: (PCI:33MHz:32-bit) 52:54:00:12:34:56
[   48.230569] e1000 0000:00:03.0 eth0: Intel(R) PRO/1000 Network Connection
udhcpc: started, v1.35.0
Setting IP address 0.0.0.0 on eth0
[   48.563707] 8021q: adding VLAN 0 to HW filter on device eth0
udhcpc: broadcasting discover
[   50.603506] e1000: eth0 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: RX
[   50.606794] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
udhcpc: broadcasting discover
udhcpc: broadcasting select for 10.0.2.15, server 10.0.2.2
udhcpc: lease of 10.0.2.15 obtained from 10.0.2.2, lease time 86400
Setting IP address 10.0.2.15 on eth0
Deleting routers
route: SIOCDELRT: No such process
Adding router 10.0.2.2
Recreating /etc/resolv.conf
 Adding DNS server 10.0.2.3
chpasswd: password for 'root' changed
Thu Nov 25 12:00:00 UTC 2021
TELNET daemon
sh-5.1# 
sh-5.1# 
sh-5.1# bash
bash: cannot set terminal process group (124): Inappropriate ioctl for device
bash: no job control in this shell
bash-5.1# 
bash-5.1# 
bash-5.1# ls -la
total 76
drwxrwxr-x    1 1000     1000          4096 Oct 14  2022 .
drwxrwxr-x    1 1000     1000          4096 Oct 14  2022 ..
-rw-------    1 root     root             6 Oct 14  2022 .bash_history
drwxr-xr-x    1 1000     1000          4096 Oct 12  2022 bin
drwxrwxr-x    2 root     root             3 Oct 12  2022 boot
drwxr-xr-x    6 root     root          3620 Nov 25 12:00 dev
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 etc
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 home
drwxr-xr-x    1 1000     1000          4096 Oct 13  2022 include
drwxr-xr-x    1 1000     1000          4096 Oct 12  2022 lib
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 lib64
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 libexec
lrwxrwxrwx    1 root     root            11 Oct 14  2022 linuxrc -> bin/busybox
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 man
drwxrwxr-x    2 root     root             3 Oct 12  2022 mnt
dr-xr-xr-x  152 root     root             0 Oct 14  2022 proc
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 root
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 sbin
drwxr-xr-x    1 1000     1000          4096 Oct 12  2022 share
dr-xr-xr-x   12 root     root             0 Oct 14  2022 sys
lrwxrwxrwx    1 root     root            12 Oct 14  2022 tmp -> /var/tmp/tmp
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 usr
drwxrwxrwt   10 root     root           200 Oct 14  2022 var
drwxrwxr-x    4 1000     1000          4096 Oct 12  2022 x86_64-any-linux-gnu
bash-5.1# 
bash-5.1# 
bash-5.1# ifconfig
eth0      Link encap:Ethernet  HWaddr 52:54:00:12:34:56  
          inet addr:10.0.2.15  Bcast:10.0.2.255  Mask:255.255.255.0
          inet6 addr: fe80::5054:ff:fe12:3456/64 Scope:Link
          inet6 addr: fec0::5054:ff:fe12:3456/64 Scope:Site
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:3 errors:0 dropped:0 overruns:0 frame:0
          TX packets:9 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:742 (742.0 B)  TX bytes:1286 (1.2 KiB)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

bash-5.1# ls -la
total 76
drwxrwxr-x    1 1000     1000          4096 Oct 14  2022 .
drwxrwxr-x    1 1000     1000          4096 Oct 14  2022 ..
-rw-------    1 root     root             6 Oct 14  2022 .bash_history
drwxr-xr-x    1 1000     1000          4096 Oct 12  2022 bin
drwxrwxr-x    2 root     root             3 Oct 12  2022 boot
drwxr-xr-x    6 root     root          3620 Nov 25 12:00 dev
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 etc
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 home
drwxr-xr-x    1 1000     1000          4096 Oct 13  2022 include
drwxr-xr-x    1 1000     1000          4096 Oct 12  2022 lib
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 lib64
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 libexec
lrwxrwxrwx    1 root     root            11 Oct 14  2022 linuxrc -> bin/busybox
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 man
drwxrwxr-x    2 root     root             3 Oct 12  2022 mnt
dr-xr-xr-x  152 root     root             0 Oct 14  2022 proc
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 root
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 sbin
drwxr-xr-x    1 1000     1000          4096 Oct 12  2022 share
dr-xr-xr-x   12 root     root             0 Oct 14  2022 sys
lrwxrwxrwx    1 root     root            12 Oct 14  2022 tmp -> /var/tmp/tmp
drwxrwxr-x    1 1000     1000          4096 Oct 12  2022 usr
drwxrwxrwt   10 root     root           200 Oct 14  2022 var
drwxrwxr-x    4 1000     1000          4096 Oct 12  2022 x86_64-any-linux-gnu
bash-5.1# 
bash-5.1# 
bash-5.1# mount
overlay on / type overlay (rw,relatime,lowerdir=/disk,upperdir=/mnt/usr,workdir=/mnt/work)
proc on /proc type proc (rw,relatime)
ram on /var type tmpfs (rw,relatime,size=32768k)
sys on /sys type sysfs (rw,relatime)
devfs on /dev type devtmpfs (rw,relatime,size=1936920k,nr_inodes=484230,mode=755)
devpts on /dev/pts type devpts (rw,relatime,mode=600,ptmxmode=000)
bash-5.1# 
bash-5.1# 


  ```

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

# [3] Booting Image Building

```#!/bin/sh

  # make TBOARD=vm board ramdisk
```


- Unlikely with the case of raspberry PI, we will get "output.iso" file under boards/vm .


# [4] Running/Stopping "output.iso"

- We can run this file as below.
```#!/bin/sh

  # make TBOARD=vm vmrun


  If you want to explicitly specify USB stick device, use the following option. 


  # make TBOARD=vm EXT4DISK_MNTDIR=/dev/sde1 vmrun 

```

- We can stop running QEMU emulator.
```#!/bin/sh

  # make TBOARD=vm vmstop
```


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
