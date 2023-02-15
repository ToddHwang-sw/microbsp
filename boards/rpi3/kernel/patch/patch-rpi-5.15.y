diff -uNr linux-rpi-5.15.y-original/drivers/base/firmware_loader/main.c linux-rpi-5.15.y/drivers/base/firmware_loader/main.c
--- linux-rpi-5.15.y-original/drivers/base/firmware_loader/main.c	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/drivers/base/firmware_loader/main.c	2023-02-14 09:35:12.000536901 -0800
@@ -471,6 +471,9 @@
 	"/lib/firmware/updates",
 	"/lib/firmware/" UTS_RELEASE,
 	"/lib/firmware"
+#ifdef TODD_KERNEL
+    , "/ovr/lib/firmware"
+#endif
 };
 
 /*
diff -uNr linux-rpi-5.15.y-original/Makefile linux-rpi-5.15.y/Makefile
--- linux-rpi-5.15.y-original/Makefile	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/Makefile	2023-02-14 09:36:37.719679558 -0800
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
