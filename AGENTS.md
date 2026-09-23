# AGENTS.md

本文件为 `hidumper` 仓库定义 Agent 工作规则。

## 1. 范围与优先级

- 本文件适用于 `OpenHarmony/base/hiviewdfx/hidumper`。
- 用户直接指令优先级高于本文件。
- 核心原则：**代码优先、证据优先、不臆造**。

## 2. 项目定位

HiDumper 是 OpenHarmony 面向开发、测试与 IDE 工具的统一系统信息导出工具，用于分析与定位问题。主要组成：

- **hidumper**：CLI 可执行程序，接收用户输入，通过 IPC 与服务端通讯。
- **DumpManagerService**（SAID 1212）：导出管理服务，负责信息分类、命令分发与执行编排。
- **DumpManagerCpuService**（SAID 1215，可选）：CPU 使用信息专用服务（依赖 `hidumper_hiviewdfx_hiview_enable`）。
- **Dumpers / Executor**：各类信息导出器（命令、CPU、内存、SA、存储、网络、事件等）。
- **Output**：内容输出（FdOutput / ZipOutput 等）。

### 2.1 按任务类型定位代码

| 任务类型 | 目录 | 关键文件 |
|---|---|---|
| CLI 入口 / 命令行参数解析 | `client/native/` | `main.cpp`、`dump_client_main.cpp` |
| CLI 参数 → 执行器编排 | `frameworks/native/` | `include/manager/dump_implement.h`、`src/manager/dump_implement.cpp` |
| 新增 / 修改某种导出器 | `frameworks/native/include/executor/` + `src/executor/` | `*_dumper.h/.cpp`（如 `cpu_dumper`、`sa_dumper`、`memory_dumper`） |
| 新增 / 修改导出器工厂 | `frameworks/native/include/factory/` + `src/factory/` | `*_factory.h/.cpp` |
| 内存信息导出（smaps/ashmem/dmabuf/jsheap/cjheap） | `frameworks/native/include/executor/memory/` | `memory_info.h`、`parse/*`、`dump_*heap_info.h` |
| 主服务（SA 1212）实现 | `services/native/` | `include/dump_manager_service.h`、`src/dump_manager_service.cpp` |
| CPU 服务（SA 1215）实现 | `services/native/` | `include/dump_manager_cpu_service.h`、`src/dump_manager_cpu_service.cpp` |
| 手写 IPC（IDumpBroker）stub/proxy | `services/zidl/` | `dump_broker_stub.h/.cpp`、`dump_broker_proxy.cpp` |
| IDL 生成 IPC（CPU 服务） | `services/IHidumperCpuService.idl`（源），生成物在 `${target_gen_dir}` | `IHidumperCpuService.idl` |
| 客户端代理 / 单例 | `interfaces/native/innerkits/include/` | `dump_manager_client.h`、`dump_broker_proxy.h`、`idump_broker.h` |
| 内存使用量 inner API | `interfaces/innerkits/` | `include/dump_usage.h`、`dump_usage.cpp` |
| 插件（gpumem 查询等） | `plugins/` | `dumper_plugin.h`、`dumper_plugin_host.cpp` |
| 权限校验 | `utils/native/` | `include/permission.h`、`src/permission.cpp` |
| 日志宏定义 | `utils/native/include/` | `hilog_wrapper.h` |
| 返回状态枚举 | `frameworks/native/` | `common.h`（`DumpStatus`） |
| 运行配置 / SELinux / init | `services/native/etc/`、`sa_profile/` | `hidumper_service.rc/.cfg`、`1212.json`、`infos_config.json` |
| 单元测试 | `test/unittest/common/` | `*_test.cpp` |
| Fuzz 测试 | `test/fuzztest/` | `*_fuzzer.cpp` |
| Python 集成测试（不进 GN 构建） | `test/scripts/` | `testModule/test_*.py` |

### 2.2 嵌套指引

本仓库无目录级别的嵌套 Agent 指引。详细架构见 `README_zh.md` 与 `figures/`。

## 3. 构建与验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

### 3.1 构建

