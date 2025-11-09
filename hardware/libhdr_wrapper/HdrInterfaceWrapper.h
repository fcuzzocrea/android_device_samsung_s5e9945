#pragma once

// #include <android-base/logging.h>
#include <dlfcn.h>
#include <hardware/exynos/hdrInterface.h>
#include <vector>

class HdrInterfaceWrapper : public hdrInterface {
  public:
    static hdrInterface* Create();
    static void Destroy(hdrInterface* inst);

    ~HdrInterfaceWrapper() override;

    // init phase
    int sethdr10pMetaInterface(class hdr10pMetaInterface __attribute__((unused)) *
                               hdr10pMetaIf) override {
        return 0;
    }
    int getHdrCoefSize(enum HdrHwId hw_id) override;

    // per frame setting phase (set when there is any change)
    int setTargetInfo(struct HdrTargetInfo* tInfo) override;
    void setHDRlayer(bool hasHdr) override;
    void setRenderIntent(int rendIntent) override;
    void setSensorInfo(float __attribute__((unused)) max,
                       float __attribute__((unused)) val) override {}

    // per frame & per layer in between phase
    int initHdrCoefBuildup(enum HdrHwId __attribute__((unused)) hw_id) override;

    // per layer setting phase
    bool needHdrProcessing(struct HdrLayerInfo* lInfo) override;
    int setLayerInfo(int layer_index, struct HdrLayerInfo* lInfo) override;

    // get coef phase
    int getHdrCoefData(enum HdrHwId hw_id, int layer_index, struct hdrCoefParcel* parcel) override;
    int getHdrCoefData(enum HdrHwId hw_id, struct hdrCoefParcel* parcel) override;

    // debug phase
    void setLogLevel(int log_level) override;
    void setDebugMode(enum DebugMode debug_mode) override;

  private:
    HdrInterfaceWrapper() = default;

    // dlopen handle and opaque object buffer constructed via vendor ctor --
    void* mLib = nullptr;
    void* mObj = nullptr;

    // inside class HdrInterfaceWrapper private:
    struct Map {
        void* addr{};
        size_t len{};
        int fd{-1};
    };
    std::vector<Map> mMaps;
    int mCoefSize[HDR_HW_MAX] = {0};  // cache per-HW size

    // Function pointer types
    using Ctor = void (*)(void* self);
    using Dtor = void (*)(void* self);
    using F_setTarget = int (*)(void* self, HdrTargetInfo*);
    using F_setHDR = void (*)(void* self, bool);
    using F_setIntent = void (*)(void* self, int);
    using F_initCoef = int (*)(void* self);
    using F_setLayer = int (*)(void* self, int, HdrLayerInfo*);
    using F_getCoefData = int (*)(void* self, int /*hw*/, int& /*out*/);
    using F_setLog = void (*)(void* self, int);
    using F_needProc = bool (*)(void* self, HdrLayerInfo*);  // optional
    using F_setDebugMode = void (*)(void* self, DebugMode);  // optional

    // Add function pointer types for the extra symbols
    using F_initHdrIf = int (*)(void*);
    using F_deinitHdrIf = void (*)(void*);
    using F_initBufFds = int (*)(void*);
    using F_deinitBufFds = void (*)(void*);
    using F_initCurIf = int (*)(void*);
    using F_isAvail = bool (*)(void*);
    using F_getAttr = int (*)(void*);
    using F_getSize = int (*)(void*);  // initHdrCoefSize()

    // Symbols resolved from the blob
    Ctor mCtor = nullptr;
    Dtor mDtor = nullptr;
    F_setTarget mSetTarget = nullptr;
    F_setHDR mSetHDR = nullptr;
    F_setIntent mSetIntent = nullptr;
    F_initCoef mInitCoef = nullptr;
    F_setLayer mSetLayer = nullptr;
    F_getCoefData mGetCoef = nullptr;
    F_setLog mSetLog = nullptr;
    F_needProc mNeedProc = nullptr;
    F_setDebugMode mSetDbg = nullptr;

    // And the members:
    F_initHdrIf mInitIf = nullptr;
    F_deinitHdrIf mDeinitIf = nullptr;
    F_initBufFds mInitBuf = nullptr;
    F_deinitBufFds mDeinitBuf = nullptr;
    F_initCurIf mInitCur = nullptr;
    F_isAvail mIsAvail = nullptr;
    F_getAttr mGetAttr = nullptr;
    F_getSize mGetSize = nullptr;

    // plumbing
    bool openLib();
    bool resolveSyms();
    bool constructObj();
    void destroyObj();

    static void* symReq(void* h, const char* name);
    static void* symOpt(void* h, const char* name);
};