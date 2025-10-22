/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.livedisplay-service.s5e9945"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include <livedisplay/s5e9945/DisplayColorCalibration.h>
#include <livedisplay/s5e9945/DisplayModes.h>
#include <livedisplay/s5e9945/ReadingEnhancement.h>
#include <livedisplay/s5e9945/SunlightEnhancement.h>

using ::aidl::vendor::lineage::livedisplay::s5e9945::DisplayColorCalibration;
using ::aidl::vendor::lineage::livedisplay::s5e9945::DisplayModes;
using ::aidl::vendor::lineage::livedisplay::s5e9945::ReadingEnhancementExynos;
using ::aidl::vendor::lineage::livedisplay::s5e9945::SunlightEnhancementExynos;

int main() {
    android::ProcessState::self()->setThreadPoolMaxThreadCount(1);
    android::ProcessState::self()->startThreadPool();

    std::shared_ptr<DisplayColorCalibration> displayColorCalibration =
            ndk::SharedRefBase::make<DisplayColorCalibration>();
    std::shared_ptr<DisplayModes> displayModes = ndk::SharedRefBase::make<DisplayModes>();
    std::shared_ptr<ReadingEnhancementExynos> readingEnhancement =
            ndk::SharedRefBase::make<ReadingEnhancementExynos>();
    std::shared_ptr<SunlightEnhancementExynos> sunlightEnhancement =
            ndk::SharedRefBase::make<SunlightEnhancementExynos>();
    binder_status_t status;

    LOG(INFO) << "LiveDisplay HAL service is starting.";

    if (displayColorCalibration == nullptr) {
        LOG(ERROR) << "Can not create an instance of LiveDisplay HAL DisplayColorCalibration "
                      "Iface, exiting.";
        goto shutdown;
    }

    if (displayModes == nullptr) {
        LOG(ERROR) << "Can not create an instance of LiveDisplay HAL DisplayModes Iface, exiting.";
        goto shutdown;
    }

    if (readingEnhancement == nullptr) {
        LOG(ERROR) << "Can not create an instance of LiveDisplay HAL ReadingEnhancement Iface, "
                      "exiting.";
        goto shutdown;
    }

    if (sunlightEnhancement == nullptr) {
        LOG(ERROR) << "Can not create an instance of LiveDisplay HAL SunlightEnhancement Iface, "
                      "exiting.";
        goto shutdown;
    }

    if (displayColorCalibration->isSupported()) {
        std::string instance = std::string(DisplayColorCalibration::descriptor) + "/default";
        status = AServiceManager_addService(displayColorCalibration->asBinder().get(),
                                            instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register service for LiveDisplay HAL DisplayColorCalibration "
                          "Iface ("
                       << status << ")";
            goto shutdown;
        }
    }

    if (displayModes->isSupported()) {
        std::string instance = std::string(DisplayModes::descriptor) + "/default";
        status = AServiceManager_addService(displayModes->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register service for LiveDisplay HAL DisplayModes Iface ("
                       << status << ")";
            goto shutdown;
        }
    }

    if (readingEnhancement->isSupported()) {
        std::string instance = std::string(ReadingEnhancementExynos::descriptor) + "/default";
        status = AServiceManager_addService(readingEnhancement->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR)
                    << "Could not register service for LiveDisplay HAL ReadingEnhancement Iface ("
                    << status << ")";
            goto shutdown;
        }
    }

    if (sunlightEnhancement->isSupported()) {
        std::string instance = std::string(SunlightEnhancementExynos::descriptor) + "/default";
        status =
                AServiceManager_addService(sunlightEnhancement->asBinder().get(), instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR)
                    << "Could not register service for LiveDisplay HAL SunlightEnhancement Iface ("
                    << status << ")";
            goto shutdown;
        }
    }

    LOG(INFO) << "LiveDisplay HAL service is ready.";
    ABinderProcess_joinThreadPool();

shutdown:
    // In normal operation, we don't expect the thread pool to shutdown
    LOG(ERROR) << "LiveDisplay HAL service is shutting down.";
    return EXIT_FAILURE;
}
