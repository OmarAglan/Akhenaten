/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#pragma once

#include "filter.h"
#include "../core/gpupixel_macros.h"

NS_GPUPIXEL_BEGIN

class GrayscaleFilter : public Filter {
 public:
  static std::shared_ptr<GrayscaleFilter> create();
  bool init();

  virtual bool proceed(bool bUpdateTargets = true,
                       int64_t frameTime = 0) override;

 protected:
  GrayscaleFilter(){};
};

NS_GPUPIXEL_END
