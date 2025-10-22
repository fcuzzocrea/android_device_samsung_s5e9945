/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/file.h>
#include <android-base/strings.h>
#include <livedisplay/s5e9945/SunlightEnhancement.h>

#include <fstream>

using android::base::ReadFileToString;
using android::base::Trim;
using android::base::WriteStringToFile;

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {
namespace s5e9945 {

static constexpr const char* kLUXPath = "/sys/class/mdnie/mdnie/anti_glare";

bool SunlightEnhancementExynos::isSupported() {
    std::fstream file(kLUXPath, file.in | file.out);
    return file.good();
}

// Methods from ::aidl::vendor::lineage::livedisplay::BnSunlightEnhancement follow.
ndk::ScopedAStatus SunlightEnhancementExynos::getEnabled(bool* _aidl_return) {
    std::string tmp;
    int32_t contents = 0;

    if (!ReadFileToString(kLUXPath, &tmp)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    contents = std::stoi(Trim(tmp));

    *_aidl_return = contents > 0;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SunlightEnhancementExynos::setEnabled(bool enabled) {
    /* see drivers/gpu/drm/samsung/panel/mdnie.c, mdnie_anti_glare_enum_items */
    if (!WriteStringToFile(enabled ? "1" : "0", kLUXPath, true)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace s5e9945
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
