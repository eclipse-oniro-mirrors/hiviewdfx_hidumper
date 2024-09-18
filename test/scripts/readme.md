# pytest命令行参数
-s: 显示输出调试信息，包括print打印的信息
-v: 显示更详细的信息
-n：支持多线程运行脚本（需要保持用例彼此独立）
--reruns NUM：失败用例重跑次数
-x：表示只要有一个用例报错，那么测试停止
-k：模糊匹配字符串进行用例跑测

# 目录结构
```
/hidumper/test
├─reports # 测试报告目录
├─testModule
|   ├─output # 命令行输出文件，例如:重定向文件，zip文件
|   ├─resource # 测试资源文件，存放测试过程中使用到的文件
|   └─tests # 测试用例目录
|       ├─test_case1.py # 测试套件1
|       ├─test_case2.py # 测试套件2
|       └─utils.py # 工具
├─main.py # 测试用例执行入口
├─pytest.ini # pytest配置文件
└─requirements.txt # 依赖文件
```

# 测试用例编写
## 简单校验
将命令和校验正则表达式写入csv文件
## 复杂校验
### 1. 新建测试文件
```
# test_base_command.py
class TestBaseCommand:
    def test_hidumper_c_all(self):
        # 校验命令行输出
        output = subprocess.check_output(f"hdc shell \"hidumper -c\"", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_hidumper_c_output)

        # 校验命令行重定向输出
        hidumper_c_file = f"{OUTPUT_PATH}/hidumper_c.txt"
        subprocess.check_output(f"hdc shell \"hidumper -c\" > {hidumper_c_file}", shell=True, text=True, encoding="utf-8")
        assert check_file(hidumper_c_file, check_function = check_hidumper_c_output)

        # 校验命令行输出到zip文件
        output = subprocess.check_output(f"hdc shell \"hidumper -c --zip\"", shell=True, text=True, encoding="utf-8")
        hidumper_c_zip_source_file = re.search("The result is:(.+)", output).group(1)
        hidumper_c_zip_target_file = f"{OUTPUT_PATH}/" + os.path.basename(hidumper_c_zip_source_file)
        subprocess.check_output(f"hdc file recv {hidumper_c_zip_source_file} {hidumper_c_zip_target_file}", shell=True, text=True, encoding="utf-8")
        assert check_zip_file(hidumper_c_zip_target_file, check_function = check_hidumper_c_output)
```
### 2. 编写校验函数
```
# test_base_command.py
# 自定义校验函数
@print_check_result
def CheckOsVersion(output) -> bool:
    # 校验OsVersion：5.0.0.1233
    ret = re.search("OsVersion: (.+?)[\d].[\d].[\d].[\d]", output)
    return ret is not None

@print_check_result
def CheckWakeUpSource(output) -> bool:
    # 校验/sys/kernel/debug/wakeup_sources至少输出四行
    ret = re.search("/sys/kernel/debug/wakeup_sources\n\n([^\n]+\n){4,}", output)
    return ret is not None
......

def check_hidumper_c_output(output):
def check_hidumper_c_output(output):
    results = [check(output) for check in [CheckBuildId, CheckOsVersion, CheckProcVersion, CheckCmdline, CheckWakeUpSource, 
                                      CheckUpTime, CheckPrintEnv, CheckLsmod, CheckSlabinfo, CheckZoneinfo, CheckVmstat, CheckVmallocinfo]]
    return all(results)
```

## 测试用例执行
windows环境下执行测试用例：
进入scripts目录
### 方式一：

    ```
    python main.py
    ```
执行参数在pytest.main中配置

### 方式二：
执行所有用例
    ```
    pytest ./
    ```
执行指定测试文件
    ```
    pytest ./testModule/test_base_command.py
    ```
执行指定测试用例
    ```
    pytest  -k "test_hidumper_c_all"
    ```
执行模糊匹配test_hidumper_c所有用例
    ```
    pytest  -k "test_hidumper_c"
    ```

## 测试报告
执行python main.py后，会在reports目录下生成测试报告
