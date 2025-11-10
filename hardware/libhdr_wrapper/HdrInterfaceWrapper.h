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
    int sethdr10pMetaInterface(class hdr10pMetaInterface __attribute__((unused))* hdr10pMetaIf) override {return 0;}
    int getHdrCoefSize(enum HdrHwId hw_id) override;

    // per frame setting phase (set when there is any change)
    int setTargetInfo(struct HdrTargetInfo* tInfo) override;
    void setHDRlayer(bool hasHdr) override;
    void setRenderIntent(int rendIntent) override;
    void setSensorInfo(float __attribute__((unused)) max, float __attribute__((unused)) val) override {}

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

    // dlopen handle and opaque object buffer constructed via vendor ctor
    void* mLib = nullptr;
    void* mObj = nullptr;

    // Function pointer types
    using Ctor = void (*)(void* self);
    using Dtor = void (*)(void* self);
    using F_setTargetInfo = int (*)(void* self, HdrTargetInfo*);
    using F_setHDRlayer = void (*)(void* self, bool);
    using F_setRenderIntent = void (*)(void* self, int);
    using F_initHdrCoefBuildup = int (*)(void* self);
    using F_needHdrProcessing = bool (*)(void* self, HdrLayerInfo*);
    using F_setLayerInfo = int (*)(void* self, int, HdrLayerInfo*);
    using F_getHdrCoefData = int (*)(void* self, int hw_id, int* out);
    using F_setLogLevel = void (*)(void* self, int);
    using F_setDebugMode = void (*)(void* self, DebugMode);

    // Add function pointer types for the extra symbols
    using F_initHdrInterfaces = void (*)(void*);
    using F_deinitHdrInterfaces = void (*)(void*);
    using F_initHdrBufFds = void (*)(void*);
    using F_deinitHdrBufFds = void (*)(void*);
    using F_initCurIf = void (*)(void*);
    using F_isHdrAvailable = bool (*)(void*);
    using F_getAttributes = unsigned int (*)(void*);
    using F_initHdrCoefSize = void (*)(void*);

    // Symbols resolved from the blob which match IF_VER 1.1
    Ctor mCtor = nullptr;
    Dtor mDtor = nullptr;
    F_setTargetInfo mSetTargetInfo = nullptr;
    F_setHDRlayer mSetHDRlayer = nullptr;
    F_setRenderIntent mSetRenderIntent = nullptr;
    F_initHdrCoefBuildup mInitHdrCoefBuildup = nullptr;
    F_needHdrProcessing mNeedHdrProcessing = nullptr;
    F_setLayerInfo mSetLayerInfo = nullptr;
    F_getHdrCoefData mGetHdrCoefData = nullptr;
    F_setLogLevel mSetLogLevel = nullptr;
    F_setDebugMode mSetDebugMode = nullptr;

    // New symbols (IF_VER 2.1)
    F_initHdrInterfaces mInitHdrInterfaces = nullptr;
    F_deinitHdrInterfaces mDeinitHdrInterfaces = nullptr;
    F_initHdrBufFds mInitHdrBufFds = nullptr;
    F_deinitHdrBufFds mDeinitHdrBufFds = nullptr;
    F_initCurIf mInitCurIf = nullptr;
    F_isHdrAvailable mIsHdrAvailable = nullptr;
    F_getAttributes mGetAttributes = nullptr;
    F_initHdrCoefSize mInitHdrCoefSize = nullptr;

    // Plumbing
    bool openLib();
    bool resolveSyms();
    bool constructObj();
    void destroyObj();

    static void* symReq(void* h, const char* name);
    static void* symOpt(void* h, const char* name);
};