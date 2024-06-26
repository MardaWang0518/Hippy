/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once

#include <memory>
#include <map>
#include "renderer/uimanager/hr_view_manager.h"
#include "renderer/virtual/hr_virtual_view_manager.h"

namespace hippy {
inline namespace render {
inline namespace native {

class HRManager {
public:
  HRManager(uint32_t instance_id, std::shared_ptr<NativeRender> &native_render);
  ~HRManager() = default;
  
//   void InitViewManager(uint32_t root_id);
  std::shared_ptr<HRViewManager> GetViewManager(uint32_t root_id);
  std::shared_ptr<HRVirtualViewManager> GetVirtualNodeManager(uint32_t root_id);

private:
  void AddViewManager(uint32_t root_id, std::shared_ptr<HRViewManager> &view_manager);
  void AddVirtualNodeManager(uint32_t root_id, std::shared_ptr<HRVirtualViewManager> &virtual_view_manager);

  uint32_t instance_id_;
  std::weak_ptr<NativeRender> native_render_;
  std::map<uint32_t, std::shared_ptr<HRViewManager>> view_manager_map_;
  std::map<uint32_t, std::shared_ptr<HRVirtualViewManager>> virtual_view_manager_map_;
};

} // namespace native
} // namespace render
} // namespace hippy
