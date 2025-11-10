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

static void printhdrCoef(int layer_index, unsigned int ids, const struct hdrCoef *output);

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
         ALOGE("Initializing HDR interfaces");
         w->mInitHdrInterfaces(w->mObj);
    }

    if (w->mInitHdrCoefSize) {
        ALOGE("Initializing HDR coefficient sizes");
        w->mInitHdrCoefSize(w->mObj);
    }

    if (w->mInitHdrBufFds){
        ALOGE("Initializing HDR buffer fd");
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
#if 0
int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, int layer_index, hdrCoefParcel* parcel) {
    if (!parcel || !parcel->hdrCoef) return -HDR_ERR_PTR;
    if (!mGetHdrCoefData)            return -HDR_ERR_PTR;
    if (layer_index < 0)             return -HDR_ERR_INVAL;

    const size_t len = sizeof(struct hdrCoef);

    int out = -1;
    const int ret = mGetHdrCoefData(mObj, layer_index, &out);
    ALOGI("%s len %zu, ret %d, layer_index %d, out %d", __func__, len, ret, layer_index, out);
    if (ret != 0 || out == -1) return (ret != 0) ? ret : -HDR_ERR_INVAL;

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
        printhdrCoef(layer_index, hw_id, (const struct hdrCoef*)parcel->hdrCoef);
        munmap(src, to_copy);
        close(fd);           // close only the dup, NOT `out`
        if (to_copy < len) { // optional: pad tail
            memset((uint8_t*)parcel->hdrCoef + to_copy, 0, len - to_copy);
        }
        return HDR_ERR_NO;
    }

    return HDR_ERR_NO;
}

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, hdrCoefParcel* parcel) {
    return getHdrCoefData(hw_id, /*layer_index=*/0, parcel);
}
#endif

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, int layer_index,
                                        hdrCoefParcel* parcel) {
    if (!parcel || !parcel->hdrCoef) return HDR_ERR_PTR;
    if (!mGetHdrCoefData)          return HDR_ERR_PTR;
    if (hw_id < 0 || hw_id >= HDR_HW_MAX) return HDR_ERR_INVAL;

    // Important: zero the destination so we can detect incomplete writes in logs
    memset(parcel->hdrCoef, 0xDA, sizeof(hdrCoef)); // optional but useful for debugging

    // The blob will write directly into parcel->hdrCoef.
    // 'out' (fd) is merely a side channel — ignore it for now.
    int fd_or_status = -1;
    int ret = mGetHdrCoefData(mObj, layer_index, &fd_or_status);
    if (ret != 0) return ret;

    // Don't close() or mmap() 'fd_or_status' here; it's not yours to manage for this flow.
    // Just return success; the data should already be in parcel->hdrCoef.
    return HDR_ERR_NO;
}

int HdrInterfaceWrapper::getHdrCoefData(HdrHwId hw_id, hdrCoefParcel* parcel) {
    // If you keep the 2-arg entry, delegate with a sane default layer (HWC passes the real one anyway)
    return getHdrCoefData(hw_id, /*layer_index=*/0, parcel);
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

#define PRINT_ARRAY(label, arr, len) do { \
    int pos = 0; \
    pos += snprintf(buf + pos, sizeof(buf) - pos, "%s: ", label); \
    for (i = 0; i < (len) && pos < (int)sizeof(buf) - 16; i++) \
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%u ", (arr)[i]); \
    buf[sizeof(buf) - 1] = '\0'; \
    ALOGI("%s", buf); \
} while (0)

static void printhdrCoef(int layer_index,
                         unsigned int ids,
                         const struct hdrCoef *output)
{
    int i;
    char buf[2048];

    ALOGI("%s +", __func__);
    ALOGI("layer_index: %d, ids: %d", layer_index, ids);

    ALOGI("hdr_en: %u", output->hdr_en);
    ALOGI("oetf_en: %u", output->oetf_en);

    PRINT_ARRAY("oetf_x", output->oetf_x, 33);
    PRINT_ARRAY("oetf_y", output->oetf_y, 33);

    ALOGI("eotf_en: %u", output->eotf_en);
    PRINT_ARRAY("eotf_x", output->eotf_x, 129);
    PRINT_ARRAY("eotf_y", output->eotf_y, 129);

    ALOGI("gm_en: %u", output->gm_en);
    PRINT_ARRAY("gm_coef", output->gm_coef, 9);

    ALOGI("tm_en: %u", output->tm_en);
    PRINT_ARRAY("tm_coef", output->tm_coef, 3);
    PRINT_ARRAY("tm_rngx", output->tm_rngx, 2);
    PRINT_ARRAY("tm_rngy", output->tm_rngy, 2);
    PRINT_ARRAY("tm_x", output->tm_x, 33);
    PRINT_ARRAY("tm_y", output->tm_y, 33);

    ALOGI("%s -", __func__);
}
