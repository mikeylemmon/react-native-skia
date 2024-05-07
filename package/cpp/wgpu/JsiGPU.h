#pragma once
#include <memory>
#include <string>
#include <utility>

#include "dawn/webgpu_cpp.h"

#include <jsi/jsi.h>

#include "JsiAdapter.h"
#include "JsiEnums.h"
#include "JsiHostObject.h"
#include "JsiPromises.h"
#include "JsiRequestAdapterOptions.h"
#include "JsiSkHostObjects.h"
#include "JsiTextureView.h"
#include "MutableJSIBuffer.h"
#include "RNSkLog.h"
#include "RNSkPlatformContext.h"

namespace RNSkia {

namespace jsi = facebook::jsi;

class JsiGPU : public JsiSkWrappingSharedPtrHostObject<wgpu::Instance> {
public:
  JsiGPU(std::shared_ptr<RNSkPlatformContext> context, wgpu::Instance m)
      : JsiSkWrappingSharedPtrHostObject<wgpu::Instance>(
            context, std::make_shared<wgpu::Instance>(std::move(m))) {}

  JSI_HOST_FUNCTION(requestAdapter) {
    auto defaultOptions = new wgpu::RequestAdapterOptions();
    auto options =
        count > 0 ? JsiRequestAdapterOptions::fromValue(runtime, arguments[0])
                  : defaultOptions;
    auto context = getContext();
    auto object = getObject();
    return RNJsi::JsiPromises::createPromiseAsJSIValue(
        runtime,
        [context = std::move(context), object = std::move(object),
         options = std::move(options)](
            jsi::Runtime &runtime,
            std::shared_ptr<RNJsi::JsiPromises::Promise> promise) -> void {
          wgpu::Adapter adapter = nullptr;
          object->RequestAdapter(
              nullptr,
              [](WGPURequestAdapterStatus, WGPUAdapter cAdapter,
                 const char *message, void *userdata) {
                if (message != nullptr) {
                  fprintf(stderr, "%s", message);
                  return;
                }
                *static_cast<wgpu::Adapter *>(userdata) =
                    wgpu::Adapter::Acquire(cAdapter);
              },
              &adapter);
          if (adapter == nullptr) {
            promise->resolve(jsi::Value::null());
          } else {
            device.SetUncapturedErrorCallback(
                [](WGPUErrorType type, const char *message, void *userdata) {
                  RNSkLogger::logToConsole("Error: %s", message);
                },
                nullptr);
            promise->resolve(jsi::Object::createFromHostObject(
                runtime, std::make_shared<JsiAdapter>(std::move(context),
                                                      std::move(adapter))));
          }
        });
  }

  JSI_HOST_FUNCTION(getPreferredCanvasFormat) {
    return jsi::String::createFromUtf8(runtime, "rgba8unorm");
  }

  // TODO: this fix, use JSI_EXPORT_PROPERTY_GETTERS instead
  EXPORT_JSI_API_BRANDNAME(JsiGPU, GPU)

  JSI_EXPORT_FUNCTIONS(JSI_EXPORT_FUNC(JsiGPU, requestAdapter),
                       JSI_EXPORT_FUNC(JsiGPU, getPreferredCanvasFormat))

  /**
   * Returns the underlying object from a host object of this type
   */
  static wgpu::Instance *fromValue(jsi::Runtime &runtime,
                                   const jsi::Value &raw) {
    const auto &obj = raw.asObject(runtime);
    if (obj.isHostObject(runtime)) {
      return obj.asHostObject<JsiGPU>(runtime)->getObject().get();
    } else {
      throw jsi::JSError(runtime, "Expected a JsiGPU object, but got a " +
                                      raw.toString(runtime).utf8(runtime));
    }
  }
};
} // namespace RNSkia
