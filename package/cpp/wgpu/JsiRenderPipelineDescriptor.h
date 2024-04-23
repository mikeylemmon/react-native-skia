#pragma once

#include "webgpu.hpp"

#include <jsi/jsi.h>

#include "JsiHostObject.h"
#include "JsiPromises.h"
#include "JsiSkHostObjects.h"
#include "RNSkPlatformContext.h"

namespace RNSkia {

namespace jsi = facebook::jsi;

class JsiRenderPipelineDescriptor
    : public JsiSkWrappingSharedPtrHostObject<wgpu::RenderPipelineDescriptor> {
public:
  JsiRenderPipelineDescriptor(std::shared_ptr<RNSkPlatformContext> context,
                              wgpu::RenderPipelineDescriptor m)
      : JsiSkWrappingSharedPtrHostObject<wgpu::RenderPipelineDescriptor>(
            context,
            std::make_shared<wgpu::RenderPipelineDescriptor>(std::move(m))) {}

  EXPORT_JSI_API_BRANDNAME(JsiRenderPipelineDescriptor,
                           RenderPipelineDescriptor)

  /**
   * Returns the underlying object from a host object of this type
   */
  static std::shared_ptr<wgpu::RenderPipelineDescriptor>
  fromValue(jsi::Runtime &runtime, const jsi::Value &raw) {
    const auto &obj = raw.asObject(runtime);
    if (obj.isHostObject(runtime)) {
      return obj.asHostObject<JsiRenderPipelineDescriptor>(runtime)
          ->getObject();
    } else {
      wgpu::RenderPipelineDescriptor object;
      if (obj.hasProperty(runtime, "label")) {
        auto label = obj.getProperty(runtime, "label");
        object.label = obj.getString(runtime).utf8(runtime).c_str();
      }
      if (obj.hasProperty(runtime, "layout")) {
        auto layout = obj.getProperty(runtime, "layout");
        object.layout = JsiPipelineLayout::fromValue(runtime, obj);
      }
      if (obj.hasProperty(runtime, "vertex")) {
        auto vertex = obj.getProperty(runtime, "vertex");
        object.vertex = JsiVertexState::fromValue(runtime, obj);
      } else {
        throw jsi::JSError(runtime, "Missing mandatory prop vertex in vertex");
      }
      if (obj.hasProperty(runtime, "primitive")) {
        auto primitive = obj.getProperty(runtime, "primitive");
        object.primitive = JsiPrimitiveState::fromValue(runtime, obj);
      } else {
        throw jsi::JSError(runtime,
                           "Missing mandatory prop primitive in primitive");
      }
      if (obj.hasProperty(runtime, "depth stencil")) {
        auto depthStencil = obj.getProperty(runtime, "depth stencil");
        object.depthStencil = JsiDepthStencilState::fromValue(runtime, obj);
      }
      if (obj.hasProperty(runtime, "multisample")) {
        auto multisample = obj.getProperty(runtime, "multisample");
        object.multisample = JsiMultisampleState::fromValue(runtime, obj);
      } else {
        throw jsi::JSError(runtime,
                           "Missing mandatory prop multisample in multisample");
      }
      if (obj.hasProperty(runtime, "fragment")) {
        auto fragment = obj.getProperty(runtime, "fragment");
        object.fragment = JsiFragmentState::fromValue(runtime, obj);
      }
    }
  }
};
} // namespace RNSkia
