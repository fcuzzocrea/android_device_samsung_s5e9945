#
# SPDX-FileCopyrightText: 2025 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

COMMON_PATH := device/samsung/s5e9945

# A/B
AB_OTA_UPDATER := false

# Additional native libraries
PRODUCT_PACKAGES += \
    public.libraries.txt

# Audio
PRODUCT_PACKAGES += \
    android.hardware.audio.effect@7.0-impl \
    android.hardware.audio.service \
    android.hardware.audio@7.1-impl \
    android.hardware.bluetooth.audio-impl \
    android.hardware.soundtrigger@2.3-impl \
    audio.bluetooth.default \
    audio.r_submix.default \
    audio.usbv2.default \
    audio_board_info.xml \
    audio_effects.xml \
    audio_policy_configuration.xml \
    mixer_usb_default.xml \
    mixer_usb_gray.xml \
    mixer_usb_white.xml \
    privapp-permissions-hotword.xml \
    SamsungDAP

PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_with_le_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_with_le_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml

TARGET_EXCLUDES_AUDIOFX := true

# Camera
PRODUCT_PACKAGES += \
    libepicoperator \
    libhypervintf \
    libhwjpeg \
    libsensorndkbridge

# Cgroup and task_profiles
PRODUCT_PACKAGES += \
    cgroups.json \
    task_profiles.s5e9945.json

# Charger
PRODUCT_PACKAGES += \
    charger_res_images_vendor

# Codec2
PRODUCT_PACKAGES += \
    samsung.hardware.media.c2@1.2-service \
    libExynosC2H264Dec \
    libExynosC2H264Enc \
    libExynosC2HevcDec \
    libExynosC2HevcEnc \
    libExynosC2Vp8Dec \
    libExynosC2Vp8Enc \
    libExynosC2Vp9Dec \
    libExynosC2Vp9Enc \
    libExynosC2Av1Dec

PRODUCT_PACKAGES += \
    codec2.vendor.base.policy \
    codec2.vendor.ext.policy

# DRM
PRODUCT_PACKAGES += \
    com.android.hardware.drm.clearkey

# Dynamic Partitions
PRODUCT_USE_DYNAMIC_PARTITIONS := true

# Fastbootd
PRODUCT_PACKAGES += \
    fastbootd

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint-service.samsung \
    init.fingerprint.rc \
    init.udfps.rc

# Graphics
PRODUCT_PACKAGES += \
    android.hardware.composer.hwc3-service.slsi \
    libdrm_sgpu \
    libexynosgraphicbuffer_public \
    libion_exynos

PRODUCT_PACKAGES += \
    calib_data_atc.xml \
    calib_data_bypass.xml \
    calib_data_colormode0.xml \
    calib_data_colortemp.xml \
    calib_data_eyetemp.xml \
    calib_data_rgbgain.xml \
    calib_data_sharpness.xml \
    calib_data_skincolor.xml \
    calib_data_whitepoint.xml \
    DQE_coef_data.xml

PRODUCT_COPY_FILES += \
    vendor/samsung/s5e9945/proprietary/recovery/root/lib/firmware/sgpu/vangogh_lite_unified_evt1.bin:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/lib/firmware/sgpu/vangogh_lite_unified_evt1.bin

# GNSS
PRODUCT_PACKAGES += \
    init.gps.rc \
    init.gps.sh.rc \
    gps.cfg

# Health
PRODUCT_PACKAGES += \
    android.hardware.health-service.samsung \
    android.hardware.health-service.samsung-recovery

# Hermes
PRODUCT_PACKAGES += \
    hermesd.rc

# Init
PRODUCT_PACKAGES += \
    fstab.s5e9945 \
    fstab.s5e9945.vendor_ramdisk \
    init.s5e9945.rc \
    ueventd.s5e9945.rc

# Kernel
PRODUCT_SET_DEBUGFS_RESTRICTIONS := true
PRODUCT_ENABLE_UFFD_GC := true

# Kernel Modules
PRODUCT_PACKAGES += \
    linker.vendor_ramdisk \
    null \
    toolbox.vendor_ramdisk

# Lineage Health
PRODUCT_PACKAGES += \
    vendor.lineage.health-service.default

