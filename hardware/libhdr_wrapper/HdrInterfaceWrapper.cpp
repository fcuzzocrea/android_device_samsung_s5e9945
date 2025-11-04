#include "HdrInterfaceWrapper.h"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace {
constexpr const char* kLib = "/vendor/lib64/libhdrwrapper.so";

// ctor/dtor
constexpr const char* kCtorSym = "_ZN13libhdrwrapperC1Ev";
constexpr const char* kDtorSym = "_ZN13libhdrwrapperD1Ev";

// required methods
constexpr const char* kSetTargetSym = "_ZN13libhdrwrapper13setTargetInfoEP13HdrTargetInfo";
constexpr const char* kSetHDRSym    = "_ZN13libhdrwrapper11setHDRlayerEb";
constexpr const char* kSetIntentSym = "_ZN13libhdrwrapper15setRenderIntentEi";
constexpr const char* kBuildupSym   = "_ZN13libhdrwrapper18initHdrCoefBuildupEv";
constexpr const char* kSetLayerSym  = "_ZN13libhdrwrapper12setLayerInfoEiP12HdrLayerInfo";
constexpr const char* kGetCoefSym   = "_ZN13libhdrwrapper14getHdrCoefDataEiRi";
constexpr const char* kSetLogSym    = "_ZN13libhdrwrapper11setLogLevelEi";

// extra init/teardown/attr
constexpr const char* kInitIfSym    = "_ZN13libhdrwrapper17initHdrInterfacesEv";
constexpr const char* kDeinitIfSym  = "_ZN13libhdrwrapper19deinitHdrInterfacesEv";
constexpr const char* kInitBufSym   = "_ZN13libhdrwrapper13initHdrBufFdsEv";
constexpr const char* kDeinitBufSym = "_ZN13libhdrwrapper15deinitHdrBufFdsEv";
constexpr const char* kInitCurSym   = "_ZN13libhdrwrapper8initCurIfEv";
constexpr const char* kIsAvailSym   = "_ZN13libhdrwrapper14isHdrAvailableEv";
constexpr const char* kGetAttrSym   = "_ZN13libhdrwrapper12getAttributesEv";

// size getter (distinct from buildup)
constexpr const char* kGetSizeSym   = "_ZN13libhdrwrapper15initHdrCoefSizeEv";

// optional
constexpr const char* kNeedProcSym  = "_ZN13libhdrwrapper18needHdrProcessingEP12HdrLayerInfo";
constexpr const char* kSetDbgSym    = "_ZN13libhdrwrapper12setDebugModeE9DebugMode";

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
    mInitCoef  = reinterpret_cast<F_initCoef>(symReq(mLib, kBuildupSym));
    mSetLayer  = reinterpret_cast<F_setLayer>(symReq(mLib, kSetLayerSym));
    mGetCoef   = reinterpret_cast<F_getCoefData>(symReq(mLib, kGetCoefSym));
    mSetLog    = reinterpret_cast<F_setLog>(symReq(mLib, kSetLogSym));

    // NEW: resolve the rest (optional)
    mInitIf    = reinterpret_cast<F_initHdrIf>(symOpt(mLib, kInitIfSym));
    mDeinitIf  = reinterpret_cast<F_deinitHdrIf>(symOpt(mLib, kDeinitIfSym));
    mInitBuf   = reinterpret_cast<F_initBufFds>(symOpt(mLib, kInitBufSym));
    mDeinitBuf = reinterpret_cast<F_deinitBufFds>(symOpt(mLib, kDeinitBufSym));
    mInitCur   = reinterpret_cast<F_initCurIf>(symOpt(mLib, kInitCurSym));
    mIsAvail   = reinterpret_cast<F_isAvail>(symOpt(mLib, kIsAvailSym));
    mGetAttr   = reinterpret_cast<F_getAttr>(symOpt(mLib, kGetAttrSym));
    mGetSize   = reinterpret_cast<F_getSize>(symOpt(mLib, kGetSizeSym));

    return mCtor && mDtor && mSetTarget && mSetHDR && mSetIntent &&
           mInitCoef && mSetLayer && mGetCoef && mSetLog;
}

bool HdrInterfaceWrapper::constructObj() {
    mObj = std::aligned_alloc(alignof(std::max_align_t), kObjSize);
    if (!mObj) { LOG(ERROR) << "alloc libhdrwrapper obj failed"; return false; }
    std::memset(mObj, 0, kObjSize);
    mCtor(mObj);

    if (mInitIf)  mInitIf(mObj);
    if (mInitBuf) mInitBuf(mObj);
    if (mInitCur) mInitCur(mObj);

    if (mIsAvail && !mIsAvail(mObj)) {
        LOG(WARNING) << "HDR not available according to blob";
    }
    if (mGetAttr) {
        int attr = mGetAttr(mObj);
        LOG(INFO) << "HDR attributes 0x" << std::hex << attr;
    }
    return true;
}

void HdrInterfaceWrapper::destroyObj() {
    if (mDeinitBuf) mDeinitBuf(mObj);
    if (mDeinitIf)  mDeinitIf(mObj);
    if (mObj && mDtor) mDtor(mObj);
    if (mObj) { std::free(mObj); mObj = nullptr; }
    if (mLib) { dlclose(mLib);   mLib = nullptr; }
    for (auto& m : mMaps) { if (m.addr) munmap(m.addr, m.len); if (m.fd>=0) close(m.fd); }
    mMaps.clear();
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
    if (hw_id < 0 || hw_id >= HDR_HW_MAX) return -HDR_ERR_INVAL;
    if (mCoefSize[hw_id] > 0) return mCoefSize[hw_id];

    // Prefer the explicit size getter if available
    if (mGetSize) {
        int sz = mGetSize(mObj);
        if (sz > 0) mCoefSize[hw_id] = sz;
        return sz > 0 ? sz : -HDR_ERR_INVAL;
    }

    // Fallback: sometimes vendors piggyback size into getHdrCoefData’s “out”
    if (mGetCoef) {
        int out = -1;
        int ret = mGetCoef(mObj, static_cast<int>(hw_id), out);
        if (ret == 0 && out > 0) { mCoefSize[hw_id] = out; return out; }
    }
    return -HDR_ERR_PTR;
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
    if (!parcel || !parcel->hdrCoef) return -HDR_ERR_PTR;
    if (!mGetCoef) return -HDR_ERR_PTR;
    if (hw_id < 0 || hw_id >= HDR_HW_MAX) return -HDR_ERR_INVAL;

    const int len = getHdrCoefSize(hw_id);
    if (len <= 0) return -HDR_ERR_INVAL;

    int out = -1;
    int ret = mGetCoef(mObj, static_cast<int>(hw_id), out);
    if (ret != 0) return ret;

    // FD case?
    if (out >= 0 && fcntl(out, F_GETFD) != -1) {
        void* src = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, out, 0);
        if (src == MAP_FAILED) { close(out); return -HDR_ERR_NOPERM; }
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
    return getHdrCoefData(hw_id, /*layer_index*/0, parcel);
}

void HdrInterfaceWrapper::setLogLevel(int log_level) {
    if (mSetLog) mSetLog(mObj, log_level);
}

void HdrInterfaceWrapper::setDebugMode(enum DebugMode debug_mode) {
    if (mSetDbg) mSetDbg(mObj, debug_mode);
}