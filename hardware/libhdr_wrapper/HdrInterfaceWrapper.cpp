#include "HdrInterfaceWrapper.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utils/Log.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#define LOG_NDEBUG 0

namespace {
// libhdrwrapper path
constexpr const char* kLib = "/vendor/lib64/libhdrwrapper.so";

// Constructor and destructor  of libhdrwrapper.so
constexpr const char* kCtorSym = "_ZN13libhdrwrapperC1Ev";
constexpr const char* kDtorSym = "_ZN13libhdrwrapperD1Ev";

/* Methods exposed by libhdrwrapper.so */
constexpr const char* kSetTargetInfoSym = "_ZN13libhdrwrapper13setTargetInfoEP13HdrTargetInfo";
constexpr const char* kSetHDRlayerSym = "_ZN13libhdrwrapper11setHDRlayerEb";
constexpr const char* kSetRenderIntentSym = "_ZN13libhdrwrapper15setRenderIntentEi";
constexpr const char* kInitHdrCoefBuildupSym = "_ZN13libhdrwrapper18initHdrCoefBuildupEv";
constexpr const char* kNeedHdrProcessingSym = "_ZN13libhdrwrapper17needHdrProcessingEP12HdrLayerInfo";
constexpr const char* kSetLayerInfoSym = "_ZN13libhdrwrapper12setLayerInfoEiP12HdrLayerInfo";
constexpr const char* kGetHdrCoefDataSym = "_ZN13libhdrwrapper14getHdrCoefDataEiRi";
constexpr const char* kSetLogLevelSym = "_ZN13libhdrwrapper11setLogLevelEi";
constexpr const char* kSetDebugModeSym = "_ZN13libhdrwrapper12setDebugModeE9DebugMode";

constexpr const char* kInitHdrInterfacesSym = "_ZN13libhdrwrapper17initHdrInterfacesEv";
constexpr const char* kDeinitHdrInterfacesSym = "_ZN13libhdrwrapper19deinitHdrInterfacesEv";
constexpr const char* kInitHdrBufFdsSym = "_ZN13libhdrwrapper13initHdrBufFdsEv";
constexpr const char* kDeinitHdrBufFdsSym = "_ZN13libhdrwrapper15deinitHdrBufFdsEv";
constexpr const char* kInitCurIfSym = "_ZN13libhdrwrapper9initCurIfEv";
constexpr const char* kIsHdrAvailableSym = "_ZN13libhdrwrapper14isHdrAvailableEv";
constexpr const char* kGetAttributesSym = "_ZN13libhdrwrapper13getAttributesEv";
constexpr const char* kInitHdrCoefSizeSym = "_ZN13libhdrwrapper15initHdrCoefSizeEv";

constexpr size_t kObjSize = 1024;
}  // namespace

void* HdrInterfaceWrapper::symReq(void* h, const char* n) {
    ALOGI("calling %s", __func__);
    dlerror();
    ALOGD("dlsym required symbol %s", n);
    void* p = dlsym(h, n);
    if (const char* e = dlerror()) {
        ALOGE("dlsym required symbol (%s) failed: %s", n, e);
        return nullptr;
    }
    return p;
}
void* HdrInterfaceWrapper::symOpt(void* h, const char* n) {
    ALOGI("calling %s", __func__);
    dlerror();
    ALOGD("dlsym optional symbol %s", n);
    void* p = dlsym(h, n);
    if (const char* e = dlerror()) {
        ALOGE("dlsym optional symbol (%s) failed: %s", n, e);
        return nullptr;
    }
    return p;
}

bool HdrInterfaceWrapper::openLib() {
    ALOGI("calling %s", __func__);
    mLib = dlopen(kLib, RTLD_NOW | RTLD_LOCAL);
    if (!mLib) {
        ALOGE("dlopen %s failed: %s", kLib, dlerror());
        return false;
    }

    return true;
}

