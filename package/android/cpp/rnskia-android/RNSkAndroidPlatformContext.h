#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "JniPlatformContext.h"
#include "RNSkPlatformContext.h"
#include "SkiaOpenGLSurfaceFactory.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include "include/ports/SkFontMgr_android.h"

#pragma clang diagnostic pop

namespace RNSkia {
namespace jsi = facebook::jsi;

class RNSkAndroidPlatformContext : public RNSkPlatformContext {
public:
  RNSkAndroidPlatformContext(
      JniPlatformContext *jniPlatformContext, jsi::Runtime *runtime,
      std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker)
      : RNSkPlatformContext(runtime, jsCallInvoker,
                            jniPlatformContext->getPixelDensity()),
        _jniPlatformContext(jniPlatformContext) {
    // Hook onto the notify draw loop callback in the platform context
    jniPlatformContext->setOnNotifyDrawLoop(
        [this]() { notifyDrawLoop(false); });
  }

  ~RNSkAndroidPlatformContext() { stopDrawLoop(); }

  void performStreamOperation(
      const std::string &sourceUri,
      const std::function<void(std::unique_ptr<SkStreamAsset>)> &op) override {
    _jniPlatformContext->performStreamOperation(sourceUri, op);
  }

  void raiseError(const std::exception &err) override {
    _jniPlatformContext->raiseError(err);
  }

  sk_sp<SkSurface> makeOffscreenSurface(int width, int height) override {
    return SkiaOpenGLSurfaceFactory::makeOffscreenSurface(width, height);
  }

  sk_sp<SkFontMgr> createFontMgr() override {
    return SkFontMgr_New_Android(nullptr);
  }

  void runOnMainThread(std::function<void()> task) override {
    _jniPlatformContext->runTaskOnMainThread(task);
  }

  sk_sp<SkImage> takeScreenshotFromViewTag(size_t tag) override {
    return _jniPlatformContext->takeScreenshotFromViewTag(tag);
  }

  void startDrawLoop() override { _jniPlatformContext->startDrawLoop(); }

  void stopDrawLoop() override { _jniPlatformContext->stopDrawLoop(); }

  void registerSurfaceDescriptor(int nativeId, void *window, int width,
                                 int height) override {
    WGPUSurfaceDescriptorFromAndroidNativeWindow androidSurfaceDesc = {};
    androidSurfaceDesc.chain.sType =
        WGPUSType_SurfaceDescriptorFromAndroidNativeWindow;
    androidSurfaceDesc.window = window;

    WGPUSurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain =
        reinterpret_cast<const WGPUChainedStruct *>(&androidSurfaceDesc);

    wgpu::Instance instance = wgpuCreateInstance(nullptr);
    wgpu::Surface surface =
        wgpuInstanceCreateSurface(instance, &surfaceDesc);

    _descriptors[nativeId] = std::make_tuple(
        std::make_shared<wgpu::Surface>(surface), width, height);
  }

  virtual std::tuple<std::shared_ptr<wgpu::Surface>, int, int>
  getSurfaceDescriptor(int nativeId) override {
    auto it = _descriptors.find(nativeId);
    if (it != _descriptors.end()) {
      return it->second;
    }
    throw std::runtime_error("Surface descriptor not found");
  }
  // TODO: add a destroy method

private:
  JniPlatformContext *_jniPlatformContext;
  std::map<int, std::tuple<std::shared_ptr<wgpu::Surface>, int, int>>
      _descriptors;
};

} // namespace RNSkia
