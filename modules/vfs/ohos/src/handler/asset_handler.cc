/*
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2022 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vfs/handler/asset_handler.h"
#include <rawfile/raw_file_manager.h>
#include "footstone/check.h"
#include "footstone/logging.h"
#include "footstone/string_view_utils.h"

using string_view = footstone::string_view;
using StringViewUtils = footstone::StringViewUtils;

namespace hippy {
inline namespace vfs {

// TODO(hot):
static napi_env s_env;
static napi_ref s_ts_resource_manager_ref;
static NativeResourceManager *s_resource_manager;

void AssetHandler::Init(napi_env env, napi_ref ts_resource_manager_ref) {
  s_env = env;
  s_ts_resource_manager_ref = ts_resource_manager_ref;

  napi_value result;
  auto status = napi_get_reference_value(s_env, s_ts_resource_manager_ref, &result);
  if (status != napi_ok) {
    // TODO(hot):
    abort();
  }
  s_resource_manager = OH_ResourceManager_InitNativeResourceManager(s_env, result);
  if (!s_resource_manager) {
    // TODO(hot):
    abort();
  }
}

void AssetHandler::RequestUntrustedContent(
    std::shared_ptr<RequestJob> request,
    std::shared_ptr<JobResponse> response,
    std::function<std::shared_ptr<UriHandler>()> next) {
  // TODO(hot):
  std::string uri = request->GetUri().latin1_value(); // "asset:/vue2/vendor.android.js"
  size_t pos = uri.find(":");
  std::string pathName = uri.substr(pos+2);

  RawFile *file = OH_ResourceManager_OpenRawFile(s_resource_manager, pathName.c_str());
  if (!file) {
    response->SetRetCode(hippy::JobResponse::RetCode::ResourceNotFound);
  } else {
    response->SetRetCode(hippy::JobResponse::RetCode::Failed);
    long size = OH_ResourceManager_GetRawFileSize(file);
    if (size > 0) {
      size_t fileLen = static_cast<unsigned long>(size);
      char *buf = new char[fileLen];
      int len = OH_ResourceManager_ReadRawFile(file, buf, fileLen);
      if (len > 0) {
        response->GetContent().assign(buf, static_cast<size_t>(len));
        // OH_ResourceManager_ReleaseNativeResourceManager(s_resource_manager);
        response->SetRetCode(hippy::JobResponse::RetCode::Success);
      }
      delete[] buf;
    }
    OH_ResourceManager_CloseRawFile(file);
  }

  auto next_handler = next();
  if (next_handler) {
    next_handler->RequestUntrustedContent(request, response, next);
  }
}

void AssetHandler::RequestUntrustedContent(
    std::shared_ptr<RequestJob> request,
    std::function<void(std::shared_ptr<JobResponse>)> cb,
    std::function<std::shared_ptr<UriHandler>()> next) {

}

void AssetHandler::LoadByAsset(const string_view& path,
                               std::shared_ptr<RequestJob> request,
                               std::function<void(std::shared_ptr<JobResponse>)> cb,
                               std::function<std::shared_ptr<UriHandler>()> next,
                               bool is_auto_fill) {

}

}
}