```bash
# 构建 hidumper 全部目标（CLI + 服务 + 库 + 配置）
./build.sh --product-name rk3568 --build-target //base/hiviewdfx/hidumper:bin
./build.sh --product-name rk3568 --build-target //base/hiviewdfx/hidumper:service

# 构建 inner_kits 库
./build.sh --product-name rk3568 --build-target //base/hiviewdfx/hidumper/interfaces/innerkits:lib_dump_usage

# 单独构建某个 GN 目标（示例：客户端库）
./build.sh --product-name rk3568 --build-target //base/hiviewdfx/hidumper/frameworks/native:hidumperclient
```

构建产物：CLI 安装到 `/system/bin/hidumper`；服务 `.so` 为 `libhidumperservice.z.so`（SA）、`libhidumper_client.z.so`（客户端）、`lib_dump_usage.z.so`、`libhidumper_plugin.z.so`。

### 3.2 单元测试 / Fuzz 构建

```bash
# 全部单元测试
./build.sh --product-name rk3568 --build-target //base/hiviewdfx/hidumper/test:unittest

# 单个测试二进制（示例：服务测试）
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 HidumperServiceTest

# 全部 Fuzz 测试
./build.sh --product-name rk3568 --build-target //base/hiviewdfx/hidumper/test:fuzztest
```

### 3.3 运行测试

```bash
# 运行单元测试
./out/rk3568/tests/hidumper/hidumper/HidumperServiceTest

# 单个 gtest 用例
./out/rk3568/tests/hidumper/hidumper/HidumperServiceTest --gtest_filter=HidumperServiceTest.DumpRequest

# 内存相关测试
./out/rk3568/tests/hidumper/hidumper/HidumperMemoryTest
./out/rk3568/tests/hidumper/hidumper/MemoryDumperTest
```

> 测试 `module_output_path = "hidumper/hidumper"`，二进制位于 `out/<product>/tests/hidumper/hidumper/`。单元测试以 `-DDUMP_TEST_MODE` 和 `-Dprivate=public` 编译，以注入 `testMainFunc_` 并暴露私有成员。

### 3.4 完成标准

任务被认为完成，当且仅当：

1. **代码改动可编译** — 上述构建命令通过，无新增告警。
2. **相关测试通过** — 对应单元测试通过（必要时加 `--gtest_filter`）。
3. **板侧验证（如适用）** — 涉及内存/CPU/SA/IPC/真实设备节点的改动需提供板侧证据（`hidumper` 命令输出、hdc 日志）。
4. **文档更新（如适用）** — 公共 API 修改需更新 `README_zh.md` / `README.md` 与头文件注释。
5. **约束被遵守** — 第 7、8 节约束被尊重，尤其公共 API / IPC 协议 / 生成代码边界。

### 3.5 无法运行验证时

明确说明无法运行的原因，列出推荐验证步骤供人工执行，标记需要人工验证的部分。

### 3.6 完成报告格式

报告应包含：改动摘要（文件列表、改动点）、验证结果（构建/测试输出）、风险评估（API 兼容性、性能、权限）、未完成事项。

## 4. 核心工作原则（必须遵守）

### 4.1 只以真实代码为准

- 任何代码级结论，必须先用搜索 / 读取工具读真实代码后再下结论。
- 给出结论时引用文件路径与行号（如 `dump_implement.h:70`）。
- 若代码缺失，明确说明「此代码在 hidumper 中未找到」。
- 不得将臆测的实现当作事实写出。

### 4.2 臆测管理

- 任何未经验证的陈述必须标注「推测」。
- 能验证的先验证；仅在确无验证途径时保留推测。

### 4.3 代码优先验证

- 用户提供的说法可能错误；先以源码核实再采信。
- 出现分歧时，以实现证据为准。

## 5. 项目梳理