bool HdrInterfaceWrapper::resolveSyms() {
    ALOGI("calling %s", __func__);
    mCtor = reinterpret_cast<Ctor>(symReq(mLib, kCtorSym));
    mDtor = reinterpret_cast<Dtor>(symReq(mLib, kDtorSym));
    mSetTargetInfo = reinterpret_cast<F_setTargetInfo>(symReq(mLib, kSetTargetInfoSym));
    mSetHDRlayer = reinterpret_cast<F_setHDRlayer>(symReq(mLib, kSetHDRlayerSym));
    mSetRenderIntent = reinterpret_cast<F_setRenderIntent>(symReq(mLib, kSetRenderIntentSym));
    mInitHdrCoefBuildup = reinterpret_cast<F_initHdrCoefBuildup>(symReq(mLib, kInitHdrCoefBuildupSym));
    mNeedHdrProcessing = reinterpret_cast<F_needHdrProcessing>(symReq(mLib, kNeedHdrProcessingSym));
    mSetLayerInfo = reinterpret_cast<F_setLayerInfo>(symReq(mLib, kSetLayerInfoSym));
    mGetHdrCoefData = reinterpret_cast<F_getHdrCoefData>(symReq(mLib, kGetHdrCoefDataSym));
    mSetLogLevel = reinterpret_cast<F_setLogLevel>(symReq(mLib, kSetLogLevelSym));
    mSetDebugMode = reinterpret_cast<F_setDebugMode>(symReq(mLib, kSetDebugModeSym));

    mInitHdrInterfaces = reinterpret_cast<F_initHdrInterfaces>(symOpt(mLib, kInitHdrInterfacesSym));
    mDeinitHdrInterfaces = reinterpret_cast<F_deinitHdrInterfaces>(symOpt(mLib, kDeinitHdrInterfacesSym));
    mInitHdrBufFds = reinterpret_cast<F_initHdrBufFds>(symOpt(mLib, kInitHdrBufFdsSym));
    mDeinitHdrBufFds = reinterpret_cast<F_deinitHdrBufFds>(symOpt(mLib, kDeinitHdrBufFdsSym));
    mInitCurIf = reinterpret_cast<F_initCurIf>(symOpt(mLib, kInitCurIfSym));
    mIsHdrAvailable = reinterpret_cast<F_isHdrAvailable>(symOpt(mLib, kIsHdrAvailableSym));
    mGetAttributes = reinterpret_cast<F_getAttributes>(symOpt(mLib, kGetAttributesSym));
    mInitHdrCoefSize = reinterpret_cast<F_initHdrCoefSize>(symOpt(mLib, kInitHdrCoefSizeSym));

    return mCtor && mDtor && mSetTargetInfo && mSetHDRlayer && mSetRenderIntent &&
           mInitHdrCoefBuildup && mSetLayerInfo && mGetHdrCoefData && mSetLogLevel && mSetDebugMode;
}

bool HdrInterfaceWrapper::constructObj() {
    ALOGI("calling %s", __func__);
    mObj = std::aligned_alloc(alignof(std::max_align_t), kObjSize);

    if (!mObj) {
        ALOGE("Alloc libhdrwrapper obj failed");
        return false;
    }

    std::memset(mObj, 0, kObjSize);
    mCtor(mObj);
    return true;
}

void HdrInterfaceWrapper::destroyObj() {
    ALOGI("calling %s", __func__);
    if (mObj && mDtor) {
        mDtor(mObj);
    }

    if (mObj) {
        std::free(mObj);
        mObj = nullptr;
    }

    if (mLib) {
        dlclose(mLib);
        mLib = nullptr;
    }
}

hdrInterface* HdrInterfaceWrapper::Create() {
    ALOGI("calling %s", __func__);
    ALOGI("Creating libhdr_wrapper instance");

    auto* w = new HdrInterfaceWrapper();

    if (!w->openLib() || !w->resolveSyms() || !w->constructObj()) {
        ALOGE("Unable to create libhdr_wrapper instance");
        delete w;
        return nullptr;
    }

    if (w->mInitHdrInterfaces) {
         w->mInitHdrInterfaces(w->mObj);
    }

    if (w->mInitHdrCoefSize) {
      w->mInitHdrCoefSize(w->mObj);
    }

    if (w->mInitHdrBufFds){
        w->mInitHdrBufFds(w->mObj);
    }

    return w;
}

void HdrInterfaceWrapper::Destroy(hdrInterface* inst) {
    ALOGI("calling %s", __func__);
    delete static_cast<HdrInterfaceWrapper*>(inst);
}

HdrInterfaceWrapper::~HdrInterfaceWrapper() {
    ALOGI("calling %s", __func__);
    ALOGI("Destroying libhdr_wrapper instance");
    destroyObj();
}

int HdrInterfaceWrapper::getHdrCoefSize(enum HdrHwId hw_id) {
    ALOGI("calling %s", __func__);
    if (hw_id < 0 || hw_id >= HDR_HW_MAX) {
        return 0;
    } else {
        return static_cast<int>(sizeof(struct hdrCoef));
    }
}

