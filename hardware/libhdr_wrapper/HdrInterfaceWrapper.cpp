#include "HdrInterfaceWrapper.h"
#include <cstdlib>
#include <cstring>

namespace {
constexpr const char* kLib = "/vendor/lib64/libhdrwrapper.so";

// ctor/dtor you found:
constexpr const char* kCtorSym = "_ZN13libhdrwrapperC1Ev"; // C2Ev also OK
constexpr const char* kDtorSym = "_ZN13libhdrwrapperD1Ev"; // D2Ev also OK

// Methods you found:
constexpr const char* kSetTargetSym = "_ZN13libhdrwrapper13setTargetInfoEP13HdrTargetInfo";
constexpr const char* kSetHDRSym    = "_ZN13libhdrwrapper11setHDRlayerEb";
constexpr const char* kSetIntentSym = "_ZN13libhdrwrapper15setRenderIntentEi";
constexpr const char* kInitCoefSym  = "_ZN13libhdrwrapper18initHdrCoefBuildupEv";
constexpr const char* kSetLayerSym  = "_ZN13libhdrwrapper12setLayerInfoEiP12HdrLayerInfo";
constexpr const char* kGetCoefSym   = "_ZN13libhdrwrapper14getHdrCoefDataEiRi";
constexpr const char* kSetLogSym    = "_ZN13libhdrwrapper11setLogLevelEi";

// Conservative opaque size for libhdrwrapper object. Update if you learn the true sizeof.
constexpr size_t kObjSize = 1024;
} // namespace

void* HdrInterfaceWrapper::sym(void* h, const char* n) {
    dlerror();
    void* p = dlsym(h, n);
    if (const char* e = dlerror()) {
        LOG(ERROR) << "dlsym(" << n << ") failed: " << e;
        return nullptr;
    }
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
    mCtor      = reinterpret_cast<Ctor     >(sym(mLib, kCtorSym));
    mDtor      = reinterpret_cast<Dtor     >(sym(mLib, kDtorSym));
    mSetTarget = reinterpret_cast<SetTarget>(sym(mLib, kSetTargetSym));
    mSetHDR    = reinterpret_cast<SetHDR   >(sym(mLib, kSetHDRSym));
    mSetIntent = reinterpret_cast<SetIntent>(sym(mLib, kSetIntentSym));
    mInitCoef  = reinterpret_cast<InitCoef >(sym(mLib, kInitCoefSym));
    mSetLayer  = reinterpret_cast<SetLayer >(sym(mLib, kSetLayerSym));
    mGetCoef   = reinterpret_cast<GetCoef  >(sym(mLib, kGetCoefSym));
    mSetLog    = reinterpret_cast<SetLogLvl>(sym(mLib, kSetLogSym));

    // Minimum: ctor, dtor, and all methods we plan to call
    return mCtor && mDtor && mSetTarget && mSetHDR && mSetIntent &&
           mInitCoef && mSetLayer && mGetCoef && mSetLog;
}

bool HdrInterfaceWrapper::constructObj() {
    // Allocate an aligned opaque buffer and run the vendor ctor on it.
    mObj = std::aligned_alloc(alignof(std::max_align_t), kObjSize);
    if (!mObj) {
        LOG(ERROR) << "alloc libhdrwrapper object failed";
        return false;
    }
    std::memset(mObj, 0, kObjSize);
    mCtor(mObj); // placement-construct
    return true;
}

void HdrInterfaceWrapper::destroyObj() {
    if (mObj && mDtor) mDtor(mObj);
    if (mObj) { std::free(mObj); mObj = nullptr; }
    if (mLib) { dlclose(mLib);   mLib = nullptr; }
}

// ===== Public API =====

hdrInterface* HdrInterfaceWrapper::Create(const char* /*docname*/) {
    auto* w = new HdrInterfaceWrapper();
    if (!w->openLib() || !w->resolveSyms() || !w->constructObj()) {
        delete w;
        return nullptr;
    }
    return w;
}