```
/base/hiviewdfx/hidumper
├── client/native/                 # CLI：main.cpp、dump_client_main.cpp
├── frameworks/native/             # 导出核心框架
│   ├── common.h                   # DumpStatus 枚举、LOG_ERR/LOG_DEBUG 宏
│   ├── include/
│   │   ├── common/                # DumpCfg、DumperOpts、DumperParameter、DumperConstant
│   │   ├── executor/              # HidumperExecutor 基类 + 各 *_dumper.h
│   │   │   └── memory/            # 内存导出器与 parse/* 解析器
│   │   ├── factory/               # ExecutorFactory 基类 + 各 *_factory.h
│   │   ├── manager/               # DumpImplement（编排单例）
│   │   └── util/                  # string_utils、config_utils、zip_utils 等
│   ├── src/                        # 与 include 镜像的源文件
│   ├── manager/                    # dump_manager、cmd_parse、dump_context
│   ├── dump_strategy/             # 按域分发策略
│   ├── task/                       # base/cpu/memory/storage/system_info/writer 子任务
│   ├── data_inventory/            # 数据盘点
│   └── utils/                     # writer_utils
├── interfaces/
│   ├── innerkits/                 # lib_dump_usage（dump_usage.h）
│   └── native/innerkits/include/  # DumpManagerClient、DumpBrokerProxy、IDumpBroker、IPC code 枚举
├── services/
│   ├── native/
│   │   ├── include/               # DumpManagerService / DumpManagerCpuService 头文件
│   │   ├── src/                    # 服务实现
│   │   └── etc/                   # hidumper_service.rc/.cfg、infos_config.json
│   ├── zidl/                      # 手写 IDumpBroker stub/proxy
│   ├── IHidumperCpuService.idl    # CPU 服务 IDL（生成 stub/proxy）
│   └── hidumper.map               # 符号导出版本脚本
├── plugins/                       # hidumper_plugin.so（dumper_plugin.h）
├── utils/native/                  # hilog_wrapper.h、dump_errors.h、permission
├── sa_profile/                   # 1212.json（SA → 进程 → .so 映射）
├── test/
│   ├── unittest/common/           # 18 个 ohos_unittest 目标
│   ├── fuzztest/                  # 12 个 ohos_fuzztest 目标
│   ├── innerkits_test/            # 独立可执行 demo（未接入 GN test 组）
│   └── scripts/                  # pytest 集成测试（未接入 GN 构建）
├── bundle.json                    # 组件元数据（@ohos/hidumper, subsystem=hiviewdfx）
├── hidumper.gni                    # 共享路径变量与 feature 开关
└── README.md / README_zh.md
```

### 5.1 经常改动的路径

- `frameworks/native/include/executor/` + `src/executor/` — 导出器行为变更（最高频）。
- `frameworks/native/include/manager/dump_implement.h` — 命令解析与执行编排。
- `services/native/src/dump_manager_service.cpp` — 主服务请求处理。
- `interfaces/native/innerkits/include/` — 客户端 IPC 代理。

### 5.2 任务 → 路径速查

| 任务 | 起点 |
|---|---|
| 新增一种系统信息导出 | `frameworks/native/include/executor/<name>_dumper.h` + `src/executor/` + 对应 `_factory`，并在 `dump_implement` 工厂表注册 |
| 修改 CLI 选项解析 | `frameworks/native/include/common/dumper_opts.h` + `manager/cmd_parse` |
| 修改内存导出（smaps/ashmem/dmabuf/gpumem） | `frameworks/native/include/executor/memory/` |
| 修改 SA dump（元能力）分发 | `frameworks/native/include/executor/sa_dumper.h` |
| 修改 CPU 使用导出 | `services/native/src/dump_manager_cpu_service.cpp` + `frameworks/.../cpu_dumper` |
| 修改 IPC 接口 | 见第 8 节协议边界，先改 IDL / 头文件 |
| 修改权限校验 | `utils/native/src/permission.cpp` |
| 平台 / 运行配置 | `services/native/etc/`、`sa_profile/` |

## 6. 知识索引

稳定背景知识见 `README_zh.md`（架构图、命令用法）与 `figures/`。本仓库无 `docs/kb/` 知识库，改动前请阅读：

- `README_zh.md` — 组件简介、目录、全部 `hidumper` 命令用法（`-lc/-c/-s/-p/--mem/--cpuusage/--zip` 等）。
- `bundle.json` — 组件依赖、inner_kits 头文件清单、构建目标。
- `hidumper.gni` — feature 开关列表（决定哪些能力按需编译）。

### 6.1 开始编辑前

按以下顺序确认：

