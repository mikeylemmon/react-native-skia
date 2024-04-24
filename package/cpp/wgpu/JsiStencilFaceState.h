#pragma once
#include <memory>
#include <string>
#include <utility>

#include "webgpu.hpp"

#include <jsi/jsi.h>

#include "JsiEnums.h"
#include "JsiHostObject.h"
#include "JsiPromises.h"
#include "JsiSkHostObjects.h"
#include "RNSkPlatformContext.h"

namespace RNSkia {

namespace jsi = facebook::jsi;

class JsiStencilFaceState
    : public JsiSkWrappingSharedPtrHostObject<wgpu::StencilFaceState> {
public:
  JsiStencilFaceState(std::shared_ptr<RNSkPlatformContext> context,
                      wgpu::StencilFaceState m)
      : JsiSkWrappingSharedPtrHostObject<wgpu::StencilFaceState>(
            context, std::make_shared<wgpu::StencilFaceState>(std::move(m))) {}

  EXPORT_JSI_API_BRANDNAME(JsiStencilFaceState, StencilFaceState)

  /**
   * Returns the underlying object from a host object of this type
   */
  static std::shared_ptr<wgpu::StencilFaceState>
  fromValue(jsi::Runtime &runtime, const jsi::Value &raw) {
    const auto &obj = raw.asObject(runtime);
    if (obj.isHostObject(runtime)) {
      return obj.asHostObject<JsiStencilFaceState>(runtime)->getObject();
    } else {
      auto object = std::make_shared<wgpu::StencilFaceState>();
      if (obj.hasProperty(runtime, "compare")) {
        auto compare = obj.getProperty(runtime, "compare");
        object->compare = getCompareFunction(
            compare.getString(runtime).utf8(runtime).c_str());
      }
      if (obj.hasProperty(runtime, "failOp")) {
        auto failOp = obj.getProperty(runtime, "failOp");
        object->failOp = getStencilOperation(
            failOp.getString(runtime).utf8(runtime).c_str());
      }
      if (obj.hasProperty(runtime, "depthFailOp")) {
        auto depthFailOp = obj.getProperty(runtime, "depthFailOp");
        object->depthFailOp = getStencilOperation(
            depthFailOp.getString(runtime).utf8(runtime).c_str());
      }
      if (obj.hasProperty(runtime, "passOp")) {
        auto passOp = obj.getProperty(runtime, "passOp");
        object->passOp = getStencilOperation(
            passOp.getString(runtime).utf8(runtime).c_str());
      }
      return object;
    }
  }
};
} // namespace RNSkia
