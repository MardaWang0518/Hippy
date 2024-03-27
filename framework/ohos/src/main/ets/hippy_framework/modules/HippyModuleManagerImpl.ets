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
import { HippyException } from '../../support/common/HippyException'
import { HippyEngineContext } from '../HippyEngineContext'
import {
  HippyNativeModuleBase,
  AudioPlayerModule,
  ConsoleModule,
  StorageModule,
  DevMenuModule,
  DeviceEventModule,
  ExceptionModule,
  ImageLoaderModule,
  NetInfoModule,
  NetworkModule,
  WebSocketModule,
  UtilsModule
} from "./native"
import util from '@ohos.util'
import { HippyModuleManager } from './HippyModuleManager'
import { HippyAPIProvider } from '..'
import { HippyCallNativeParams } from '../bridge/HippyCallNativeParams'
import { LogUtils } from '../../support/utils/LogUtils'
import { HippyJavaScriptModuleBase } from './javascript/HippyJavaScriptModuleBase'
import { Dimensions } from './javascript/Dimensions'
import { EventDispatcher } from './javascript/EventDispatcher'
import { HippyAny, HippyArray, HippyValue } from '../../support/common/HippyTypes'

export type HippyNativeModuleCreator = (ctx: HippyEngineContext) => HippyNativeModuleBase
export type HippyJSModuleCreator = (ctx: HippyEngineContext) => HippyJavaScriptModuleBase

export function hippyNativeModuleCreatorRegisterMap(): Map<string, HippyNativeModuleCreator> {
  let registerMap: Map<string, HippyNativeModuleCreator> = new Map()
  registerMap.set(AudioPlayerModule.NAME, (ctx): HippyNativeModuleBase => new AudioPlayerModule(ctx))
  registerMap.set(ConsoleModule.NAME, (ctx): HippyNativeModuleBase => new ConsoleModule(ctx))
  registerMap.set(StorageModule.NAME, (ctx): HippyNativeModuleBase => new StorageModule(ctx))
  registerMap.set(DevMenuModule.NAME, (ctx): HippyNativeModuleBase => new DevMenuModule(ctx))
  registerMap.set(DeviceEventModule.NAME, (ctx): HippyNativeModuleBase => new DeviceEventModule(ctx))
  registerMap.set(ExceptionModule.NAME, (ctx): HippyNativeModuleBase => new ExceptionModule(ctx))
  registerMap.set(ImageLoaderModule.NAME, (ctx): HippyNativeModuleBase => new ImageLoaderModule(ctx))
  registerMap.set(NetInfoModule.NAME, (ctx): HippyNativeModuleBase => new NetInfoModule(ctx))
  registerMap.set(NetworkModule.NAME, (ctx): HippyNativeModuleBase => new NetworkModule(ctx))
  registerMap.set(WebSocketModule.NAME, (ctx): HippyNativeModuleBase => new WebSocketModule(ctx))
  registerMap.set(UtilsModule.NAME, (ctx): HippyNativeModuleBase => new UtilsModule(ctx))
  return registerMap
}

export function hippyJSModuleCreatorRegisterMap(): Map<string, HippyJSModuleCreator> {
  let registerMap: Map<string, HippyJSModuleCreator> = new Map()
  registerMap.set(Dimensions.MODULE_NAME, (ctx): HippyJavaScriptModuleBase => new Dimensions(ctx))
  registerMap.set(EventDispatcher.MODULE_NAME, (ctx): HippyJavaScriptModuleBase => new EventDispatcher(ctx))
  return registerMap
}

export class HippyModuleManagerImpl implements HippyModuleManager {
  private ctx: HippyEngineContext;
  private nativeModuleCreatorRegisterMap: Map<string, HippyNativeModuleCreator>;
  private jsModuleCreatorRegisterMap: Map<string, HippyJSModuleCreator>;
  private cachedNativeModuleMap: Map<string, HippyNativeModuleBase> = new Map();
  private cachedJSModuleMap: Map<string, HippyJavaScriptModuleBase> = new Map();
  private mEnableV8Serialization: boolean;

  constructor(ctx: HippyEngineContext, enableV8Serialization = false) {
    this.ctx = ctx;
    this.nativeModuleCreatorRegisterMap = hippyNativeModuleCreatorRegisterMap();
    this.jsModuleCreatorRegisterMap = hippyJSModuleCreatorRegisterMap();
    this.mEnableV8Serialization = enableV8Serialization;
  }

  getJavaScriptModule(name: string): HippyJavaScriptModuleBase | null {
    if (this.cachedJSModuleMap.has(name)) {
      return this.cachedJSModuleMap.get(name) as HippyJavaScriptModuleBase
    } else {
      let creator = this.jsModuleCreatorRegisterMap.get(name)
      if (creator) {
        let module = creator(this.ctx)
        this.cachedJSModuleMap.set(name, module)
        return module
      } else {
        return null
      }
    }
  }

  public callNatives(params: HippyCallNativeParams): void {
    const buffer = params.buffer;
    const moduleName = params.moduleName;
    const moduleFunc = params.moduleFunc;

    // callNative 参数反序列化
    let moduleParam = this.bytesToArgument(buffer, false);
    LogUtils.i('hippy', 'JsDriver callNatives moduleParam: ' + moduleParam);
    let module = this.getNativeModule(moduleName)
    if (module) {
      module.call(moduleFunc, moduleParam, null)
    }
  }

  public initModules(): void {

  }

  public destroy(): void {
    throw new Error('Method not implemented.')
  }

  public addModules(apiProviders: HippyAPIProvider[]): void {
    throw new Error('Method not implemented.')
  }

  public getNativeModule(name: string): HippyNativeModuleBase | null {
    if (this.cachedNativeModuleMap.has(name)) {
      return this.cachedNativeModuleMap.get(name) as HippyNativeModuleBase
    } else {
      let creator = this.nativeModuleCreatorRegisterMap.get(name)
      if (creator) {
        let module = creator(this.ctx)
        this.cachedNativeModuleMap.set(name, module)
        return module
      } else {
        return null
      }
    }
  }

  public bytesToArgument(buffer: ArrayBuffer, useJSValueType: boolean): Array<HippyAny> {
    let result = new Array<HippyAny>();
    if (buffer) {
      try {
        if (this.mEnableV8Serialization) {
          // result = this.parseV8SerializeData(buffer, useJSValueType);
        } else {
          result = this.parseJsonData(buffer);
        }
      } catch (e) {
        throw new HippyException("parse bytesToArgument error");
      }
    }
    return result;
  }

  // public parseV8SerializeData(buffer: ArrayBuffer, useJSValueType: boolean) {
  //
  // }

  public parseJsonData(buffer: ArrayBuffer): Array<HippyAny> {
    const bytes = new Uint8Array(buffer);
    const textDecoder = util.TextDecoder.create('utf-8')
    const str = textDecoder.decodeWithStream(bytes)
    const paramArray = JSON.parse(str) as Array<HippyAny>;
    return paramArray;
  }

}