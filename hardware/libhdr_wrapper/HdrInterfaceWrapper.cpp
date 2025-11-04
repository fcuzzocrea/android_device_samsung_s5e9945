#include "HdrInterfaceWrapper.h"
#include <cstdlib>
#include <cstring>

namespace {
constexpr const char* kLib = "/vendor/lib64/libhdrwrapper.so";

// ctor/dtor (you confirmed these exist)
constexpr const char* kCtorSym = "_ZN13libhdrwrapperC1Ev"; // C2Ev also exists
constexpr const char* kDtorSym = "_ZN13libhdrwrapperD1Ev"; // D2Ev also exists

// required methods (you confirmed)
constexpr const char* kSetTargetSym = "_ZN13libhdrwrapper13setTargetInfoEP13HdrTargetInfo";
constexpr const char* kSetHDRSym    = "_ZN13libhdrwrapper11setHDRlayerEb";
constexpr const char* kSetIntentSym = "_ZN13libhdrwrapper15setRenderIntentEi";
constexpr const char* kInitCoefSym  = "_ZN13libhdrwrapper18initHdrCoefBuildupEv";
constexpr const char* kSetLayerSym  = "_ZN13libhdrwrapper12setLayerInfoEiP12HdrLayerInfo";
constexpr const char* kGetCoefSym   = "_ZN13libhdrwrapper14getHdrCoefDataEiRi";
constexpr const char* kSetLogSym    = "_ZN13libhdrwrapper11setLogLevelEi";

// optional methods (present in earlier dump; probe safely)
constexpr const char* kNeedProcSym  = "_ZN13libhdrwrapper18needHdrProcessingEP12HdrLayerInfo";
constexpr const char* kSetDbgSym    = "_ZN13libhdrwrapper12setDebugModeE9DebugMode";

// conservative opaque size for the vendor object
constexpr size_t kObjSize = 1024;
} // namespace

void* HdrInterfaceWrapper::symReq(void* h, const char* n) {
    dlerror();
    void* p = dlsym(h, n);
    if (const char* e = dlerror()) {
        LOG(ERROR) << "dlsym(" << n << ") failed: " << e;
        return nullptr;
    }
    return p;
}
void* HdrInterfaceWrapper::symOpt(void* h, const char* n) {
    dlerror();
    void* p = dlsym(h, n);
    (void)dlerror(); // ignore errors
    return p;
}

bool HdrInterfaceWrapper::openLib() {
    mLib = dlopen(kLib, RTLD_NOW | RTLD_LOCAL);
    if (!mLib) {
        LOG(ERROR) << "dlopen(" << kLib << ") failed: " << dlerror();
        return false;
    }
    return true;
}

bool HdrInterfaceWrapper::resolveSyms() {
    mCtor      = reinterpret_cast<Ctor>(symReq(mLib, kCtorSym));
    mDtor      = reinterpret_cast<Dtor>(symReq(mLib, kDtorSym));
    mSetTarget = reinterpret_cast<F_setTarget>(symReq(mLib, kSetTargetSym));
    mSetHDR    = reinterpret_cast<F_setHDR>(symReq(mLib, kSetHDRSym));
    mSetIntent = reinterpret_cast<F_setIntent>(symReq(mLib, kSetIntentSym));
    mInitCoef  = reinterpret_cast<F_initCoef>(symReq(mLib, kInitCoefSym));
    mSetLayer  = reinterpret_cast<F_setLayer>(symReq(mLib, kSetLayerSym));
    mGetCoef   = reinterpret_cast<F_getCoefData>(symReq(mLib, kGetCoefSym));
    mSetLog    = reinterpret_cast<F_setLog>(symReq(mLib, kSetLogSym));

    // optional
    mNeedProc  = reinterpret_cast<F_needProc>(symOpt(mLib, kNeedProcSym));
    mSetDbg    = reinterpret_cast<F_setDebugMode>(symOpt(mLib, kSetDbgSym));

    return mCtor && mDtor && mSetTarget && mSetHDR && mSetIntent &&
           mInitCoef && mSetLayer && mGetCoef && mSetLog;
}

bool HdrInterfaceWrapper::constructObj() {
    mObj = std::aligned_alloc(alignof(std::max_align_t), kObjSize);
    if (!mObj) {
        LOG(ERROR) << "alloc libhdrwrapper obj failed";
        return false;
    }
    std::memset(mObj, 0, kObjSize);
    mCtor(mObj); // placement construct
    return true;
}

void HdrInterfaceWrapper::destroyObj() {
    if (mObj && mDtor) mDtor(mObj);
    if (mObj) { std::free(mObj); mObj = nullptr; }
    if (mLib) { dlclose(mLib);   mLib = nullptr; }
}

// ===== Factory / dtor =====
hdrInterface* HdrInterfaceWrapper::Create() {
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
    destroyObj();
}

// ===== hdrInterface forwards =====

int HdrInterfaceWrapper::getHdrCoefSize(enum HdrHwId hw_id) {
    // Blob doesn’t expose a dedicated size call; many return a size/fd via getHdrCoefData.
    if (!mGetCoef) return -HDR_ERR_PTR;
    int out = -1;
    int ret = mGetCoef(mObj, static_cast<int>(hw_id), out);
    return (ret == 0) ? out : ret;
}

int HdrInterfaceWrapper::setTargetInfo(struct HdrTargetInfo* tInfo) {
    return mSetTarget ? mSetTarget(mObj, tInfo) : -HDR_ERR_PTR;
}

void HdrInterfaceWrapper::setHDRlayer(bool hasHdr) {
    if (mSetHDR) mSetHDR(mObj, hasHdr);
}

void HdrInterfaceWrapper::setRenderIntent(int rendIntent) {
    if (mSetIntent) mSetIntent(mObj, rendIntent);
}

int HdrInterfaceWrapper::initHdrCoefBuildup(enum HdrHwId /*hw_id*/) {
    return mInitCoef ? mInitCoef(mObj) : -HDR_ERR_PTR;
}

bool HdrInterfaceWrapper::needHdrProcessing(struct HdrLayerInfo* lInfo) {
    // Optional in blob; default false if not present.
    return mNeedProc ? mNeedProc(mObj, lInfo) : false;
}

int HdrInterfaceWrapper::setLayerInfo(int layer_index, struct HdrLayerInfo* lInfo) {
    return mSetLayer ? mSetLayer(mObj, layer_index, lInfo) : -HDR_ERR_PTR;
}

int HdrInterfaceWrapper::getHdrCoefData(enum HdrHwId hw_id, int /*layer_index*/,
                                        struct hdrCoefParcel* parcel) {
    if (!mGetCoef) return -HDR_ERR_PTR;
    int out = -1;
    int ret = mGetCoef(mObj, static_cast<int>(hw_id), out);
    if (ret == 0 && parcel) {
        // We don’t yet know if 'out' is a size or an fd; surface it via the pointer field.
        parcel->hdrCoef = reinterpret_cast<void*>(static_cast<intptr_t>(out));
    }
    return ret;
}

int HdrInterfaceWrapper::getHdrCoefData(enum HdrHwId hw_id, struct hdrCoefParcel* parcel) {
    return getHdrCoefData(hw_id, /*layer_index*/0, parcel);
}

void HdrInterfaceWrapper::setLogLevel(int log_level) {
    if (mSetLog) mSetLog(mObj, log_level);
}

void HdrInterfaceWrapper::setDebugMode(enum DebugMode debug_mode) {
    if (mSetDbg) mSetDbg(mObj, debug_mode);
}