1. 确认任务类别（见 2.1 表）。
2. 确认已阅读相关 README / 头文件（或明确声明「无相关文档」）。
3. 根据「项目约束」确认不违反任何约束。
4. 声明：「将修改 X，已阅读 Y，遵循 Z 约束」。

## 7. 编码约定

### 7.1 命名空间与单例

- 全部代码位于 `OHOS::HiviewDFX`。
- 客户端单例用 `DelayedRefSingleton<T>`（如 `DumpManagerClient`）；框架编排用 `Singleton<T>`（如 `DumpImplement`）；服务 SA 用 `DumpDelayedSpSingleton<T>`（见 `delayed_sp_singleton.h`）。

### 7.2 日志宏（必须复用）

- 定义于 `utils/native/include/hilog_wrapper.h`：`DUMPER_HILOGF/E/W/I/D(module, fmt, ...)`，域 `0xD002D20`，tag `DumperService`，自动带函数名前缀。
- 服务端 / zidl 一律使用 `DUMPER_HILOG*`，例如：
  ```cpp
  DUMPER_HILOGE(MODULE_SERVICE, "request error, ret: %{public}d.", ret);
  ```
- 仅在直接写 stdout 的客户端流程才用 `common.h` 的 `LOG_ERR`/`LOG_DEBUG`/`LOG_TIME`。
- 含敏感信息（内存数据、进程栈、SA 内部信息）不得打印到非安全日志；用 `%{public}s` 仅对非敏感字段。

### 7.3 返回值（DumpStatus）

- 定义于 `frameworks/native/common.h:21`：`DUMP_OK=0` 为成功边界，`<0` 为错误（`DUMP_FAIL/DUMP_NOPERMISSION/DUMP_INVALID_ARG/DUMP_TIMEOUT/DUMP_REQUEST_MAX`），`>0` 为成功附信息（`DUMP_HELP/DUMP_CHECK_OK/DUMP_MORE_DATA`）。
- 客户端约定：`ret < DUMP_OK` 视为失败；`DUMP_INVALID_ARG` 通常静默返回不打日志。
- IPC parcel 层错误用 `utils/native/include/dump_errors.h` 的 `DumperError` 枚举，带 `ErrCodeOffset`。

### 7.4 导出器 / 工厂模式

- 抽象基类 `HidumperExecutor`（`include/executor/hidumper_executor.h`），生命周期 `PreExecute → Execute → AfterExecute`，配合 `DoPreExecute/DoExecute/DoAfterExecute` 包装与 `SetDumpConfig/IsCanceled`。
- 每个导出器配一个 `<Name>Factory`（继承 `ExecutorFactory`），由 `DumpImplement::AddExecutorFactoryToMap()` 以 `DumperConstant` 枚举值为键注册。
- 新增导出器须同时补 `include/executor/` + `src/executor/` + `include/factory/` + `src/factory/`，并在工厂表中注册。
- `StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>` 为导出数据载体。

### 7.5 fd 使用

- 客户端通过 `outFd` 直写输出；服务端 `dprintf(outFd, ...)`。涉及 fd 的打开/关闭须配对，避免泄漏。

## 8. 项目约束

### 8.1 性能约束

- 内存 / CPU / 进程信息导出是潜在高频或大输出路径，不要在导出循环中做全量扫描、冗余字符串格式化或 INFO 级日志。
- `--zip` 与大内存导出注意压缩内存峰值，复用 `util/zip` 已有实现。

### 8.2 架构约束

- 导出器走 `Executor + Factory` 抽象，不要在 `DumpImplement` 编排层直接硬编码具体导出逻辑。
- 主服务（1212）与 CPU 服务（1215）职责分离：CPU 专用能力归 CPU 服务，不要把 CPU 逻辑塞进主服务。
- 客户端只持 `DumpManagerClient` 代理，不直接实现导出逻辑。

### 8.3 公共 API 约束

**禁止（未经确认不得）：**

