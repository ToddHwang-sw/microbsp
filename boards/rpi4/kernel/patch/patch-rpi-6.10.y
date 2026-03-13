diff -uNr linux-rpi-6.10.y-orig/drivers/base/firmware_loader/main.c linux-rpi-6.10.y/drivers/base/firmware_loader/main.c
--- linux-rpi-6.10.y-orig/drivers/base/firmware_loader/main.c	2024-10-15 01:17:47.000000000 -0700
+++ linux-rpi-6.10.y/drivers/base/firmware_loader/main.c	2026-03-03 10:04:22.026014735 -0800
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
diff -uNr linux-rpi-6.10.y-orig/net/wireless/sme.c linux-rpi-6.10.y/net/wireless/sme.c
--- linux-rpi-6.10.y-orig/net/wireless/sme.c	2024-10-15 01:17:47.000000000 -0700
+++ linux-rpi-6.10.y/net/wireless/sme.c	2026-03-03 10:05:13.972556734 -0800
@@ -844,7 +844,8 @@
 		return;
 	}
 
-	if (WARN_ON(bss_not_found)) {
+	/* if (WARN_ON(bss_not_found)) { */
+	if (bss_not_found) {
 		cfg80211_connect_result_release_bsses(wdev, cr);
 		return;
 	}
diff -uNr linux-rpi-6.10.y-orig/arch/arm/boot/dts/broadcom/bcm2711-rpi-4-b.dts linux-rpi-6.10.y/arch/arm/boot/dts/broadcom/bcm2711-rpi-4-b.dts
--- linux-rpi-6.10.y-orig/arch/arm/boot/dts/broadcom/bcm2711-rpi-4-b.dts	2024-10-15 01:17:47.000000000 -0700
+++ linux-rpi-6.10.y/arch/arm/boot/dts/broadcom/bcm2711-rpi-4-b.dts	2026-03-12 09:19:40.764383148 -0700
@@ -267,6 +267,11 @@
 	status = "okay";
 };
 
+/* Todd - spi / It is required due to dtparam spi=on is interrupted by U-boot. */
+&spi0 {
+	status = "okay";
+};
+
 &vc4 {
 	status = "okay";
 };
