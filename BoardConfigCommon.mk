#
# Copyright 2014 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

COMMON_PATH := device/samsung/s5e9945

# Architecture
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv9-2a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_VARIANT := cortex-a76

# DTS
BOARD_DTB_CFG := $(COMMON_PATH)/configs/kernel/dtb.cfg
BOARD_INCLUDE_DTB_IN_BOOTIMG := true
BOARD_INCLUDE_RECOVERY_DTBO := true
BOARD_KERNEL_SEPARATED_DTBO := true

# Display
TARGET_USES_VULKAN := true

# Filesystem
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_PRODUCTIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SYSTEM_DLKMIMAGE_FILE_SYSTEM_TYPE := erofs
BOARD_SYSTEM_EXTIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := erofs
BOARD_VENDOR_DLKMIMAGE_FILE_SYSTEM_TYPE := erofs
TARGET_COPY_OUT_PRODUCT := product
TARGET_COPY_OUT_SYSTEM_DLKM := system_dlkm
TARGET_COPY_OUT_SYSTEM_EXT := system_ext
TARGET_COPY_OUT_VENDOR := vendor
TARGET_COPY_OUT_VENDOR_DLKM := vendor_dlkm

-include vendor/lineage/config/BoardConfigReservedSize.mk

# Firmware
TARGET_NO_BOOTLOADER := true

# Kernel
BOARD_BOOT_HEADER_VERSION := 4
BOARD_BOOTCONFIG := androidboot.serialconsole=0
BOARD_KERNEL_IMAGE_NAME := Image
BOARD_MKBOOTIMG_ARGS := --header_version $(BOARD_BOOT_HEADER_VERSION)
BOARD_MKBOOTIMG_INIT_ARGS := $(BOARD_MKBOOTIMG_ARGS)
TARGET_KERNEL_ADDITIONAL_FLAGS := \
    DTC_FLAGS=-@ \
    KCFLAGS=-D__ANDROID_COMMON_KERNEL__ \
    TARGET_SOC=s5e9945
TARGET_KERNEL_CONFIG := \
    $(shell KCONFIG_CONFIG=kernel/samsung/s5e9945/arch/arm64/configs/erd9945_u_gki_defconfig \
    kernel/samsung/s5e9945/scripts/kconfig/merge_config.sh -m -r \
    kernel/samsung/s5e9945/arch/arm64/configs/gki_defconfig \
    kernel/samsung/s5e9945/arch/arm64/configs/s5e9945-base_defconfig \
    kernel/samsung/s5e9945/arch/arm64/configs/s5e9945-bazel_defconfig \
    kernel/samsung/s5e9945/arch/arm64/configs/s5e9945_user.cfg \
    kernel/samsung/s5e9945/arch/arm64/configs/s5e9945-user_defconfig \
    1>/dev/null; echo erd9945_u_gki_defconfig)
TARGET_KERNEL_NO_GCC := true
TARGET_KERNEL_SOURCE := kernel/samsung/s5e9945
TARGET_KERNEL_CLANG_VERSION := r522817

# Modules
BOARD_RECOVERY_RAMDISK_KERNEL_MODULES_LOAD := $(shell cat $(COMMON_PATH)/configs/kernel/modules/ramdisk) $(BOARD_RECOVERY_RAMDISK_KERNEL_MODULES_LOAD)
BOARD_SYSTEM_KERNEL_MODULES_LOAD := $(shell cat $(COMMON_PATH)/configs/kernel/modules/system)
BOARD_VENDOR_KERNEL_MODULES_LOAD := kiwi_v2.ko sec_debug_ssld_info.ko cfg80211.ko
BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD := $(BOARD_RECOVERY_RAMDISK_KERNEL_MODULES_LOAD)
BOOT_KERNEL_MODULES := $(BOARD_RECOVERY_RAMDISK_KERNEL_MODULES_LOAD)
RECOVERY_KERNEL_MODULES := $(BOARD_RECOVERY_RAMDISK_KERNEL_MODULES_LOAD)
SYSTEM_KERNEL_MODULES := $(BOARD_SYSTEM_KERNEL_MODULES_LOAD)

