# HidumperFuncHap — hidumper 功能测试专用靶子应用

> 本 HAP 是 `test/unittest/common/hidumper_function_test.cpp`（HidumperFunctionTest）的
> **确定性 dump 靶子**，仿照 hiviewdfx_hiview PR !4552（faultlogger_hap）与
> faultloggerd `test/resource` 的仓内测试 HAP 范式。

## 为什么需要它

hidumper 的快照类命令（`--mem-jsheap` / `--mem-heap --native|--kotlin|--jsvm|--arkweb-js` /
`--mem-cjheap`）经 appmgr 路由，**只能送达 appspawn 派生并注册进 `appRunningRecordMap_`
的应用进程**（`app_running_manager.cpp:1412`：查不到 pid → "no application found"）。
裸二进制收不到 dump 请求，因此快照靶子必须是一个应用。本 HAP 让 UT 不再依赖
"设备上碰巧在跑的第三方应用"。

## 提供的靶子（entry/src/main/cpp/hidumper_target.cpp + ets/pages/Index.ets）

| 靶子 | 实现 | 服务的 hidumper 命令 |
|---|---|---|
| ArkTS 堆 ballast | Index.ets setInterval 填充字符串数组 | `--mem-jsheap`（含 `--raw/--single/--gc/--clean/--leakobj/-T`） |
| 自驱动 JSVM isolate | native 线程 Init→CreateVM→OpenVMScope→CreateEnv→OpenEnvScope→**OpenHandleScope**→循环 RunScript+PumpMessageLoop（50ms） | `--mem-heap --jsvm`（无需 UI 驱动） |
| KMP mem-dump listener | `dlopen("libohhidebug.so")` + `dlsym("OH_HiDebug_RegisterMemDumpListener")`（API 26），注册名 `"KMP"`，回调向 fd 写 ~10.5KB 合成快照（>1024B 门槛） | `--mem-heap --kotlin`（产物 `.kdump`） |
| ArkWeb `:render` 进程 | Index.ets Web 组件加载 rawfile 页（100ms JS ballast） | `--mem-heap --arkweb-js`（对 render pid） |
| native malloc churn | 64KB×128 池 | `--mem <pid>` 观测（**不作快照靶**：memleak tracker 不为第三方应用记录分配栈，nativeheap 产物仅 NSNAP1.0 空头部 416B） |
| 崩溃触发（ohosTest） | `CrashTest#SegvCase` → `TriggerSegv()` 空指针写 | `-e / -e --list / -e --print` 自造 cppcrash 记录 |

### 已验证的关键事实（勿重新摸底）
- **JSVM HandleScope 契约**：手机侧 libjsvm（node-api 契约）在任何 handle 创建前要求显式
  `OH_JSVM_OpenHandleScope`，否则引擎层 `HandleScope::Extend` → SIGTRAP abort；OH 新版源码
  由 JsvmApiEnterScope 内部管理。本 HAP 全序列打开，两端实证通过。
- **libjsvm 不得链接只能 dlopen**：32 位板端镜像不带 libjsvm.so，DT_NEEDED 会拖死整个
  napi 模块（jscrash "Cannot read property start of undefined"）。
- **通用测试证书双端可装**：hapsigner dist 材料签名的 HAP 在 OpenHarmony 板（RK3568）与
  商用 log 版手机（SCA-AL00）均 `bm install` 成功（实证 2026-10-20）。
- **RK 无 hisysevent fault 管道**：以测试命令触发崩溃后 cppcrash 文件生成，但 `-e --list` 始终
  "no records found."（AppExitReason 记录缺失）→ UT 对该情形断言空态诊断而非跳过。

## 构建（GN，仓内正式路径）

```bash
# 源码树根目录
prebuilts/build-tools/linux-x86/bin/ninja -C out/<product> \
    hidumper_func_hap_resource hidumper_func_hap_ohos_test
# 产物: out/<product>/tests/unittest/hidumper/hidumper/resource/
#   HidumperFuncHap.hap  HidumperFuncHapOhosTest.hap（已用通用证书签名）
```

