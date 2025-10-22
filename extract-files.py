#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2025 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from shutil import (
    copytree,
    ignore_patterns
)
import os
import sys
import tempfile
from zipfile import ZipFile

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/samsung/s5e9945',
    'hardware/samsung',
]


blob_fixups: blob_fixups_user_type = {
    'vendor/bin/hermesd': blob_fixup()
        .binary_regex_replace(b'security.securehw.available', b'vendor.securehw.available\x00\x00')
        .binary_regex_replace(b'security.securenvm.available', b'vendor.securenvm.available\x00\x00'),
    'vendor/bin/hw/android.hardware.graphics.composer3-service.exynos': blob_fixup()
        .replace_needed(
            'android.hardware.graphics.composer@2.1-resources.so',
            'android.hardware.graphics.composer@2.1-resources_samsung.so')
        .replace_needed(
            'android.hardware.graphics.composer@2.2-resources.so',
            'android.hardware.graphics.composer@2.2-resources_samsung.so'),
    'vendor/bin/hw/gps.sh': blob_fixup()
        .regex_replace('apex/com.samsung.android.gnss.lsi.root', 'vendor')
        .regex_replace('bin/gpsd_K43', 'bin/hw/gpsd_K43')
        .regex_replace('etc/firmware', 'firmware/gnss')
        .regex_replace('etc/cfg', 'etc/gnss'),
    'vendor/etc/init/android.hardware.security.keymint-service.samsung.rc': blob_fixup()
        .regex_replace('-service', '-service.samsung'),
    (
        'vendor/etc/init/vendor.samsung.hardware.gnss-service.rc'
    ): blob_fixup()
        .regex_replace('apex/com.samsung.android.gnss.lsi.root', 'vendor')
        .regex_replace('bin', 'bin/hw'),
    'vendor/etc/init/vendor.samsung.hardware.camera.provider-service_64.rc': blob_fixup()
        .regex_replace('vendor_secdir w', 'w')
        .regex_replace('vendor_secdir', 'camera'),
    'vendor/etc/vintf/manifest/sec_c2_manifest_default0_1_2.xml': blob_fixup()
        .regex_replace('.*t0.*\n', ''),
    'vendor/lib64/android.hardware.graphics.composer@2.2-resources_samsung.so': blob_fixup()
        .replace_needed(
            'android.hardware.graphics.composer@2.1-resources.so',
            'android.hardware.graphics.composer@2.1-resources_samsung.so'),
    (
        'vendor/lib64/hw/audio.primary.s5e9945.so',
        'vendor/lib64/libaudioproxy2.so',
        'vendor/lib64/libaudioparamupdate.so',
    ): blob_fixup()
        .replace_needed('libaudioroute.so', 'libaudioroute_samsung.so')
        .replace_needed('libtinyalsa.so', 'libtinyalsa_samsung.so'),
    'vendor/lib64/hw/vulkan.samsung.so': blob_fixup()
        .clear_symbol_version('AHardwareBuffer_acquire')
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_getId')
        .clear_symbol_version('AHardwareBuffer_getNativeHandle')
        .clear_symbol_version('AHardwareBuffer_release'),
    'vendor/lib64/libOpenCL.so': blob_fixup()
        .clear_symbol_version('AHardwareBuffer_acquire')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_getNativeHandle')
        .clear_symbol_version('AHardwareBuffer_release'),
    'vendor/lib64/lib_profiler.so': blob_fixup()
        .replace_needed('libprotobuf-cpp-full-21.7.so', 'libprotobuf-cpp-full-21.12.so'),
    (
        'vendor/lib64/libalsautils_sec.so',
        'vendor/lib64/libaudioroute_samsung.so',
    ): blob_fixup()
        .replace_needed('libtinyalsa.so', 'libtinyalsa_samsung.so'),
    'vendor/lib64/libexynosgraphicbuffer.so': blob_fixup()
        .add_needed('libshim_ui.so'),
    (
        'vendor/lib64/libgraphgen.so',
    ): blob_fixup()
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_lock')
        .clear_symbol_version('AHardwareBuffer_unlock'),
    'vendor/lib64/libsamsungcamerahal.so': blob_fixup()
        .sig_replace('e0 3a', 'a0 3b'),
    'vendor/lib64/libsec-ril.so': blob_fixup()
        .replace_needed('libprotobuf-cpp-full-21.7.so', 'libprotobuf-cpp-full-21.12.so')
        .sig_replace(
            '0e 40 f9 e1 03 16 aa 82 0c 80 52 e3 03 15 aa',
            '0e 40 f9 e1 03 16 aa 82 0c 80 52 03 00 80 d2'),
    (
        'vendor/lib64/libsensorlistener.so',
        'vendor/lib64/libvdis_core.so',
    ): blob_fixup()
        .add_needed('libshim_sensorndkbridge.so'),
    'vendor/lib64/libskeymint_cli.so': blob_fixup()
        .add_needed('libshim_crypto.so'),
}  # fmt: skip


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    'libuuid': lib_fixup_vendor_suffix,
}

module = ExtractUtilsModule(
    's5e9945',
    'samsung',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if len(sys.argv) > 1 and os.path.isdir(sys.argv[1]):
    apex_path = os.path.join(sys.argv[1], 'vendor/apex/com.samsung.android.gnss.lsi.root.signed')

    if not os.path.isdir(apex_path):
        print(f'Extracting {apex_path}...')

        with tempfile.TemporaryDirectory() as tmp_dir:
            ZipFile(apex_path + '.apex').extractall(tmp_dir)

            with tempfile.TemporaryDirectory() as tmp_payload_dir:
                os.system('sudo mount -o ro ' + tmp_dir + '/apex_payload.img ' + tmp_payload_dir)
                copytree(tmp_payload_dir, apex_path, ignore = lambda path, names: 'lost+found')
                os.system('sudo umount ' + tmp_payload_dir)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
