import React from "react";

import type { BackbufferProps } from "../../../dom/types";
import type { SkiaDefaultProps } from "../../processors";

export const Backbuffer = ({
  tx = "repeat",
  ty = "repeat",
  fm = "nearest",
  mm = "none",
  fit = "cover",
  transform = [],
  ...props
}: SkiaDefaultProps<
  BackbufferProps,
  "tx" | "ty" | "fm" | "mm" | "fit" | "transform"
>) => {
  return (
    <skBackbuffer
      tx={tx}
      ty={ty}
      fm={fm}
      mm={mm}
      fit={fit}
      transform={transform}
      {...props}
    />
  );
};
