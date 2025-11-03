#pragma once
#include <dlfcn.h>
#include <android-base/logging.h>
#include <string>
#include <hardware/exynos/hdrInterface.h>

class HdrInterfaceWrapper : public hdrInterface {
public:
    static hdrInterface* Create(const char* docname = nullptr);
    static void Destroy(hdrInterface* p);

    ~HdrInterfaceWrapper() override;

    // hdrInterface API (forward to blob)
    int  setTargetInfo(struct HdrTargetInfo* tInfo) override;
    int  sethdr10pMetaInterface(class hdr10pMetaInterface* /*hdr10pMetaIf*/) override { return 0; }
    int  initHdrCoefBuildup(enum HdrHwId /*hw_id*/) override;
    int  getHdrCoefSize(enum HdrHwId /*hw_id*/) override; // best-effort via getHdrCoefData
    void setHDRlayer(bool hasHdr) override;
    void setRenderIntent(int rendIntent) override;
    void setSensorInfo(float, float) override {} // not exported
    int  setLayerInfo(int layer_index, int /*dataspace*/,
                      void* /*static_md*/, int /*static_len*/,
                      void* /*dyn_md*/, int /*dyn_len*/,
                      bool /*premult*/, enum HdrBpc /*bpc*/,
                      enum RenderSource /*src*/, float* /*tf*/,
                      bool /*bypass*/) override;
    int  getHdrCoefData(enum HdrHwId hw_id, int layer_index,
                        struct hdrCoefParcel* parcel) override;
    int  getHdrCoefData(enum HdrHwId hw_id, struct hdrCoefParcel* parcel) override;
    int  getHdrCoef(android_dataspace_t ids[], int mastering_luminance[], int n_layer,
                    android_dataspace_t ods, int peak_luminance,
                    struct hdrCoef output[4], int res_map[4]) override;
    void setLogLevel(int log_level) override;

private:
    HdrInterfaceWrapper() = default;

    void* mLib = nullptr;   // dlopen handle
    void* mObj = nullptr;   // opaque libhdrwrapper instance storage

    // Function pointer types (AArch64 Itanium ABI: 'this' is first param)
    using Ctor      = void (*)(void* self);
    using Dtor      = void (*)(void* self);
    using SetTarget = int  (*)(void* self, HdrTargetInfo*);
    using SetHDR    = void (*)(void* self, bool);
    using SetIntent = void (*)(void* self, int);
    using InitCoef  = int  (*)(void* self);
    using CoefSize  = int  (*)(void* self);                         // not exported — we’ll infer
    using SetLayer  = int  (*)(void* self, int, void* /*HdrLayerInfo* or null*/);
    using GetCoef   = int  (*)(void* self, int /*hw*/, int& /*out*/);
    using SetLogLvl = void (*)(void* self, int);

    // Resolved symbols
    Ctor      mCtor      = nullptr;
    Dtor      mDtor      = nullptr;
    SetTarget mSetTarget = nullptr;
    SetHDR    mSetHDR    = nullptr;
    SetIntent mSetIntent = nullptr;
    InitCoef  mInitCoef  = nullptr;
    SetLayer  mSetLayer  = nullptr;
    GetCoef   mGetCoef   = nullptr;
    SetLogLvl mSetLog    = nullptr;

    bool openLib();
    bool resolveSyms();
    bool constructObj();
    void destroyObj();

    static void* sym(void* h, const char* n);
};
