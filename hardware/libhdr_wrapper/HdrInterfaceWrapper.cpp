#include "HdrInterfaceWrapper.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utils/Log.h>
#include <cstdlib>
#include <cstring>

namespace {
// libhdrwrapper path
constexpr const char* kLib = "/vendor/lib64/libhdrwrapper.so";

// Constructor and destructor of libhdrwrapper.so
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
        ALOGE("Unable to create libhdr_wrapper instance");
        delete w;
        return nullptr;
    }

    if (w->mInitHdrInterfaces) {
        ALOGI("Initializing HDR interfaces");
        w->mInitHdrInterfaces(w->mObj);
    }

    if (w->mInitHdrCoefSize) {
        ALOGI("Initializing HDR coefficient sizes");
        w->mInitHdrCoefSize(w->mObj);
    }

    if (w->mInitHdrBufFds) {
        ALOGI("Initializing HDR buffer fd");
        w->mInitHdrBufFds(w->mObj);
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

int HdrInterfaceWrapper::getHdrCoefSize(enum HdrHwId hw_id) {
    if (hw_id < 0 || hw_id >= HDR_HW_MAX) {
        ALOGE("%s:selected hw id (%d) out of range", __func__, hw_id);
        return HDR_ERR_INVAL;
    } else {
        return static_cast<int>(sizeof(struct hdrCoef));
    }
}

int HdrInterfaceWrapper::setTargetInfo(struct HdrTargetInfo* tInfo) {
    if (mSetTargetInfo) {
        return mSetTargetInfo(mObj, tInfo);
    } else {
        return HDR_ERR_INVAL;
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
        return HDR_ERR_INVAL;
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
    int ret;
    ret = mSetLayerInfo(mObj, layer_index, lInfo);
    if (ret) {
        return mSetLayerInfo(mObj, layer_index, lInfo);
    } else {
        return HDR_ERR_INVAL;
    }
}

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, int layer_index, hdrCoefParcel* parcel) {
    if (!parcel || !parcel->hdrCoef) {
        return HDR_ERR_INVAL;
    }

    if (!mGetHdrCoefData) {
        return HDR_ERR_INVAL;
    }

    if (hw_id < 0 || hw_id >= HDR_HW_MAX) {
        ALOGE("%s:selected hw id (%d) out of range", __func__, hw_id);
        return HDR_ERR_INVAL;
    }

    // Zero the destination
    memset(parcel->hdrCoef, 0x00, sizeof(hdrCoef));

    // The blob will write directly into parcel->hdrCoef.
    int out = -1;
    int ret = mGetHdrCoefData(mObj, layer_index, &out);
    if (ret != HDR_ERR_NO) {
        return ret;
    }

    // If we arrive here, we just return success; the data should already be in parcel->hdrCoef.
    return HDR_ERR_NO;
}

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, hdrCoefParcel* parcel) {
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
