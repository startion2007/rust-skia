#include "SkCanvas.h"
#include "SkImageInfo.h"
#include "SkSurface.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkTypes.h"

#if defined(SK_VULKAN)

#include "GrContext.h"
#include "GrBackendSurface.h"
#include "vk/GrVkBackendContext.h"

#endif

// SkSurface

extern "C" SkSurface* C_SkSurface_MakeRasterN32Premul(int width, int height, const SkSurfaceProps* surfaceProps) {
    return SkSurface::MakeRasterN32Premul(width, height, surfaceProps).release();
}

extern "C" SkSurface* C_SkSurface_MakeRenderTarget(
    GrContext* context,
    SkBudgeted budgeted,
    const SkImageInfo* imageInfo) {
    return SkSurface::MakeRenderTarget(context, budgeted, *imageInfo).release();
}

// SkImage

extern "C" SkImage* C_SkSurface_makeImageSnapshot(SkSurface* self) {
    return self->makeImageSnapshot().release();
}

// SkData

extern "C" SkData* C_SkImage_encodeToData(SkImage* self) {
    return self->encodeToData().release();
}

extern "C" void C_SkData_unref(const SkData* self) {
    self->unref();
}

// SkPaint

extern "C" void C_SkPaint_destruct(const SkPaint* self) {
    self->~SkPaint();
}

extern "C" void C_SkPath_destruct(const SkPath* self) {
    self->~SkPath();
}

extern "C" void C_SkCanvas_destruct(const SkCanvas* self) {
    self->~SkCanvas();
}

// SkImageInfo

extern "C" void C_SkImageInfo_Make(SkImageInfo* self, int width, int height, SkColorType ct, SkAlphaType at, SkColorSpace* cs) {
    *self = SkImageInfo::Make(width, height, ct, at, sk_sp<SkColorSpace>(cs));
}

extern "C" void C_SkImageInfo_MakeN32(SkImageInfo* self, int width, int height, SkAlphaType at, SkColorSpace* cs) {
    *self = SkImageInfo::MakeN32(width, height, at, sk_sp<SkColorSpace>(cs));
}

extern "C" void C_SkImageInfo_MakeN32Premul(SkImageInfo* self, int width, int height, SkColorSpace* cs) {
    *self = SkImageInfo::MakeN32Premul(width, height, sk_sp<SkColorSpace>(cs));
}

extern "C" void C_SkImageInfo_destruct(SkImageInfo* self) {
    self->~SkImageInfo();
}

// SkColorSpace

extern "C" SkColorSpace* C_SkColorSpace_MakeSRGB() {
    return SkColorSpace::MakeSRGB().release();
}

extern "C" SkColorSpace* C_SkColorSpace_MakeSRGBLinear() {
    return SkColorSpace::MakeSRGBLinear().release();
}

extern "C" SkColorSpace* C_SkColorSpace_MakeRGB(SkColorSpace::RenderTargetGamma gamma, SkColorSpace::Gamut gamut) {
    return SkColorSpace::MakeRGB(gamma, gamut).release();
}

extern "C" SkColorSpace* C_SkColorSpace_MakeRGB2(SkColorSpace::RenderTargetGamma gamma, const SkMatrix44* toXYZD50) {
    return SkColorSpace::MakeRGB(gamma, *toXYZD50).release();
}

extern "C" SkColorSpace* C_SkColorSpace_MakeRGB3(const SkColorSpaceTransferFn* coeffs, SkColorSpace::Gamut gamut) {
    return SkColorSpace::MakeRGB(*coeffs, gamut).release();
}

extern "C" SkColorSpace* C_SkColorSpace_MakeRGB4(const SkColorSpaceTransferFn* coeffs, const SkMatrix44* toXYZD50) {
    return SkColorSpace::MakeRGB(*coeffs, *toXYZD50).release();
}

extern "C" SkColorSpace* C_SkColorSpace_MakeRGB5(SkGammaNamed gammaNamed, const SkMatrix44* toXYZD50) {
    return SkColorSpace::MakeRGB(gammaNamed, *toXYZD50).release();
}

extern "C" SkColorSpace* C_SkColorSpace_makeLinearGamma(const SkColorSpace* self) {
    return self->makeLinearGamma().release();
}

extern "C" SkColorSpace* C_SkColorSpace_makeSRGBGamma(const SkColorSpace* self) {
    return self->makeSRGBGamma().release();
}

extern "C" SkColorSpace* C_SkColorSpace_makeColorSpin(const SkColorSpace* self) {
    return self->makeColorSpin().release();
}

extern "C" SkData* C_SkColorSpace_serialize(const SkColorSpace* self) {
    return self->serialize().release();
}

extern "C" SkColorSpace* C_SkColorSpace_Deserialize(const void* data, size_t length) {
    return SkColorSpace::Deserialize(data, length).release();
}

#if defined(SK_VULKAN)

extern "C" SkSurface* C_SkSurface_MakeFromBackendTexture(
    GrContext* context,
    const GrBackendTexture* backendTexture,
    GrSurfaceOrigin origin,
    int sampleCnt,
    SkColorType colorType) {
    return SkSurface::MakeFromBackendTexture(context, *backendTexture, origin, sampleCnt, colorType, nullptr, nullptr).release();
}

extern "C" void C_SkSurface_getBackendTexture(
        SkSurface* self,
        SkSurface::BackendHandleAccess handleAccess,
        GrBackendTexture* backendTexture) {
    *backendTexture = self->getBackendTexture(handleAccess);
}

extern "C" void C_GrBackendTexture_destruct(const GrBackendTexture* self) {
    self->~GrBackendTexture();
}

// The GrVkBackendContext struct binding's length is too short
// because of the std::function that is used in it.

typedef PFN_vkVoidFunction (*GetProcFn)(const char* name, VkInstance instance, VkDevice device);
typedef const void* (*GetProcFnVoidPtr)(const char* name, VkInstance instance, VkDevice device);

extern "C" void* C_GrVkBackendContext_New(
        void* instance,
        void* physicalDevice,
        void* device,
        void* queue,
        uint32_t graphicsQueueIndex,

        /* PFN_vkVoidFunction makes us trouble on the Rust side */
        GetProcFnVoidPtr getProc) {

    auto& context = *new GrVkBackendContext();
    context.fInstance = static_cast<VkInstance>(instance);
    context.fPhysicalDevice = static_cast<VkPhysicalDevice>(physicalDevice);
    context.fDevice = static_cast<VkDevice>(device);
    context.fQueue = static_cast<VkQueue>(queue);
    context.fGraphicsQueueIndex = graphicsQueueIndex;

    context.fGetProc = *(reinterpret_cast<GetProcFn*>(&getProc));
    return &context;
}

extern "C" void C_GrVkBackendContext_Delete(void* vkBackendContext) {
    delete static_cast<GrVkBackendContext*>(vkBackendContext);
}

extern "C" GrContext* C_GrContext_MakeVulkan(const void* vkBackendContext) {
    return GrContext::MakeVulkan(*static_cast<const GrVkBackendContext*>(vkBackendContext)).release();
}

#endif
