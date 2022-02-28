diff -uNr linux-rpi-5.10.y/drivers/base/firmware_loader/main.c linux-rpi-5.10.y-new/drivers/base/firmware_loader/main.c
--- linux-rpi-5.10.y/drivers/base/firmware_loader/main.c	2021-11-05 08:51:13.000000000 -0700
+++ linux-rpi-5.10.y-new/drivers/base/firmware_loader/main.c	2021-11-06 22:05:58.153264278 -0700
@@ -469,6 +469,9 @@
 	"/lib/firmware/updates",
 	"/lib/firmware/" UTS_RELEASE,
 	"/lib/firmware"
+#ifdef TODD_KERNEL
+	, "/ovr/lib/firmware" 
+#endif
 };
 
 /*
diff -uNr linux-rpi-5.10.y/Makefile linux-rpi-5.10.y-new/Makefile
--- linux-rpi-5.10.y/Makefile	2021-11-05 08:51:13.000000000 -0700
+++ linux-rpi-5.10.y-new/Makefile	2021-11-06 22:54:33.550033649 -0700
@@ -416,7 +416,7 @@
 endif
 
 export KBUILD_USERCFLAGS := -Wall -Wmissing-prototypes -Wstrict-prototypes \
-			      -O2 -fomit-frame-pointer -std=gnu89
+			      -O2 -fomit-frame-pointer -std=gnu89 
 export KBUILD_USERLDFLAGS :=
 
 KBUILD_HOSTCFLAGS   := $(KBUILD_USERCFLAGS) $(HOST_LFS_CFLAGS) $(HOSTCFLAGS)
@@ -497,7 +497,7 @@
 		   -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE \
 		   -Werror=implicit-function-declaration -Werror=implicit-int \
 		   -Werror=return-type -Wno-format-security \
-		   -std=gnu89
+		   -std=gnu89 -DTODD_KERNEL
 KBUILD_CPPFLAGS := -D__KERNEL__
 KBUILD_AFLAGS_KERNEL :=
 KBUILD_CFLAGS_KERNEL :=
