import React, { useEffect, useRef, useState } from "react";
import { gpu, SkiaDomView, useImage } from "@shopify/react-native-skia";

import { demo3 } from "./demo3";
import { demo2 } from "./demo2";
import { demo1 } from "./demo1/main";
import { demo4 } from "./demo1/demo4";
import { demo5 } from "./demo1/demo5";
import { Bitmap, demo7 } from "./demo1/demo7";
import { demo6 } from "./demo6/main";
//import { demo6 } from "./demo6/main";

const draw = async (context: GPUCanvasContext, image: Bitmap) => {
  const adapter = await gpu.requestAdapter();
  const device = await adapter!.requestDevice();
  const presentationFormat = "rgba8unorm";
  context.configure({
    device,
    format: presentationFormat,
    alphaMode: "premultiplied",
  });
  //await demo5(device, context, image);
  await demo6(device, context, image);
  //context.present();
};

const usePixels = (mod: number) => {
  const image = useImage(mod);
  if (!image) {
    return null;
  }
  return new Bitmap(image.readPixels() as Uint8Array, image.width(), image.height());
};

export const Breathe = () => {
  const image = usePixels(require("./demo6/webgpu.png"));
  const ref = useRef<SkiaDomView>(null);
  useEffect(() => {
    setTimeout(() => {
      if (image && ref.current) {
        const ctx = ref.current.getWGPUContext();
        draw(ctx, image);
      }
    }, 1000);
  }, [image]);
  return <SkiaDomView style={{ flex: 1 }} ref={ref} />;
};