`HidumperFunctionTest` 的 BUILD.gn 已 deps 本目录 group 并挂
`test/resource/ohos_test.xml`（testfwk 自动推送两个 HAP 到
`/data/test/resource/hidumper/`；UT 的 `SetUpTestCase` 幂等 `bm install`）。

## 签名证书再生成

`signature/openharmony_sx.p7b` 与 `AppScope/app.json5` 的 bundleName
（`ohos.samples.hidumperfunc`）**绑定**。改包名必须重跑：

```bash
bash generate_signature.sh   # 需要 java 11+ 与 developtools/hapsigner 仓
```

材料 = `developtools/hapsigner/dist/`（OpenHarmony.p12 等，与 GN 默认签名同源，
见 `build/ohos_var.gni` default_key_alias/default_keystore_path）。

## 手动安装（不走 testfwk 时）

```bash
hdc -t <serial> file send HidumperFuncHap.hap /data/test/resource/hidumper/
hdc -t <serial> file send HidumperFuncHapOhosTest.hap /data/test/resource/hidumper/
# UT 启动时自动安装；或手动：
hdc -t <serial> shell "bm install -p /data/test/resource/hidumper/HidumperFuncHap.hap"
hdc -t <serial> shell "bm install -p /data/test/resource/hidumper/HidumperFuncHapOhosTest.hap"
# 启动 HAP / 崩溃触发（造 faultlog 记录）：用系统能力管理 CLI 的 start / test 子命令，
# 完整命令行见 hidumper_function_test.cpp 的 StartFuncTestHap() 与 CRASH_TEST_CMD 常量
```

## 已知环境坑（本地构建）

- hvigor 对联网机器会做 SDK license 校验且 prebuilts SDK 无 licenses/*.sha256 →
  离线（CI）环境自然通过；联网本地构建需自行补齐 SDK licenses 或改用离线方式。
- `compileSdkVersion`（26.1.0）必须命中编译机 `prebuilts/ohos-sdk/linux/` 实存
  SDK 的 `platformVersion`：API26 起 hvigor 以各组件 `oh-uni-package.json` 的
  `platformVersion`（点分制）做**精确**版本匹配（sdkmanager-common
  `configFullApiVersion`/`compareApiVersion`，按 major.minor.patch 数值比较，
  非前缀/区间）。本 HAP 含 native（`entry/src/main/cpp`），故 ets/js/toolchains/
  previewer/native 五个组件的 `platformVersion` 必须**一致**等于 pin，任一不符
  → hvigor 联网下载缺失组件 → 被 SDK license 校验拦截（dayu200_tdd 实证）。
  - pin 定 "26.1.0"：与树内先例（power_dialog、telephony_data、Settings_FaceAuth
    三个 HAP 均 26.1.0）及 master `api_full_version`（build/version.gni）、官方
    `ohos-sdk-full-linux` 下载版本（build/prebuilts_config.json）一致。
  - 坑：daily-build Canary 快照可能**内部不一致**（native 先升到 "26.1.0"、
    ets/js/toolchains/previewer 仍是 "26"≡26.0.0），此时任何单一 pin 都建不出
    含 native 的 HAP（26.1.0 缺那 4 个、26.0.0 缺 native）。CI 池用的是五组件
    一致的 26.1.0；本地若拿到不一致 Canary，需把 4 个滞后组件的 `platformVersion`
    归一到 "26.1.0" 才能本地复现（`26/`=26.0.0.25 meta 3.0.0 与 hvigor 6 不兼容，
    "The SDK management mode has changed"）。
- CI precise（tdd）池经 tool_dayu200 `copy_sdk.sh` 将池级 `prebuilts/ohos-sdk/`
  复制进构建工作区；HAP 目标不再从 precise 构建图排除（历史上曾因池内无可用
  26.x SDK 而以 `ohos_module_precise_build` 条件隔离，池更新到一致 26.1.0 后解除，
  precise gn 二进制会据 changed-file→target 映射把两个 ohos_app 动作选入 `precise`）。