int HdrInterfaceWrapper::setTargetInfo(struct HdrTargetInfo* tInfo) {
    ALOGI("calling %s", __func__);
    if (mSetTargetInfo) {
        return mSetTargetInfo(mObj, tInfo);
    } else {
        return HDR_ERR_PTR;
    }
}

void HdrInterfaceWrapper::setHDRlayer(bool hasHdr) {
    ALOGI("calling %s", __func__);
    if (mSetHDRlayer) {
        mSetHDRlayer(mObj, hasHdr);
    }
}

void HdrInterfaceWrapper::setRenderIntent(int rendIntent) {
    ALOGI("calling %s", __func__);
    if (mSetRenderIntent) {
        mSetRenderIntent(mObj, rendIntent);
    }
}

int HdrInterfaceWrapper::initHdrCoefBuildup(enum HdrHwId) {
    ALOGI("calling %s", __func__);
    if (mInitHdrCoefBuildup) {
        return mInitHdrCoefBuildup(mObj);
    } else {
        return HDR_ERR_PTR;
    }
}

bool HdrInterfaceWrapper::needHdrProcessing(struct HdrLayerInfo* lInfo) {
    ALOGI("calling %s", __func__);
    if (mNeedHdrProcessing) {
        return mNeedHdrProcessing(mObj, lInfo);
    } else {
        return false;
    }
}

int HdrInterfaceWrapper::setLayerInfo(int layer_index, struct HdrLayerInfo* lInfo) {
    ALOGI("calling %s", __func__);
    int ret;
    ret = mSetLayerInfo(mObj, layer_index, lInfo);
    ALOGI("%s ret is %d", __func__, ret);
    if (ret) {
        return mSetLayerInfo(mObj, layer_index, lInfo);
    } else {
        return HDR_ERR_PTR;
    }
}

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, int layer_index, hdrCoefParcel* parcel) {
    ALOGI("calling %s 3 args", __func__);
    if (!parcel || !parcel->hdrCoef) return -HDR_ERR_PTR;
    if (!mGetHdrCoefData)            return -HDR_ERR_PTR;
    if (layer_index < 0)             return -HDR_ERR_INVAL;

    const size_t len = sizeof(struct hdrCoef);

    int out = -1;
    const int ret = mGetHdrCoefData(mObj, layer_index, &out);
    if (ret != 0 || out == -1) return (ret != 0) ? ret : -HDR_ERR_INVAL;

    // FD case?
    if (out >= 0 && fcntl(out, F_GETFD) != -1) {
        // Borrowed FD → dup it, operate on the dup, then close the dup.
        int fd = dup(out);
        if (fd < 0) return -HDR_ERR_INVAL;

        struct stat st{};
        if (fstat(fd, &st) != 0 || st.st_size <= 0) {
            close(fd);
            return -HDR_ERR_INVAL;
        }

        const size_t to_copy = std::min<size_t>(len, st.st_size);
        void* src = mmap(nullptr, to_copy, PROT_READ, MAP_PRIVATE, fd, 0);
        if (src == MAP_FAILED) {
            close(fd);
            return -HDR_ERR_NOPERM;
        }

        memcpy(parcel->hdrCoef, src, to_copy);
        munmap(src, to_copy);
        close(fd);           // close only the dup, NOT `out`
        if (to_copy < len) { // optional: pad tail
            memset((uint8_t*)parcel->hdrCoef + to_copy, 0, len - to_copy);
        }
        return 0;
    }

    // Pointer case
    if (out == 0) return -HDR_ERR_INVAL;
    memcpy(parcel->hdrCoef, reinterpret_cast<void*>(static_cast<intptr_t>(out)), len);
    return 0;
}

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, hdrCoefParcel* parcel) {
    ALOGI("calling %s 2 aregfs", __func__);
    return getHdrCoefData(hw_id, /*layer_index=*/0, parcel);
}

void HdrInterfaceWrapper::setLogLevel(int log_level) {
    ALOGI("calling %s", __func__);
    if (mSetLogLevel) {
        mSetLogLevel(mObj, log_level);
    }
}

void HdrInterfaceWrapper::setDebugMode(enum DebugMode debug_mode) {
    ALOGI("calling %s", __func__);
    if (mSetDebugMode) {
        mSetDebugMode(mObj, debug_mode);
    }
}
