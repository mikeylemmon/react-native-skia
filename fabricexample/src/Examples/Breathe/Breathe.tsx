import { CommonActions, useNavigation } from "@react-navigation/native";
import {
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
  useDerivedValue,
  useSharedValue,
  withRepeat,
  withTiming,
} from "react-native-reanimated";

import { frag } from "../../components/ShaderLib";

const linearMixShader = frag`
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
  const nav = useNavigation();
  const { width, height } = useWindowDimensions();
  const [cx, cy] = [width / 2, height / 2];
  const [pw, ph] = [pd * width, pd * height];
  const [pcx, pcy] = [pd * cx, pd * cy];
  const center = useMemo(() => vec(pcx, pcy), [pcx, pcy]);

  const progress = useSharedValue(0);
  useEffect(() => {
    // @refresh reset
    progress.value = withRepeat(
      withTiming(1, { duration: 18000, easing: Easing.linear }),
      -1,
      false
    );
    return () => {
      cancelAnimation(progress);
    };
  }, [progress]);

  const transform = useDerivedValue(
    () => [{ rotate: mix(progress.value, -Math.PI, Math.PI) }],
    [progress]
  );
  const img1 = useImage(require("../Transitions/assets/4.jpg"));
  const unis = useDerivedValue(() => ({
    progress: 0.05,
    resolution: [pw, ph],
  }));

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
          <Shader source={linearMixShader} uniforms={unis}>
            <ImageShader
              image={null}
              fit="contain"
              width={width}
              height={height}
              transform={[{ scale: 1.018 }, { translateY: -10 }]}
              origin={center}
            />
            <ImageShader
              image={img1}
              fit="cover"
              width={width}
              height={height}
              transform={transform}
              origin={center}
            />
          </Shader>
        </Fill>
      </Canvas>
    </>
  );
};
