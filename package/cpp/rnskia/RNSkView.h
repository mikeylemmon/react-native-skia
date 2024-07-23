
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "JsiValueWrapper.h"
#include "RNSkPlatformContext.h"

#include "JsiSkImage.h"
#include "JsiSkPoint.h"
#include "JsiSkRect.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

#pragma clang diagnostic pop

namespace RNSkia {

namespace jsi = facebook::jsi;

class RNSkCanvasProvider {
public:
  explicit RNSkCanvasProvider(std::function<void()> requestRedraw)
      : _requestRedraw(requestRedraw) {}

  /**
   Returns the scaled width of the view
   */
  virtual float getScaledWidth() = 0;

  /**
   Returns the scaled height of the view
   */
  virtual float getScaledHeight() = 0;

  /**
   Render to a canvas
   */
  virtual bool renderToCanvas(const std::function<void(SkCanvas *)> &) = 0;

protected:
  std::function<void()> _requestRedraw;
};

class RNSkRenderer {
public:
  explicit RNSkRenderer(std::function<void()> requestRedraw)
      : _requestRedraw(requestRedraw) {}

  /**
   Tries to render the current set of drawing operations. If we're busy we'll
   return false so that the calling RNSkBaseDrawView can request a new render
   next frame. The tryRender method is typically called on each frame if there
   are any redraw requests. The method will be called from the main thread, so
   the implementor must make sure any thread requirements are met before
   rendering. This method will also allow the rendering to be dispatched to
   another thread.
   */
  virtual bool
  tryRender(std::shared_ptr<RNSkCanvasProvider> canvasProvider) = 0;

  /**
   Renders directly to the canvas in the canvas provider. This method is called
   from a Javascript call to render a snapshot of the SkiaView to an image, and
   can therefore run outside the tryRender loop and directly in the javascript
   thread.
   */
  virtual void
  renderImmediate(std::shared_ptr<RNSkCanvasProvider> canvasProvider) = 0;

  void setShowDebugOverlays(bool showDebugOverlays) {
    _showDebugOverlays = showDebugOverlays;
  }
  bool getShowDebugOverlays() { return _showDebugOverlays; }

protected:
  std::function<void()> _requestRedraw;
  bool _showDebugOverlays;
};

class RNSkOffscreenCanvasProvider : public RNSkCanvasProvider {
public:
  RNSkOffscreenCanvasProvider(std::shared_ptr<RNSkPlatformContext> context,
                              std::function<void()> requestRedraw, float width,
                              float height)
      : RNSkCanvasProvider(requestRedraw), _width(width), _height(height) {
    _surface = context->makeOffscreenSurface(_width, _height);
    _pd = context->getPixelDensity();
  }

  /**
   Returns a snapshot of the current surface/canvas
   */
  sk_sp<SkImage> makeSnapshot(SkRect *bounds) {
    sk_sp<SkImage> image;
    if (bounds != nullptr) {
      SkIRect b =
          SkIRect::MakeXYWH(bounds->x() * _pd, bounds->y() * _pd,
                            bounds->width() * _pd, bounds->height() * _pd);
      image = _surface->makeImageSnapshot(b);
    } else {
      image = _surface->makeImageSnapshot();
    }
    return image->makeNonTextureImage();
  }

  /**
   Returns the scaled width of the view
   */
  float getScaledWidth() override { return _width; };

  /**
   Returns the scaled height of the view
   */
  float getScaledHeight() override { return _height; };

  /**
   Render to a canvas
   */
  bool renderToCanvas(const std::function<void(SkCanvas *)> &cb) override {
    cb(_surface->getCanvas());
    return true;
  };

  sk_sp<SkSurface> getSurface() { return _surface; };

private:
  float _width;
  float _height;
  float _pd = 1.0f;
  sk_sp<SkSurface> _surface;
};

enum RNSkDrawingMode { Default, Continuous };

using RNSkTouchInfo = struct {
  enum TouchType { Start, Active, End, Cancelled };
  double x;
  double y;
  double force;
  TouchType type;
  size_t id;
  long timestamp;
};

class RNSkView : public std::enable_shared_from_this<RNSkView> {
public:
  /**
   * Constructor
   */
  RNSkView(std::shared_ptr<RNSkPlatformContext> context,
           std::shared_ptr<RNSkCanvasProvider> canvasProvider,
           std::shared_ptr<RNSkRenderer> renderer)
      : _platformContext(context), _canvasProvider(canvasProvider),
        _renderer(renderer), _backbufferSwap(false) {}

  /**
   Destructor
   */
  virtual ~RNSkView() { endDrawingLoop(); }

  /**
   Sets custom properties. Custom properties are properties that are set
   directly from Javascript without having to go through the async bridge.
   */
  virtual void setJsiProperties(
      std::unordered_map<std::string, RNJsi::JsiValueWrapper> &props) {
    // Nothing here...
  }

  /**
   * Repaints the Skia view using the underlying context and the drawcallback.
   * This method schedules a draw request that will be run on the correct
   * thread and js runtime.
   */
  void requestRedraw() { _redrawRequestCounter++; }

  /**
   Renders immediate. Be carefull to not call this method from another thread
   than the UI thread
   */
  void renderImmediate() {
    _renderer->renderImmediate(_canvasProvider);
    _redrawRequestCounter = 0;
  }

  /**
   Sets the native id of the view
   */
  virtual void setNativeId(size_t nativeId) {
    _nativeId = nativeId;
    beginDrawingLoop();
  }