# Livedisplay
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay-service.s5e9945

# Media
PRODUCT_PACKAGES += \
    media_codecs_c2.xml \
    media_codecs_performance_c2.xml \
    media_profiles_V1_0.xml

# Memtrack
PRODUCT_PACKAGES += \
    android.hardware.memtrack-service.exynos

# NFC
PRODUCT_PACKAGES += \
    android.hardware.nfc-service.sec \
    com.android.nfc_extras \
    init.nfc.samsung.rc \
    libnfc-sec-vendor.conf \
    libse-gto-hal.conf \
    NfcOverlay

# Overlays
PRODUCT_PACKAGES += \
    ApertureOverlayCommon \
    CarrierConfigOverlayCommon \
    FrameworkResOverlayCommon \
    LineageDialerOverlayCommon \
    LineageSDKOverlayCommon \
    LineageSettingsOverlayCommon \
    SettingsOverlayCommon \
    SystemUIOverlayCommon

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.audio.pro.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.pro.prebuilt.xml \
    frameworks/native/data/etc/android.hardware.camera.ar.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.ar.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.keystore.app_attest_key.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.keystore.app_attest_key.prebuilt.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.uicc.prebuilt.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.prebuilt.xml \
    frameworks/native/data/etc/android.hardware.strongbox_keystore.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.strongbox_keystore.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.prebuilt.xml \
    frameworks/native/data/etc/android.hardware.wifi.aware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.aware.xml \
    frameworks/native/data/etc/android.hardware.wifi.rtt.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.rtt.xml \
    frameworks/native/data/etc/android.software.app_widgets.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.app_widgets.xml \
    frameworks/native/data/etc/android.software.freeform_window_management.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.freeform_window_management.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.prebuilt.xml \
    frameworks/native/data/etc/android.software.picture_in_picture.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.picture_in_picture.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.android.nfc_extras.xml

PRODUCT_PACKAGES += \
    android.hardware.audio.low_latency.prebuilt.xml \
    android.hardware.bluetooth.prebuilt.xml \
    android.hardware.bluetooth_le.prebuilt.xml \
    android.hardware.camera.concurrent.prebuilt.xml \
    android.hardware.camera.flash-autofocus.prebuilt.xml \
    android.hardware.camera.front.prebuilt.xml \
    android.hardware.camera.full.prebuilt.xml \
    android.hardware.camera.raw.prebuilt.xml \
    android.hardware.fingerprint.prebuilt.xml \
    android.hardware.hardware_keystore_V3.xml \
    android.hardware.location.gps.prebuilt.xml \
    android.hardware.nfc.ese.prebuilt.xml \
    android.hardware.nfc.hce.prebuilt.xml \
    android.hardware.nfc.hcef.prebuilt.xml \
    android.hardware.nfc.prebuilt.xml \
    android.hardware.se.omapi.ese.prebuilt.xml \
    android.hardware.se.omapi.uicc.prebuilt.xml \
    android.hardware.sensor.accelerometer.prebuilt.xml \
    android.hardware.sensor.barometer.prebuilt.xml \
    android.hardware.sensor.compass.prebuilt.xml \
    android.hardware.sensor.gyroscope.prebuilt.xml \
    android.hardware.sensor.hifi_sensors.prebuilt.xml \
    android.hardware.sensor.light.prebuilt.xml \
    android.hardware.sensor.proximity.prebuilt.xml \
    android.hardware.sensor.stepcounter.prebuilt.xml \
    android.hardware.sensor.stepdetector.prebuilt.xml \
    android.hardware.telephony.gsm.prebuilt.xml \
    android.hardware.telephony.ims.prebuilt.xml \
    android.hardware.telephony.satellite.prebuilt.xml \
    android.hardware.usb.accessory.prebuilt.xml \
    android.hardware.usb.host.prebuilt.xml \
    android.hardware.vulkan.compute-0.prebuilt.xml \
    android.hardware.vulkan.level-1.prebuilt.xml \
    android.hardware.vulkan.version-1_3.prebuilt.xml \
    android.hardware.wifi.direct.prebuilt.xml \
    android.hardware.wifi.passpoint.prebuilt.xml \
    android.hardware.wifi.prebuilt.xml \
    android.software.ipsec_tunnels.prebuilt.xml \
    android.software.opengles.deqp.level-latest.prebuilt.xml \
    android.software.sip.voip.prebuilt.xml \
    android.software.verified_boot.prebuilt.xml \
    android.software.vulkan.deqp.level-2023-03-01.prebuilt.xml \
    com.nxp.mifare.prebuilt.xml \
    handheld_core_hardware.prebuilt.xml

