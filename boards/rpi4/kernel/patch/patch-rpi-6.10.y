diff -uNr linux-rpi-6.10.y-orig/drivers/base/firmware_loader/main.c linux-rpi-6.10.y/drivers/base/firmware_loader/main.c
--- linux-rpi-6.10.y-orig/drivers/base/firmware_loader/main.c	2024-10-15 01:17:47.000000000 -0700
+++ linux-rpi-6.10.y/drivers/base/firmware_loader/main.c	2026-02-28 20:30:40.332395307 -0800
@@ -474,7 +474,10 @@
 	"/lib/firmware/updates/" UTS_RELEASE,
 	"/lib/firmware/updates",
 	"/lib/firmware/" UTS_RELEASE,
-	"/lib/firmware"
+	"/lib/firmware",
+	/* MBSP */
+	"/ovr/lib/firmware/" UTS_RELEASE,
+	"/ovr/lib/firmware"
 };
 
 /*
