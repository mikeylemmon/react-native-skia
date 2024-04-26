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

class JsiTextureView
    : public JsiSkWrappingSharedPtrHostObject<wgpu::TextureView> {
public:
  JsiTextureView(std::shared_ptr<RNSkPlatformContext> context,
                 wgpu::TextureView m)
      : JsiSkWrappingSharedPtrHostObject<wgpu::TextureView>(
            context, std::make_shared<wgpu::TextureView>(std::move(m))) {}

  EXPORT_JSI_API_BRANDNAME(JsiTextureView, TextureView)

  /**
   * Returns the underlying object from a host object of this type
   */
  static std::shared_ptr<wgpu::TextureView> fromValue(jsi::Runtime &runtime,
                                                      const jsi::Value &raw) {
    const auto &obj = raw.asObject(runtime);
    if (obj.isHostObject(runtime)) {
      return obj.asHostObject<JsiTextureView>(runtime)->getObject();
    } else {
      throw jsi::JSError(runtime,
                         "Expected a JsiTextureView object, but got a " +
                             raw.toString(runtime).utf8(runtime));
    }
  }
};
} // namespace RNSkia
