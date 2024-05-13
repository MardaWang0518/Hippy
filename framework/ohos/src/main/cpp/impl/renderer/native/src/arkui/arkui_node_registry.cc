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

#include "renderer/arkui/arkui_node_registry.h"
#include "renderer/arkui/arkui_node.h"
#include "renderer/arkui/native_node_api.h"
#include "footstone/logging.h"

namespace hippy {
inline namespace render {
inline namespace native {

ArkUINodeRegistry& ArkUINodeRegistry::GetInstance() {
  static std::unique_ptr<ArkUINodeRegistry> instance = nullptr;
  if (instance == nullptr) {
    instance = std::unique_ptr<ArkUINodeRegistry>(new ArkUINodeRegistry());
  }
  return *instance;
}

void ArkUINodeRegistry::RegisterNode(ArkUINode *node) {
  auto [_it, inserted] = nodesByHandle_.emplace(node->GetArkUINodeHandle(), node);
  if (!inserted) {
    FOOTSTONE_LOG(WARNING) << "Node with handle " << node->GetArkUINodeHandle() << " was already registered";
  }
}

void ArkUINodeRegistry::UnregisterNode(ArkUINode *node) {
  auto it = nodesByHandle_.find(node->GetArkUINodeHandle());
  if (it == nodesByHandle_.end()) {
    FOOTSTONE_LOG(WARNING) << "Node with handle " << node->GetArkUINodeHandle() << " not found";
    return;
  }

  nodesByHandle_.erase(it);
}

void ArkUINodeRegistry::RegisterTouchHandler(ArkUINode *node, TouchEventHandler *touchEventHandler) {
  FOOTSTONE_DLOG(INFO) << "Register touch handler for node handle " << node->GetArkUINodeHandle();
  auto [_it, inserted] = touchHandlersByNodeHandle_.emplace(node->GetArkUINodeHandle(), touchEventHandler);
  if (!inserted) {
    FOOTSTONE_LOG(WARNING) << "Touch handler for node handle " << node->GetArkUINodeHandle() << " was already registered";
  }
}

void ArkUINodeRegistry::UnregisterTouchHandler(ArkUINode* node) {
  FOOTSTONE_DLOG(INFO) << "Unregister touch handler for node handle " << node->GetArkUINodeHandle();
  auto it = touchHandlersByNodeHandle_.find(node->GetArkUINodeHandle());
  if (it == touchHandlersByNodeHandle_.end()) {
    FOOTSTONE_LOG(WARNING) << "Touch handler for node handle " << node->GetArkUINodeHandle() << " not found";
    return;
  }
  touchHandlersByNodeHandle_.erase(it);
}

ArkUINodeRegistry::ArkUINodeRegistry() {
  NativeNodeApi::GetInstance()->registerNodeEventReceiver([](ArkUI_NodeEvent* event) {
    ArkUINodeRegistry::GetInstance().ReceiveEvent(event);
  });
}

void ArkUINodeRegistry::ReceiveEvent(ArkUI_NodeEvent *event) {
  try {
    if (event->kind == ArkUI_NodeEventType::NODE_TOUCH_EVENT) {
      auto it = touchHandlersByNodeHandle_.find(event->node);
      if (it == touchHandlersByNodeHandle_.end()) {
        FOOTSTONE_LOG(WARNING) << "Touch event for node with handle " << event->node << " not found";
        return;
      }

      it->second->OnTouchEvent(event->touchEvent);
      return;
    }

    auto it = nodesByHandle_.find(event->node);
    if (it == nodesByHandle_.end()) {
      FOOTSTONE_LOG(WARNING) << "Node with handle " << event->node << " not found";
      return;
    }

    it->second->OnNodeEvent(event);
  } catch (std::exception& e) {
    // TODO(hot):
  }
}

} // namespace native
} // namespace render
} // namespace hippy