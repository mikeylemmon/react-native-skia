import {
  BlurMask,
  Canvas,
  Circle,
  Fill,
  Group,
  mix,
  polar2Canvas,
  Skia,
  useCanvasRef,
  vec,
} from "@shopify/react-native-skia";
import React, { useEffect, useMemo } from "react";
import { StyleSheet, useWindowDimensions } from "react-native";
import type { SharedValue } from "react-native-reanimated";
import { useDerivedValue } from "react-native-reanimated";

import { useLoop } from "../../components/Animations";

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

  // const offA = useRef(Skia.Surface.MakeOffscreen(width, height));
  // const offB = useRef(Skia.Surface.MakeOffscreen(width, height));
  // const ab = useRef(-1);
  // ab.current *= -1;
  // const off = ab.current < 0 ? offA : offB;
  // const offShad = off.current
  //   ?.makeImageSnapshot()
  //   .makeShaderOptions(
  //     TileMode.Repeat,
  //     TileMode.Repeat,
  //     FilterMode.Nearest,
  //     MipmapMode.None
  //   );

  // const bufs = useMemo(() => {
  //   const offA = Skia.Surface.MakeOffscreen(width, height)!;
  //   const paint = Skia.Paint();
  //   paint.setColor(Skia.Color("pink"));
  //   const cA = offA.getCanvas();
  //   cA.clear(Skia.Color("green"));
  //   cA.drawRect({ x: 50, y: 50, width: 100, height: 100 }, paint);
  //   return { offA };
  // }, [width, height]);

  const canvasRef = useCanvasRef();

  useEffect(() => {
    const cc = canvasRef.current!;
    console.log("Calling getSurface...");
    const xx = cc.getSurface();
    console.log("...got surface?", xx);
  }, [canvasRef]);
  // }, [bufs, canvasRef]);

  return (
    <Canvas ref={canvasRef} style={styles.container} debug={true}>
      <Fill color="rgb(36,43,56)" />
      <Group origin={center} transform={transform} blendMode="screen">
        <BlurMask style="solid" blur={40} />
        {new Array(numRings).fill(0).map((_, index) => {
          return <Ring key={index} index={index} progress={progress} />;
        })}
      </Group>
    </Canvas>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
});
