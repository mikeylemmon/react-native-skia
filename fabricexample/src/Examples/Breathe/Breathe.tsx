import { CommonActions, useNavigation } from "@react-navigation/native";
import {
  Backbuffer,
  Canvas,
  Fill,
  ImageShader,
  mix,
  Shader,
  useImage,
  vec,
} from "@shopify/react-native-skia";
import React, { useEffect, useMemo } from "react";
import { Button, PixelRatio, useWindowDimensions, View } from "react-native";
import {
  cancelAnimation,
  Easing,
  interpolate,
  useDerivedValue,
  useSharedValue,
  withRepeat,
  withTiming,
} from "react-native-reanimated";

import { frag } from "../../components/ShaderLib";

function useTimeloop(secs: number) {
  // @refresh reset
  const progress = useSharedValue(0);
  useEffect(() => {
    progress.value = withRepeat(
      withTiming(1, { duration: secs * 1000, easing: Easing.linear }),
      -1,
      false
    );
    return () => {
      cancelAnimation(progress);
    };
  }, [progress, secs]);
  return progress;
}

const mixShader = frag`
  uniform shader image1;
  uniform shader image2;
  uniform float  progress;
  uniform float2 resolution;

  half4 main(vec2 xy) {
    vec2 uv = xy / resolution;
    return mix(
      image1.eval(uv * resolution),
      image2.eval(uv * resolution),
      progress
    );
  }
`;

const pd = PixelRatio.get();

export const Breathe = () => {
  const img1 = useImage(require("../Transitions/assets/4.jpg"));
  const nav = useNavigation();
  const { width, height } = useWindowDimensions();
  const [cx, cy] = [width / 2, height / 2];
  const [pw, ph] = [pd * width, pd * height];
  const [pcx, pcy] = [pd * cx, pd * cy];
  const center = useMemo(() => vec(pcx, pcy), [pcx, pcy]);

  const mixUniforms = useDerivedValue(() => ({
    progress: 0.05,
    resolution: [pw, ph],
  }));

  const progress = useTimeloop(30);

  const imageXform = useDerivedValue(
    () => [{ rotate: mix(progress.value, -Math.PI, Math.PI) }],
    [progress]
  );

  const bbXform = useDerivedValue(
    () => [
      {
        scale: interpolate(
          Math.sin(progress.value * Math.PI),
          [-1, 1],
          [0.94, 0.994]
        ),
      },
      {
        rotate:
          ((2 * Math.PI) / 3) *
          interpolate(
            Math.cos(progress.value * 2 * Math.PI),
            [-1, 1],
            [0.95, 1.04]
          ),
      },
    ],
    [progress]
  );

  return (
    <>
      <View style={{ position: "absolute", left: 30, top: 10, zIndex: 50 }}>
        <Button
          onPress={() => nav.dispatch(CommonActions.goBack())}
          title="Back"
        />
      </View>
      <Canvas style={{ flex: 1 }}>
        <Fill>
          <Shader source={mixShader} uniforms={mixUniforms}>
            <Backbuffer
              width={width}
              height={height}
              origin={center}
              transform={bbXform}
              // transform={[
              //   { scale: 0.92 },
              //   { rotate: ((2 * Math.PI) / 3) * 1.04 },
              // ]}
            />
            <ImageShader
              image={img1}
              fit="cover"
              width={width}
              height={height}
              transform={imageXform}
              origin={center}
            />
          </Shader>
        </Fill>
      </Canvas>
    </>
  );
};