  /**
   Returns the native id
   */
  size_t getNativeId() { return _nativeId; }

  /**
   Sets the drawing mode for the view
   */
  void setDrawingMode(RNSkDrawingMode mode) {
    _drawingMode = mode;
    requestRedraw();
  }

  /**
   * Set to true to show the debug overlays on render
   */
  void setShowDebugOverlays(bool show) {
    _renderer->setShowDebugOverlays(show);
    requestRedraw();
  }

  /**
    Update touch state with new touch points
   */
  virtual void updateTouchState(std::vector<RNSkTouchInfo> &) {
    requestRedraw();
  }

  /**
   Renders the view into an SkImage instead of the screen.
   */
  sk_sp<SkImage> makeImageSnapshot(SkRect *bounds) {

    auto provider = std::make_shared<RNSkOffscreenCanvasProvider>(
        getPlatformContext(), std::bind(&RNSkView::requestRedraw, this),
        _canvasProvider->getScaledWidth(), _canvasProvider->getScaledHeight());

    _renderer->renderImmediate(provider);
    return provider->makeSnapshot(bounds);
  }

  std::shared_ptr<RNSkRenderer> getRenderer() { return _renderer; }

  sk_sp<SkSurface> getBackbuffer() {
    auto bb = _backbufferSwap ? _backbufferA : _backbufferB;
    // auto bb = _backbufferB;
    if (bb == nullptr) {
      return nullptr;
    }
    return bb->getSurface();
  }

protected:
  std::shared_ptr<RNSkPlatformContext> getPlatformContext() {
    return _platformContext;
  }
  std::shared_ptr<RNSkCanvasProvider> getCanvasProvider() {
    return _canvasProvider;
  }

  /**
   Ends an ongoing beginDrawCallback loop for this view. This method is made
   protected if the drawing loop should be stopped before reaching the
   destructor (like we do for Android views)
   */
  void endDrawingLoop() {
    if (_drawingLoopId != 0) {
      _drawingLoopId = 0;
      _platformContext->endDrawLoop(_nativeId);
    }
  }

private:
  /**
   Starts beginDrawCallback loop if the drawing mode is continuous
   */
  void beginDrawingLoop() {
    if (_drawingLoopId != 0 || _nativeId == 0) {
      return;
    }
    // Set to zero to avoid calling beginDrawLoop before we return
    _drawingLoopId = _platformContext->beginDrawLoop(
        _nativeId, [weakSelf = weak_from_this()](bool invalidated) {
          auto self = weakSelf.lock();
          if (self) {
            self->drawLoopCallback(invalidated);
          }
        });
  }

  /**
    Draw loop callback
   */
  void drawLoopCallback(bool invalidated) {
    // if (_redrawRequestCounter > 0 ||
    //     _drawingMode == RNSkDrawingMode::Continuous) {
    //   _redrawRequestCounter = 0;
    //
    //   if (!_renderer->tryRender(_canvasProvider)) {
    //     // The renderer could not render cause it was busy, just schedule
    //     // redrawing on the next frame.
    //     requestRedraw();
    //   }
    // }
    drawLoopCallbackBB(invalidated);
  }

  /**
    Draw loop callback (backbuffered)
   */
  std::shared_ptr<RNSkOffscreenCanvasProvider> _backbufferA;
  std::shared_ptr<RNSkOffscreenCanvasProvider> _backbufferB;
  bool _backbufferSwap;
  void drawLoopCallbackBB(bool invalidated) {
    if (_redrawRequestCounter == 0 && _drawingMode != RNSkDrawingMode::Continuous) {
      return;
    }
    _redrawRequestCounter = 0;

    if (_backbufferA == nullptr) {
      auto sw = _canvasProvider->getScaledWidth();
      auto sh = _canvasProvider->getScaledHeight();
      auto ww = sw > 8192 ? 8192 : sw;
      auto hh = sh > 8192 ? 8192 : sh;
      _backbufferA = std::make_shared<RNSkOffscreenCanvasProvider>(
        getPlatformContext(), std::bind(&RNSkView::requestRedraw, this),
        ww, hh);
      _backbufferB = std::make_shared<RNSkOffscreenCanvasProvider>(
        getPlatformContext(), std::bind(&RNSkView::requestRedraw, this),
        ww, hh);
    }
    auto renderDest = _backbufferSwap ? _backbufferB : _backbufferA;
    auto renderPrev = _backbufferSwap ? _backbufferA : _backbufferB;
    _backbufferSwap = !_backbufferSwap;
    _renderer->renderImmediate(renderDest);
    // _renderer->renderImmediate(_backbufferA);

    // auto bb = renderDest->getSurface();
    auto bb = renderPrev->getSurface();
    _canvasProvider->renderToCanvas([=](SkCanvas *canvas) {
      canvas->clear(SK_ColorTRANSPARENT);
      bb->draw(canvas, 0, 0);
    });

  }

  std::shared_ptr<RNSkPlatformContext> _platformContext;
  std::shared_ptr<RNSkCanvasProvider> _canvasProvider;
  std::shared_ptr<RNSkRenderer> _renderer;

  RNSkDrawingMode _drawingMode = RNSkDrawingMode::Default;
  size_t _nativeId;

  size_t _drawingLoopId = 0;
  std::atomic<int> _redrawRequestCounter = {1};
};

} // namespace RNSkia
