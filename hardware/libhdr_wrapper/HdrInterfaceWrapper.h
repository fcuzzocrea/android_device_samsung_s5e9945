#pragma once
#include <dlfcn.h>
#include <android-base/logging.h>
#include <hardware/exynos/hdrInterface.h>

class HdrInterfaceWrapper : public hdrInterface {
public:
    static hdrInterface* Create();               // factory you’ll call from HWC
    static void Destroy(hdrInterface* inst);     // optional; delete is fine too

    ~HdrInterfaceWrapper() override;

    // ---- hdrInterface vfuncs (exact signatures from your header) ----
    int sethdr10pMetaInterface(class hdr10pMetaInterface* /*hdr10pMetaIf*/) override { return 0; }
    int getHdrCoefSize(enum HdrHwId hw_id) override;

    int setTargetInfo(struct HdrTargetInfo* tInfo) override;
    void setHDRlayer(bool hasHdr) override;
    void setRenderIntent(int rendIntent) override;
    void setSensorInfo(float /*max*/, float /*val*/) override {}

    int initHdrCoefBuildup(enum HdrHwId /*hw_id*/) override;

    bool needHdrProcessing(struct HdrLayerInfo* lInfo) override;   // optional in blob
    int  setLayerInfo(int layer_index, struct HdrLayerInfo* lInfo) override;

    int getHdrCoefData(enum HdrHwId hw_id, int layer_index,
                       struct hdrCoefParcel* parcel) override;
    int getHdrCoefData(enum HdrHwId hw_id, struct hdrCoefParcel* parcel) override;

    void setLogLevel(int log_level) override;
    void setDebugMode(enum DebugMode debug_mode) override;         // optional in blob

private:
    HdrInterfaceWrapper() = default;

    // -- dlopen handle and opaque object buffer constructed via vendor ctor --
    void* mLib = nullptr;
    void* mObj = nullptr;

    // ---- Function pointer types (AArch64 Itanium; implicit this as 1st arg) ----
    using Ctor      = void (*)(void* self);
    using Dtor      = void (*)(void* self);
    using F_setTarget     = int  (*)(void* self, HdrTargetInfo*);
    using F_setHDR        = void (*)(void* self, bool);
    using F_setIntent     = void (*)(void* self, int);
    using F_initCoef      = int  (*)(void* self);
    using F_setLayer      = int  (*)(void* self, int, HdrLayerInfo*);
    using F_getCoefData   = int  (*)(void* self, int /*hw*/, int& /*out*/);
    using F_setLog        = void (*)(void* self, int);
    using F_needProc      = bool (*)(void* self, HdrLayerInfo*);   // optional
    using F_setDebugMode  = void (*)(void* self, DebugMode);       // optional

    // ---- Resolved symbols ----
    Ctor    mCtor = nullptr;
    Dtor    mDtor = nullptr;
    F_setTarget   mSetTarget = nullptr;
    F_setHDR      mSetHDR = nullptr;
    F_setIntent   mSetIntent = nullptr;
    F_initCoef    mInitCoef = nullptr;
    F_setLayer    mSetLayer = nullptr;
    F_getCoefData mGetCoef = nullptr;
    F_setLog      mSetLog = nullptr;
    F_needProc    mNeedProc = nullptr;      // optional
    F_setDebugMode mSetDbg = nullptr;       // optional

    // plumbing
    bool openLib();
    bool resolveSyms();
    bool constructObj();
    void destroyObj();

    static void* symReq(void* h, const char* name); // required symbol
    static void* symOpt(void* h, const char* name); // optional symbol
};