- 修改已发布的 `interfaces/innerkits/`、`interfaces/native/innerkits/include/` 头文件的函数签名、参数类型、返回值类型。
- 修改已有 API 的错误码语义（`DumpStatus` 值不可重排/重赋值）。
- 删除或重命名已有公共 API（`dump_usage.h`、`dumper_plugin.h`、`idump_broker.h`、`dump_manager_client.h`、`dump_manager_cpu_service.h`）。
- 修改 IPC 接口码枚举（`hidumper_service_ipc_interface_code.h`、`hidumper_cpu_service_ipc_interface_code.h`）已有项的数值。

**修改前必须确认：**

- 新增公共 API：确认是否需要权限校验、DFX 日志、hisysevent。
- 新增 IPC 接口码：追加在枚举末尾，不插队。
- 修改返回值语义：评估对应用层与 inner_kits 使用方的影响。

### 8.4 安全与权限边界

**禁止：**

- 绕过 `utils/native/src/permission.cpp` 中的 `AccessTokenID` 校验逻辑。
- 在未校验的情况下直接使用跨进程传递的文件描述符。
- 将敏感系统信息（进程栈、内存布局、SA 内部数据）写入非安全日志或明文导出到无权限调用方。
- 修改 SA 配置（uid 1212、caps `CAP_DAC_READ_SEARCH`/`CAP_SYS_PTRACE`）除非经安全评审。

**修改前必须确认：**

- 涉及 `access_token`、`memmgr`、`drivers_interface_memorytracker` 的改动。
- 新增需要更高权限的导出能力（读 `/proc/<pid>/smaps`、ptrace 抓栈等）。

### 8.5 协议与数据格式兼容性

**禁止：**

- 修改 `services/IHidumperCpuService.idl` 已有接口签名或 `DumpCpuData`（`dump_cpu_data.h`）的 Parcel 序列化字段顺序。
- 修改 `IDumpBroker`（`idump_broker.h`，descriptor `ohos.HiviewDFX.HiDumper.IDumpBroker`）已有方法。
- 修改跨进程传递数据结构的字段顺序。

**修改前必须确认：**

- 新增 IDL 方法：是否需要跨版本兼容。
- 修改 `dump_cpu_data.h` sequenceable 布局：影响 proxy/stub 双端。

### 8.6 生成代码边界

**禁止：**

- 直接手改 IDL 编译器生成的 `*_proxy.cpp` / `*_stub.cpp`（CPU 服务 IPC）。

**正确做法：**

- 修改 `services/IHidumperCpuService.idl`，重新由 `idl_gen_interface` 生成。
- 手写 IPC（`IDumpBroker`）的 stub/proxy 可直接编辑 `services/zidl/`。

### 8.7 设备 / 文件操作约束

- 不执行可能影响设备正常运行的破坏性操作。
- 读 `/proc`、`/sys`、`/dev` 节点的导出器须处理打开失败与权限不足，复用现有 `dump_utils`。
- 需板侧验证的改动必须提供证据（命令输出、hdc 日志）。

## 9. 常见陷阱

- **新增导出器只写了 executor 忘记注册 factory** — `DumpImplement` 工厂表无该项，命令路由不到。
- **混用日志体系** — 服务端用了 `LOG_ERR`（写 stdout）而非 `DUMPER_HILOGE`，日志丢失或落到错误通道。
- **`DumpStatus` 判错用 `== DUMP_FAIL`** — 应统一用 `ret < DUMP_OK`，否则漏掉 `DUMP_NOPERMISSION`/`DUMP_TIMEOUT` 等。
- **改 IPC 接口码插队** — 破坏与已发布客户端的兼容性，新码只能追加在枚举末尾。
- **手改 IDL 生成代码** — 下次重新生成被覆盖；应改 IDL 源文件。
- **`DUMP_INVALID_ARG` 当错误打日志** — 这是用户用法不对的预期分支，应静默返回，避免正常探询产生噪声。
- **在导出循环里打 INFO 日志 / 格式化大字符串** — 大输出场景产生性能与日志风暴。
- **新增 inner_kits 头文件未登记** — `bundle.json` 的 `inner_kits` 未同步，外部无法引用。
- **改 `dump_cpu_data.h` 字段顺序** — sequenceable Parcel 顺序错位，proxy/stub 数据损坏。
- **客户端进程直接实现导出** — 应经 `DumpManagerClient::Request` 走 IPC 到服务端，保持单一通道。
