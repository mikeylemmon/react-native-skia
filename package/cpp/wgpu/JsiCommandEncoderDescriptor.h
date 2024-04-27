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

class JsiCommandEncoderDescriptor
    : public JsiSkWrappingSharedPtrHostObject<wgpu::CommandEncoderDescriptor> {
public:
  JsiCommandEncoderDescriptor(std::shared_ptr<RNSkPlatformContext> context,
                              wgpu::CommandEncoderDescriptor m)
      : JsiSkWrappingSharedPtrHostObject<wgpu::CommandEncoderDescriptor>(
            context,
            std::make_shared<wgpu::CommandEncoderDescriptor>(std::move(m))) {}

  // TODO: this fix, use JSI_EXPORT_PROPERTY_GETTERS instead
  EXPORT_JSI_API_BRANDNAME(JsiCommandEncoderDescriptor,
                           CommandEncoderDescriptor)

  /**
   * Returns the underlying object from a host object of this type
   */
  static std::shared_ptr<wgpu::CommandEncoderDescriptor>
  fromValue(jsi::Runtime &runtime, const jsi::Value &raw) {
    const auto &obj = raw.asObject(runtime);
    if (obj.isHostObject(runtime)) {
      return obj.asHostObject<JsiCommandEncoderDescriptor>(runtime)
          ->getObject();
    } else {
      throw jsi::JSError(
          runtime, "Expected a JsiCommandEncoderDescriptor object, but got a " +
                       raw.toString(runtime).utf8(runtime));
    }
  }
};
} // namespace RNSkia