# Partitions - Classic
BOARD_BOOTIMAGE_PARTITION_SIZE := 67108864
BOARD_CACHEIMAGE_PARTITION_SIZE := 367001600
BOARD_DTBOIMG_PARTITION_SIZE := 8388608
BOARD_FLASH_BLOCK_SIZE := 4096
BOARD_INIT_BOOT_IMAGE_PARTITION_SIZE := 16777216
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 100663296
BOARD_ROOT_EXTRA_FOLDERS := efs
BOARD_SUPER_PARTITION_GROUPS := samsung_dynamic_partitions
BOARD_SUPER_PARTITION_SIZE := 12981370880
BOARD_USES_METADATA_PARTITION := true
BOARD_VENDOR_BOOTIMAGE_PARTITION_SIZE := $(BOARD_BOOTIMAGE_PARTITION_SIZE)

# Partitions - Dynamic
BOARD_SAMSUNG_DYNAMIC_PARTITIONS_PARTITION_LIST := \
    product \
    system \
    system_dlkm \
    system_ext \
    vendor \
    vendor_dlkm
BOARD_SAMSUNG_DYNAMIC_PARTITIONS_SIZE := $(shell echo $$(( $(BOARD_SUPER_PARTITION_SIZE) - 4 * 1024**2 )))

# Platform
BOARD_VENDOR := samsung
TARGET_BOARD_PLATFORM := erd9945
TARGET_BOOTLOADER_BOARD_NAME := s5e9945
TARGET_SOC := s5e9945

# Properties
TARGET_PRODUCT_PROP += $(COMMON_PATH)/configs/props/product.prop
TARGET_VENDOR_PROP += $(COMMON_PATH)/configs/props/vendor.prop

# RIL
 $(call soong_config_set,cbd,protocol,sipc)

# Ramdisks
BOARD_RAMDISK_USE_LZ4 := true
BOARD_VENDOR_RAMDISK_FRAGMENTS := dlkm
BOARD_VENDOR_RAMDISK_FRAGMENT.dlkm.MKBOOTIMG_ARGS := --ramdisk_type DLKM

# Recovery
BOARD_RECOVERY_MKBOOTIMG_ARGS := --header_version 2 --cmdline ""
TARGET_RECOVERY_FSTAB_GENRULE := gen_fstab_s5e9945_recovery
TARGET_RECOVERY_PIXEL_FORMAT := RGBX_8888
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true

# Releasetools
TARGET_RELEASETOOLS_EXTENSIONS := $(COMMON_PATH)

# RIL
ENABLE_VENDOR_RIL_SERVICE := true

# SELinux
include device/samsung_slsi/sepolicy/sepolicy.mk

# SELinux
BOARD_VENDOR_SEPOLICY_DIRS += $(COMMON_PATH)/sepolicy/vendor
BOARD_SEPOLICY_TEE_FLAVOR := teegris
include device/samsung_slsi/sepolicy/sepolicy.mk

# Security
VENDOR_SECURITY_PATCH := 2025-08-01

# USB
$(call soong_config_set,samsungUsbGadgetVars,gadget_name,17900000.dwc3)

# Verified Boot
BOARD_AVB_ENABLE := true
BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS := --flags 3
BOARD_AVB_RECOVERY_ALGORITHM := NONE
BOARD_AVB_RECOVERY_KEY_PATH := external/avb/test/data/testkey_rsa4096.pem
BOARD_AVB_RECOVERY_ROLLBACK_INDEX := 0
BOARD_AVB_RECOVERY_ROLLBACK_INDEX_LOCATION := 1

# VINTF
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := \
    $(COMMON_PATH)/configs/vintf/compatibility_matrix.device.xml \
    hardware/samsung/vintf/samsung_framework_compatibility_matrix.xml \
    vendor/lineage/config/device_framework_matrix.xml
DEVICE_MANIFEST_FILE := $(COMMON_PATH)/configs/vintf/manifest.xml

# Wi-Fi
BOARD_WLAN_DEVICE                             := qcwcn
WIFI_HAL_INTERFACE_COMBINATIONS               := {{{STA}, 1}, {{AP}, 1}}, {{{STA}, 1}, {{P2P, NAN}, 1}}, {{{AP}, 2}}, {{{STA}, 2}}
BOARD_WPA_SUPPLICANT_DRIVER                   := NL80211
BOARD_HOSTAPD_DRIVER                          := NL80211
BOARD_HOSTAPD_CONFIG_80211W_MFP_OPTIONAL      := true
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true
WIFI_FEATURE_HOSTAPD_11AX                     := true
WPA_SUPPLICANT_VERSION                        := VER_0_8_X
