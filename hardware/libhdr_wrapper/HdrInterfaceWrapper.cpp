#include "HdrInterfaceWrapper.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utils/Log.h>
#include <cstdlib>
#include <cstring>

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
    mLib = dlopen(kLib, RTLD_NOW | RTLD_LOCAL);
    if (!mLib) {
        ALOGE("dlopen %s failed: %s", kLib, dlerror());
        return false;
    }

    return true;
}

bool HdrInterfaceWrapper::resolveSyms() {
    mCtor = reinterpret_cast<Ctor>(symReq(mLib, kCtorSym));
    mDtor = reinterpret_cast<Dtor>(symReq(mLib, kDtorSym));
    mSetTargetInfo = reinterpret_cast<F_setTargetInfo>(symReq(mLib, kSetTargetInfoSym));
    mSetHDRlayer = reinterpret_cast<F_setHDRlayer>(symReq(mLib, kSetHDRlayerSym));
    mSetRenderIntent = reinterpret_cast<F_setRenderIntent>(symReq(mLib, kSetRenderIntentSym));
    mInitHdrCoefBuildup = reinterpret_cast<F_initHdrCoefBuildup>(symReq(mLib, kInitHdrCoefBuildupSym));
    mNeedHdrProcessing = reinterpret_cast<F_needHdrProcessing>(symReq(mLib, kNeedHdrProcessingSym));
    mSetLayerInfo = reinterpret_cast<F_setLayerInfo>(symReq(mLib, kSetLayerInfoSym));
    mGetHdrCoefData = reinterpret_cast<F_getHdrCoefData>(symReq(mLib, kNeedHdrProcessingSym));
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
    ALOGI("Creating libhdr_wrapper instance");
    auto* w = new HdrInterfaceWrapper();
    if (!w->openLib() || !w->resolveSyms() || !w->constructObj()) {
        delete w;
        return nullptr;
    }
    return w;
}

void HdrInterfaceWrapper::Destroy(hdrInterface* inst) {
    delete static_cast<HdrInterfaceWrapper*>(inst);
}

HdrInterfaceWrapper::~HdrInterfaceWrapper() {
    ALOGI("Destroying libhdr_wrapper instance");
    destroyObj();
}

int HdrInterfaceWrapper::setTargetInfo(struct HdrTargetInfo* tInfo) {
    if (mSetTargetInfo) {
        return mSetTargetInfo(mObj, tInfo);
    } else {
        return -HDR_ERR_PTR;
    }
}

void HdrInterfaceWrapper::setHDRlayer(bool hasHdr) {
    if (mSetHDRlayer) {
        mSetHDRlayer(mObj, hasHdr);
    }
}

void HdrInterfaceWrapper::setRenderIntent(int rendIntent) {
    if (mSetRenderIntent) {
        mSetRenderIntent(mObj, rendIntent);
    }
}

int HdrInterfaceWrapper::initHdrCoefBuildup(enum HdrHwId) {
    if (mInitHdrCoefBuildup) {
        return mInitHdrCoefBuildup(mObj);
    } else {
        return -HDR_ERR_PTR;
    }
}

bool HdrInterfaceWrapper::needHdrProcessing(struct HdrLayerInfo* lInfo) {
    if (mNeedHdrProcessing) {
        return mNeedHdrProcessing(mObj, lInfo);
    } else {
        return false;
    }
}

int HdrInterfaceWrapper::setLayerInfo(int layer_index, struct HdrLayerInfo* lInfo) {
    if (mSetLayerInfo) {
        return mSetLayerInfo(mObj, layer_index, lInfo);
    } else {
        return -HDR_ERR_PTR;
    }
}

int HdrInterfaceWrapper::getHdrCoefData(enum HdrHwId hw_id, int __attribute__((unused)) layer_index,
                                        struct hdrCoefParcel* parcel) {
    if (!parcel || !parcel->hdrCoef) {
        return -HDR_ERR_PTR;
    }

    if (!mGetHdrCoefData) {
        return -HDR_ERR_PTR;
    }

    if (hw_id < 0 || hw_id >= HDR_HW_MAX) return -HDR_ERR_INVAL;

    const int len = getHdrCoefSize(hw_id);

    if (len <= 0) return -HDR_ERR_INVAL;

    int out = -1;
    int ret = mGetHdrCoefData(mObj, static_cast<int>(hw_id), out);

    if (ret != 0) return ret;

    // FD case?
    if (out >= 0 && fcntl(out, F_GETFD) != -1) {
        void* src = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, out, 0);

        if (src == MAP_FAILED) {
            close(out);
            return -HDR_ERR_NOPERM;
        }

        memcpy(parcel->hdrCoef, src, len);
        munmap(src, len);
        close(out);
        return 0;
    }

    // Pointer case (address in our process)
    void* src = reinterpret_cast<void*>(static_cast<intptr_t>(out));
    memcpy(parcel->hdrCoef, src, len);
    return 0;
}

int HdrInterfaceWrapper::getHdrCoefData(enum HdrHwId hw_id, struct hdrCoefParcel* parcel) {
    return getHdrCoefData(hw_id, 0, parcel);
}

void HdrInterfaceWrapper::setLogLevel(int log_level) {
    if (mSetLogLevel) {
        mSetLogLevel(mObj, log_level);
    }
}

void HdrInterfaceWrapper::setDebugMode(enum DebugMode debug_mode) {
    if (mSetDebugMode) {
        mSetDebugMode(mObj, debug_mode);
    }
}

int HdrInterfaceWrapper::getHdrCoefSize(enum HdrHwId hw_id) {
    if (hw_id < 0 || hw_id >= HDR_HW_MAX) {
        return -HDR_ERR_INVAL;
    }

    if (mCoefSize[hw_id] > 0) {
        return mCoefSize[hw_id];
    }

    // Prefer the explicit size getter if available
    if (mInitHdrCoefSize) {
        int sz = mInitHdrCoefSize(mObj);
        if (sz > 0) {
            mCoefSize[hw_id] = sz;
        }

        if (sz > 0) {
            return sz;
        } else {
            return -HDR_ERR_INVAL;
        }
    }

    // Fallback: sometimes vendors piggyback size into getHdrCoefData’s “out”
    if (mGetHdrCoefData) {
        int out = -1;
        int ret = mGetHdrCoefData(mObj, static_cast<int>(hw_id), out);

        if (ret == 0 && out > 0) {
            mCoefSize[hw_id] = out;
            return out;
        }
    }

    return -HDR_ERR_PTR;
}
