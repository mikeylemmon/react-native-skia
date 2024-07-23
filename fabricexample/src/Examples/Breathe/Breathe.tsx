import {
  BlurMask,
  Canvas,
  Circle,
  Fill,
  Group,
  ImageShader,
  mix,
  PaintStyle,
  polar2Canvas,
  Shader,
  Skia,
  useCanvasRef,
  useImage,
  vec,
} from "@shopify/react-native-skia";
import React, { useEffect, useMemo } from "react";
import { PixelRatio, StyleSheet, useWindowDimensions } from "react-native";
import type { SharedValue } from "react-native-reanimated";
import { useDerivedValue } from "react-native-reanimated";

import { useLoop } from "../../components/Animations";
import { linear } from "../Transitions/transitions/linear";
import { transition } from "../Transitions/transitions/Base";

const linearMixShader = transition(linear);

const c1 = "#a1bea2";
const c2 = "#529ca0";

interface RingProps {
  index: number;
  progress: SharedValue<number>;
}

const numRings = 6;

const Ring = ({ index, progress }: RingProps) => {
  const { width, height } = useWindowDimensions();
  const R = width / 4;
  const center = useMemo(
    () => vec(width / 2, height / 2 - 64),
    [height, width]
  );

  const theta = (index * (2 * Math.PI)) / numRings;
  const transform = useDerivedValue(() => {
    const { x, y } = polar2Canvas(
      { theta, radius: progress.value * R },
      { x: 0, y: 0 }
    );
    const scale = mix(progress.value, 0.3, 1);
    return [{ translateX: x }, { translateY: y }, { scale }];
  }, [progress, R]);

  return (
    <Circle
      c={center}
      r={R}
      color={index % 2 ? c1 : c2}
      origin={center}
      transform={transform}
    />
  );
};

const pd = PixelRatio.get();

export const Breathe = () => {
  const { width, height } = useWindowDimensions();
  const center = useMemo(
    () => vec(width / 2, height / 2 - 64),
    [height, width]
  );

  const progress = useLoop({ duration: 3000 });

  const transform = useDerivedValue(
    () => [{ rotate: mix(progress.value, -Math.PI, 0) }],
    [progress]
  );
  const img1 = useImage(require("../Transitions/assets/2.jpg"));

  const unis = useDerivedValue(() => {
    return {
      // progress: progress.value * 0.94 + 0.03,
      progress: 0.05,
      resolution: [width, height],
    };
  });

  return (
    <Canvas style={{ flex: 1 }}>
      <Fill>
        <Shader source={linearMixShader} uniforms={unis}>
          <ImageShader
            image={null}
            fit="cover"
            width={width}
            height={height}
            // transform={[{ scale: 0.25 }]}
          />
          <ImageShader
            image={img1}
            fit="cover"
            width={width}
            height={height}
            transform={transform}
            origin={vec((pd * width) / 2, (pd * (height - 64)) / 2)}
          />
        </Shader>
      </Fill>
    </Canvas>
  );
};

// const styles = StyleSheet.create({
//   container: {
//     flex: 1,
//   },
// });

// const canvasRef = useCanvasRef();

// useEffect(() => {
//   setTimeout(() => {
//     const cc = canvasRef.current!;
//     console.log("Calling getSurface...");
//     const xx = cc.getSurface();
//     console.log("...got surface?", !!xx);
//     const bb = xx?.getCanvas();
//     console.log("...got canvas?", !!bb);
//     if (!xx || !bb) {
//       return;
//     }
//     console.log("Painting to backbuffer...");
//     const paint = Skia.Paint();
//     paint.setColor(Skia.Color("#6622bb"));
//     paint.setStyle(PaintStyle.Fill);
//     const pad = 200;
//     const xw = xx.width();
//     const xh = xx.height();
//     const ww = xw - pad * 2;
//     const hh = xh - pad * 2;
//     const rect = { x: pad, y: pad, width: ww, height: hh };
//     bb.clear(Skia.Color("#22bb77"));
//     bb.drawRect(rect, paint);
//     paint.setColor(Skia.Color("#2288bb"));
//     paint.setStyle(PaintStyle.Stroke);
//     paint.setStrokeWidth(25);
//     bb.drawLine(50, 50, xw / 2, xh / 2, paint);
//   }, 500);
// }, [canvasRef]);

// return (
//   <Canvas ref={canvasRef} style={styles.container} debug={true}>
//     <Fill color="rgb(36,43,56)" />
//     <Group origin={center} transform={transform} blendMode="screen">
//       <BlurMask style="solid" blur={40} />
//       {new Array(numRings).fill(0).map((_, index) => {
//         return <Ring key={index} index={index} progress={progress} />;
//       })}
//     </Group>
//   </Canvas>
// );
