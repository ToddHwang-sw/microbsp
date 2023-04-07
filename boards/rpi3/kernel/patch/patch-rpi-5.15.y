diff -uNr linux-rpi-5.15.y-orig/arch/arm64/Makefile linux-rpi-5.15.y/arch/arm64/Makefile
--- linux-rpi-5.15.y-orig/arch/arm64/Makefile	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/arch/arm64/Makefile	2023-04-06 20:13:02.619304932 -0700
@@ -93,6 +93,9 @@
 asm-arch := armv8.5-a
 endif
 
+### MicroBSP 
+asm-arch := armv8-a
+
 ifdef asm-arch
 KBUILD_CFLAGS	+= -Wa,-march=$(asm-arch) \
 		   -DARM64_ASM_ARCH='"$(asm-arch)"'
diff -uNr linux-rpi-5.15.y-orig/drivers/base/firmware_loader/main.c linux-rpi-5.15.y/drivers/base/firmware_loader/main.c
--- linux-rpi-5.15.y-orig/drivers/base/firmware_loader/main.c	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/drivers/base/firmware_loader/main.c	2023-04-06 20:11:50.483011797 -0700
@@ -471,6 +471,9 @@
 	"/lib/firmware/updates",
 	"/lib/firmware/" UTS_RELEASE,
 	"/lib/firmware"
+#ifdef TODD_KERNEL
+    , "/ovr/lib/firmware"
+#endif
 };
 
 /*
diff -uNr linux-rpi-5.15.y-orig/Makefile linux-rpi-5.15.y/Makefile
--- linux-rpi-5.15.y-orig/Makefile	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/Makefile	2023-04-06 20:11:50.483011797 -0700
@@ -433,7 +433,7 @@
 HOSTPKG_CONFIG	= pkg-config
 
 export KBUILD_USERCFLAGS := -Wall -Wmissing-prototypes -Wstrict-prototypes \
-			      -O2 -fomit-frame-pointer -std=gnu89
+			      -O2 -fomit-frame-pointer -std=gnu89 -DTODD_KERNEL
 export KBUILD_USERLDFLAGS :=
 
 KBUILD_HOSTCFLAGS   := $(KBUILD_USERCFLAGS) $(HOST_LFS_CFLAGS) $(HOSTCFLAGS)
@@ -516,7 +516,7 @@
 		   -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE \
 		   -Werror=implicit-function-declaration -Werror=implicit-int \
 		   -Werror=return-type -Wno-format-security \
-		   -std=gnu89
+		   -std=gnu89 -DTODD_KERNEL
 KBUILD_CPPFLAGS := -D__KERNEL__
 KBUILD_AFLAGS_KERNEL :=
 KBUILD_CFLAGS_KERNEL :=
diff -uNr linux-rpi-5.15.y-orig/net/wireless/sme.c linux-rpi-5.15.y/net/wireless/sme.c
--- linux-rpi-5.15.y-orig/net/wireless/sme.c	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/net/wireless/sme.c	2023-04-06 21:29:41.884042817 -0700
@@ -753,7 +753,9 @@
 		return;
 	}
 
-	if (WARN_ON(!cr->bss))
+	/* Todd */
+	/* if (WARN_ON(!cr->bss)) */
+	if (!cr->bss)
 		return;
 
 	wdev->current_bss = bss_from_pub(cr->bss);