# pKVM
$(call inherit-product, packages/modules/Virtualization/apex/product_packages.mk)

# Power
PRODUCT_PACKAGES += \
    android.hardware.power-service.pixel-libperfmgr \
    powerhint.json

# PowerShare
PRODUCT_PACKAGES += \
    vendor.lineage.powershare-service.samsung

# Recovery
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/init/etc/init/init.recovery.s5e9945.rc:recovery/root/init.recovery.s5e9945.rc

# RIL
PRODUCT_PACKAGES += \
    cbd \
    init.baseband.rc \
    init.vendor.onebinary.rc \
    init.vendor.rilcommon.rc \
    vendor.samsung.rild.rc \
    secril_config_svc \
    sehradiomanager \
    sehradiomanager.conf

# SamsungDoze
PRODUCT_PACKAGES += \
    SamsungDoze

# SBWC
 PRODUCT_PACKAGES += \
     vendor.samsung_slsi.hardware.SbwcDecompService@1.0-service \
     libsbwchelper

# Secure Element
PRODUCT_PACKAGES += \
    android.hardware.secure_element-service.thales-st33

# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors-service.samsung-multihal \
    hals.conf \
    init.sensorhub.rc

# Shipping API Levels
PRODUCT_SHIPPING_API_LEVEL := 34

# Soong Namespaces
PRODUCT_SOONG_NAMESPACES += \
    bootable/deprecated-ota \
    hardware/google/interfaces \
    hardware/google/pixel \
    hardware/qcom-caf/wlan \
    hardware/samsung \
    hardware/samsung_slsi-linaro/exynos/cpboot_v3 \
    $(COMMON_PATH)

# Storage
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

# Teegris
PRODUCT_PACKAGES += \
    teegris.rc \
    teegris_tui.rc

# Thermal
PRODUCT_PACKAGES += \
    android.hardware.thermal-service.pixel \
    thermal_info_config.json \
    thermal_symlinks

# Thethering
PRODUCT_PACKAGES += \
    TetheringOverlay

# Touch HAL
PRODUCT_PACKAGES += \
    vendor.lineage.touch-service.samsung

# USB
PRODUCT_PACKAGES += \
    android.hardware.usb-service.samsung \
    android.hardware.usb.gadget-service.samsung \
    init.s5e9945.usb.rc

# Vendor service manager
PRODUCT_PACKAGES += \
    vndservicemanager

# Vibrator
PRODUCT_PACKAGES += \
    android.hardware.vibrator-service.samsung

# Wi-Fi
PRODUCT_PACKAGES += \
    android.hardware.wifi-service \
    hostapd \
    indoorchannel.info \
    init.insmod.sh \
    libcld80211 \
    p2p_supplicant_overlay.conf \
    wifi_qcom_ap_exynos.rc \
    wifi_sec.rc \
    wpa_supplicant \
    wpa_supplicant.conf \
    wpa_supplicant_overlay.conf

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/wifi/qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/kiwi_v2/qcom_cfg.ini \
    $(COMMON_PATH)/configs/wifi/wlan-connection-roaming.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan-connection-roaming.ini \
    $(COMMON_PATH)/configs/wifi/wlan-connection-roaming-backup.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan-connection-roaming-backup.ini

# Setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-xhdpi-6144-dalvik-heap.mk)

# Call Samsung LSI board support package makefiles
include hardware/samsung_slsi-linaro/config/BoardConfig9945.mk
$(call inherit-product, hardware/samsung_slsi-linaro/graphics/base/hwcomposer_property.mk)
$(call inherit-product, hardware/samsung_slsi-linaro/config/config.mk)

# Call the proprietary setup
$(call inherit-product, vendor/samsung/s5e9945/s5e9945-vendor.mk)
