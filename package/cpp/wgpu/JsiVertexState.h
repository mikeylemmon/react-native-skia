#pragma once
#include <memory>
#include <string>
#include <utility>

#include "webgpu.hpp"

#include <jsi/jsi.h>

#include "JsiEnums.h"
#include "JsiHostObject.h"
#include "JsiPromises.h"
#include "JsiShaderModule.h"
#include "JsiSkHostObjects.h"
#include "RNSkPlatformContext.h"

namespace RNSkia {

namespace jsi = facebook::jsi;

class JsiVertexState
    : public JsiSkWrappingSharedPtrHostObject<wgpu::VertexState> {
public:
  JsiVertexState(std::shared_ptr<RNSkPlatformContext> context,
                 wgpu::VertexState m)
      : JsiSkWrappingSharedPtrHostObject<wgpu::VertexState>(
            context, std::make_shared<wgpu::VertexState>(std::move(m))) {}

  EXPORT_JSI_API_BRANDNAME(JsiVertexState, VertexState)

  /**
   * Returns the underlying object from a host object of this type
   */
  static std::shared_ptr<wgpu::VertexState> fromValue(jsi::Runtime &runtime,
                                                      const jsi::Value &raw) {
    const auto &obj = raw.asObject(runtime);
    if (obj.isHostObject(runtime)) {
      return obj.asHostObject<JsiVertexState>(runtime)->getObject();
    } else {
      auto object = std::make_shared<wgpu::VertexState>();
      if (obj.hasProperty(runtime, "module")) {
        auto module = obj.getProperty(runtime, "module");

        object->module = *JsiShaderModule::fromValue(runtime, module).get();
      } else {
        throw jsi::JSError(runtime,
                           "Missing mandatory prop module in VertexState");
      }
      if (obj.hasProperty(runtime, "entryPoint")) {
        auto entryPoint = obj.getProperty(runtime, "entryPoint");

        object->entryPoint =
            entryPoint.getString(runtime).utf8(runtime).c_str();
      } else {
        throw jsi::JSError(runtime,
                           "Missing mandatory prop entryPoint in VertexState");
      }
      return object;
    }
  }
};
} // namespace RNSkia
