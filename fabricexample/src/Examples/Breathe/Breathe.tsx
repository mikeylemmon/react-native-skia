import { CommonActions, useNavigation } from "@react-navigation/native";
import {
  Backbuffer,
  Canvas,
  Circle,
  ColorShader,
  Fill,
  Group,
  ImageShader,
  Mask,
  mix,
  Paint,
  PaintStyle,
  Shader,
  Skia,
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
  uniform float  mixValue;
  uniform float2 resolution;

  half4 main(vec2 xy) {
    vec2 uv = xy / resolution;
    return mix(
      image1.eval(uv * resolution),
      image2.eval(uv * resolution),
      mixValue
    );
  }
`;

const paintShape = Skia.Paint();
paintShape.setColor(Skia.Color("#6622aa"));
paintShape.setStyle(PaintStyle.Stroke);
paintShape.setStrokeWidth(8);

const pd = PixelRatio.get();

export const Breathe = () => {
  const img = useImage(require("../Transitions/assets/1.jpg"));
  const nav = useNavigation();
  const { width, height } = useWindowDimensions();
  const [cx, cy] = [width / 2, height / 2];
  const [pw, ph] = [pd * width, pd * height];
  const [pcx, pcy] = [pd * cx, pd * cy];
  const center = useMemo(() => vec(pcx, pcy), [pcx, pcy]);

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
          [0.92, 0.97]
        ),
      },
      {
        rotate:
          ((2 * Math.PI) / 3) *
          interpolate(
            Math.cos(progress.value * 2 * Math.PI),
            [-1, 1],
            [0.95, 1.05]
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
          <Shader
            source={mixShader}
            uniforms={useDerivedValue(() => ({
              mixValue: 0.03,
              resolution: [pw, ph],
            }))}
          >
            <Backbuffer
              width={width}
              height={height}
              origin={center}
              transform={bbXform}
            />
            {/* <ImageShader
              image={img}
              fit="cover"
              width={width}
              height={height}
              transform={imageXform}
              origin={center}
            /> */}
            <ColorShader color="#000" />
          </Shader>
        </Fill>
        <Circle cx={cx} cy={cy - 140} r={60} paint={paintShape} />
      </Canvas>
    </>
  );
};