void HdrInterfaceWrapper::Destroy(hdrInterface* p) {
    delete static_cast<HdrInterfaceWrapper*>(p);
}

HdrInterfaceWrapper::~HdrInterfaceWrapper() {
    destroyObj();
}

// ===== Forwarders =====

int HdrInterfaceWrapper::setTargetInfo(struct HdrTargetInfo* tInfo) {
    return mSetTarget ? mSetTarget(mObj, tInfo) : -HDR_ERR_PTR;
}

int HdrInterfaceWrapper::initHdrCoefBuildup(enum HdrHwId /*hw_id*/) {
    return mInitCoef ? mInitCoef(mObj) : -HDR_ERR_PTR;
}

// The blob doesn’t publish a dedicated “size” API.
// Many vendors return a size/fd via getHdrCoefData(hw, out).
int HdrInterfaceWrapper::getHdrCoefSize(enum HdrHwId hw_id) {
    if (!mGetCoef) return -HDR_ERR_PTR;
    int out = -1;
    int ret = mGetCoef(mObj, static_cast<int>(hw_id), out);
    return (ret == 0) ? out : ret;  // if 'out' is size, this returns it; if fd, adjust your callers
}

void HdrInterfaceWrapper::setHDRlayer(bool hasHdr) {
    if (mSetHDR) mSetHDR(mObj, hasHdr);
}

void HdrInterfaceWrapper::setRenderIntent(int rendIntent) {
    if (mSetIntent) mSetIntent(mObj, rendIntent);
}

// Blob signature is setLayerInfo(int, HdrLayerInfo*). We don’t know that struct layout;
// start with nullptr — many flows only use layer index to select internal state.
int HdrInterfaceWrapper::setLayerInfo(int layer_index, int /*dataspace*/,
                                      void* /*static_md*/, int /*static_len*/,
                                      void* /*dyn_md*/, int /*dyn_len*/,
                                      bool /*premult*/, enum HdrBpc /*bpc*/,
                                      enum RenderSource /*src*/, float* /*tf*/,
                                      bool /*bypass*/) {
    return mSetLayer ? mSetLayer(mObj, layer_index, nullptr) : -HDR_ERR_PTR;
}

// Blob has only getHdrCoefData(int hw, int& out). No per-layer overload found;
// ignore layer_index and surface the 'out' value via the parcel pointer field.
int HdrInterfaceWrapper::getHdrCoefData(enum HdrHwId hw_id, int /*layer_index*/,
                                        struct hdrCoefParcel* parcel) {
    if (!mGetCoef) return -HDR_ERR_PTR;
    int out = -1;
    int ret = mGetCoef(mObj, static_cast<int>(hw_id), out);
    if (ret == 0 && parcel) {
        parcel->hdrCoef = reinterpret_cast<void*>(static_cast<intptr_t>(out));
    }
    return ret;
}

int HdrInterfaceWrapper::getHdrCoefData(enum HdrHwId hw_id, struct hdrCoefParcel* parcel) {
    return getHdrCoefData(hw_id, /*layer_index*/0, parcel);
}

int HdrInterfaceWrapper::getHdrCoef(android_dataspace_t /*ids*/[], int /*mastering_luminance*/[],
                                    int /*n_layer*/, android_dataspace_t /*ods*/,
                                    int /*peak_luminance*/, struct hdrCoef /*output*/[4],
                                    int /*res_map*/[4]) {
    // The blob doesn’t export a direct getHdrCoef(); keep it a no-op for now.
    // Return 0 to behave like other defaulted methods in the header.
    return 0;
    // If you prefer to signal "not available", return -HDR_ERR_NOPERM;
}

void HdrInterfaceWrapper::setLogLevel(int log_level) {
    if (mSetLog) mSetLog(mObj, log_level);
}

