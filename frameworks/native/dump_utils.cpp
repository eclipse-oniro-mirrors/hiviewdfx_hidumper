/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 */
#include "dump_utils.h"

#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <set>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"
#include "datetime_ex.h"
#include "parameters.h"
#include "securec.h"
#include "string_ex.h"

#include "system_ability_definition.h"
#include "hilog_wrapper.h"

namespace OHOS {
inline const std::map<int, std::string> saNameMap_ = {
    { 0, "SystemAbilityManager" },
    { 200, "AccountMgr" },
    { 301, "AIEngine" },
    { ABILITY_MGR_SERVICE_ID, "AbilityManagerService" },
    { DATAOBS_MGR_SERVICE_SA_ID, "DataObserverMgr" },
    { APP_MGR_SERVICE_ID, "ApplicationManagerService" },
    { URI_PERMISSION_MGR_SERVICE_ID, "UriPermissionMgr" },
    { QUICK_FIX_MGR_SERVICE_ID, "QuickFixMgrService"},
    { BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, "BundleMgr" },
    { SERVICE_ROUTER_MGR_SERVICE_ID, "ServiceRouterMgr" },
    { INSTALLD_SERVICE_ID, "Installd" },
    { FORM_MGR_SERVICE_ID, "FormMgr" },
    { WIFI_DEVICE_SYS_ABILITY_ID, "WifiDevice" },
    { WIFI_HOTSPOT_SYS_ABILITY_ID, "WifiHotspot" },
    { WIFI_ENHANCER_SYS_ABILITY_ID, "WifiEnhancer" },
    { WIFI_P2P_SYS_ABILITY_ID, "WifiP2p" },
    { WIFI_SCAN_SYS_ABILITY_ID, "WifiScan" },
    { BLUETOOTH_HOST_SYS_ABILITY_ID, "BluetoothHost" },
    { NFC_MANAGER_SYS_ABILITY_ID, "NfcManager" },
    { SE_MANAGER_SYS_ABILITY_ID, "SeManager" },
    { NET_MANAGER_SYS_ABILITY_ID, "NetManager" },
    { DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "DistributedKvData" },
    { DISTRIBUTED_SCHED_SA_ID, "DistributedSched" },
    { DISTRIBUTED_SCHED_ADAPTER_SA_ID, "DistributedSchedAdapter" },
    { DISTRIBUTED_SCENARIO_MGR_SA_ID, "DistributedScenarioMgr" },
    { CONTINUATION_MANAGER_SA_ID, "ContinuationManagerService" },
    { RES_SCHED_SYS_ABILITY_ID, "ResourceSched" },
    { RESSCHEDD_SA_ID, "ResourceSchedDamon" },
    { BACKGROUND_TASK_MANAGER_SERVICE_ID, "BackgroundTaskManager" },
    { WORK_SCHEDULE_SERVICE_ID, "WorkSchedule" },
    { CONCURRENT_TASK_SERVICE_ID, "ConcurrentTaskService" },
    { RESOURCE_QUOTA_CONTROL_SYSTEM_ABILITY_ID, "ResourceQuotaControl"},
    { DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID, "DeviceStandbyService"},
    { TASK_HEARTBEAT_MGR_SYSTEM_ABILITY_ID, "TaskHeartbeatMgrService"},
    { COMPONENT_SCHEDULE_SERVICE_ID, "ComponentSchedServer" },
    { SOC_PERF_SERVICE_SA_ID, "SocPerfService" },
    { SUSPEND_MANAGER_SYSTEM_ABILITY_ID, "SuspendManager" },
    { APP_NAP_SYSTEM_ABILITY_ID, "AppNap" },
    { ABNORMAL_EFFICIENCY_MGR_SYSTEM_ABILITY_ID, "AbnormalEfficiencyManager" },
    { LOCATION_GEO_CONVERT_SA_ID, "LocationGeoConvert" },
    { LOCATION_LOCATOR_SA_ID, "LocationLocator" },
    { LOCATION_GNSS_SA_ID, "LocationGnss" },
    { LOCATION_NETWORK_LOCATING_SA_ID, "LocationNetworkLocating" },
    { LOCATION_NOPOWER_LOCATING_SA_ID, "LocationNopowerLocating" },
    { AUDIO_DISTRIBUTED_SERVICE_ID, "AudioDistributed" },
    { COMMON_EVENT_SERVICE_ABILITY_ID, "CommonEventService" },
    { ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID, "DistributedNotificationService" },
    { COMMON_EVENT_SERVICE_ID, "CommonEventService" },
    { POWER_MANAGER_SERVICE_ID, "PowerManagerService" },
    { POWER_MANAGER_BATT_SERVICE_ID, "BatteryService" },
    { POWER_MANAGER_BATT_STATS_SERVICE_ID, "BatteryStatisticsService" },
    { POWER_MANAGER_THERMAL_SERVICE_ID, "ThermalService" },
    { DISPLAY_MANAGER_SERVICE_ID, "DisplayPowerManagerService" },
    { 3502, "DpmsService" },
    { 3510, "KeystoreService" },
    { DEVICE_SECURITY_LEVEL_MANAGER_SA_ID, "DslmService" },
    { CERT_MANAGER_SERVICE_SA_ID, "CertManagerService"},
    { DEVICE_THREAT_DETECTION_SERVICE_SA_ID, "DeviceThreatDetectionService" },
    { SENSOR_SERVICE_ABILITY_ID, "SensorService" },
    { MISCDEVICE_SERVICE_ABILITY_ID, "MiscDeviceService" },
    { PASTEBOARD_SERVICE_ID, "PasteboardService" },
    { SCREENLOCK_SERVICE_ID, "ScreenlockService" },
    { WALLPAPER_MANAGER_SERVICE_ID, "WallpaperManagerService" },
    { PRINT_SERVICE_ID, "PrintFramework" },
    { SCAN_SERVICE_ID, "ScanFramework" },
    { TELEPHONY_SYS_ABILITY_ID, "Telephony" },
    { DCALL_SYS_ABILITY_ID, "DistributedCallMgr" },
    { TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID, "TelephonyCallManager" },
    { TELEPHONY_CELLULAR_CALL_SYS_ABILITY_ID, "TelephonyCellularCall" },
    { TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID, "TelephonyCellularData" },
    { TELEPHONY_SMS_MMS_SYS_ABILITY_ID, "TelephonySmsMms" },
    { TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID, "TelephonyStateRegistry" },
    { TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID, "TelephonyCoreService" },
    { TELEPHONY_DATA_STORAGE_SYS_ABILITY_ID, "TelephonyDataStorage" },
    { TELEPHONY_IMS_SYS_ABILITY_ID, "TelephonyIms" },
    { DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "DeviceManagerService" },
    { DISTRIBUTED_HARDWARE_SA_ID, "DistributedHardwareFramework" },
    { DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, "DistributedCameraSourceService" },
    { DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, "DistributedCameraSinkService" },
    { DISTRIBUTED_HARDWARE_AUDIO_SOURCE_SA_ID, "DistributedAudioSourceService" },
    { DISTRIBUTED_HARDWARE_AUDIO_SINK_SA_ID, "DistributedAudioSinkService" },
    { DISTRIBUTED_HARDWARE_SCREEN_SOURCE_SA_ID, "DistributedScreenSourceService" },
    { DISTRIBUTED_HARDWARE_SCREEN_SINK_SA_ID, "DistributedScreenSinkService" },
    { DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, "DistributedInputSourceService" },
    { DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, "DistributedInputSinkService" },
    { INPUT_METHOD_SYSTEM_ABILITY_ID, "InputMethodService" },
    { MEDICAL_SENSOR_SERVICE_ABILITY_ID, "MedicalSensorService" },
    { USB_SYSTEM_ABILITY_ID, "UsbService" },
    { STORAGE_MANAGER_DAEMON_ID, "StorageDaemon" },
    { STORAGE_MANAGER_MANAGER_ID, "StorageManager"},
    { FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, "DistributedFileDaemon"},
    { FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, "CloudFileSyncService"},
    { FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, "CloudFileDaemonService"},
    { DEVICE_SERVICE_MANAGER_SA_ID, "HdfDeviceServiceManager" },
    { HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, "HdfExternalDeviceManager" },
    { DISTRIBUTED_DEVICE_PROFILE_SA_ID, "DistributedDeviceProfile" },
    { WINDOW_MANAGER_SERVICE_ID, "WindowManagerService" },
    { DISPLAY_MANAGER_SERVICE_SA_ID, "DisplayManagerService" },
    { SOFTBUS_SERVER_SA_ID, "DSoftbus" },
    { DEVICE_AUTH_SERVICE_ID, "DeviceAuthService" },
    { WINDOW_MANAGER_ID, "WindowManager" },
    { VSYNC_MANAGER_ID, "VsyncManager" },
    { VSYNC_MANAGER_TEST_ID, "VsyncManagerTest" },
    { GRAPHIC_DUMPER_SERVICE_SA_ID, "GraphicDumperService" },
    { GRAPHIC_DUMPER_COMMAND_SA_ID, "GraphicDumperCommand" },
    { ANIMATION_SERVER_SA_ID, "AnimationServer" },
    { CA_DAEMON_ID, "CaDaemon" },
    { ACCESSIBILITY_MANAGER_SERVICE_ID, "AccessibilityManagerService" },
    { DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID, "DeviceUsageStatistics" },
    { MEMORY_MANAGER_SA_ID, "MemoryManagerService" },
    { AVSESSION_SERVICE_ID, "AVSessionService" },
    { SYS_INSTALLER_DISTRIBUTED_SERVICE_ID, "SysInstallerService" },
    { MODULE_UPDATE_SERVICE_ID, "ModuleUpdateService" },
    { SUBSYS_ACE_SYS_ABILITY_ID_BEGIN, "UiService" },
    { ARKUI_UI_APPEARANCE_SERVICE_ID, "UiAppearanceService" },
    { SUBSYS_USERIAM_SYS_ABILITY_USERIDM, "UserIdmService" },
    { SUBSYS_USERIAM_SYS_ABILITY_AUTHEXECUTORMGR, "AuthExecutorMgrService" },
    { RENDER_SERVICE, "RenderService" },
    { ACCESS_TOKEN_MANAGER_SERVICE_ID, "AccessTokenManagerService" },
    { TOKEN_SYNC_MANAGER_SERVICE_ID, "TokenSyncManagerService" },
    { SANDBOX_MANAGER_SERVICE_ID, "SandboxManagerService" },
    { DLP_PERMISSION_SERVICE_ID, "DlpPermissionService" },
    { RISK_ANALYSIS_MANAGER_SA_ID, "RiskAnalysisManagerService" },
    { DATA_COLLECT_MANAGER_SA_ID, "DataCollectManagerService" },
    { MSDP_DEVICESTATUS_SERVICE_ID, "DeviceStatusService" },
    { MULTIMODAL_INPUT_SERVICE_ID, "MultimodalInput" },
    { DFX_SYS_HIVIEW_ABILITY_ID, "HiviewService" },
    { PRIVACY_MANAGER_SERVICE_ID, "PrivacyManagerService"},
    { DFX_FAULT_LOGGER_ABILITY_ID, "HiviewFaultLogger" },
    { DFX_SYS_EVENT_SERVICE_ABILITY_ID, "HiviewSysEventService" },
    { DFX_SYS_NATIVE_MEMORY_PROFILER_SERVICE_ABILITY_ID, "HiviewSysNativeMemoryProfilerService"},
    { XPERF_SYS_TRACE_SERVICE_ABILITY_ID, "XperfTraceService" },
    { XPERF_SYS_IO_SERVICE_ABILITY_ID, "XperfIoService" },
    { XPERF_BIGDATA_MANAGER_SERVICE_ABILITY_ID, "XperfBigDataManagerService" },
    { DFX_HI_DUMPER_SERVICE_ABILITY_ID, "HiDumperService" },
    { XPOWER_MANAGER_SYSTEM_ABILITY_ID, "XpowerManager"},
    { DFX_HI_PERF_SERVICE_ABILITY_ID, "HiPerfService"},
    { DFX_HI_DUMPER_CPU_SERVICE_ABILITY_ID, "HiDumperCpuService" },
    { SUBSYS_USERIAM_SYS_ABILITY_USERAUTH, "UserAuthService" },
    { SUBSYS_USERIAM_SYS_ABILITY_PINAUTH, "PinAuthService" },
    { SUBSYS_USERIAM_SYS_ABILITY_FACEAUTH, "FaceAuthService" },
    { SUBSYS_USERIAM_SYS_ABILITY_FINGERPRINTAUTH, "FingerprintAuthService" },
    { SUBSYS_USERIAM_SYS_ABILITY_FINGERPRINTAUTH_EX, "FingerprintAuthServiceEx" },
    { NET_MANAGER_SYS_ABILITY_ID, "NetManager" },
    { COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, "NetConnManager" },
    { COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, "NetPolicyManager" },
    { COMM_NET_STATS_MANAGER_SYS_ABILITY_ID, "NetStatsManager" },
    { COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, "NetTetheringManager" },
    { COMM_VPN_MANAGER_SYS_ABILITY_ID, "VPNManager" },
    { COMM_DNS_MANAGER_SYS_ABILITY_ID, "DNSManager"},
    { COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, "EthernetManager" },
    { COMM_NETSYS_NATIVE_SYS_ABILITY_ID, "NetsysNative" },
    { COMM_MDNS_MANAGER_SYS_ABILITY_ID, "MDNSManager"},
    { COMM_NETSYS_EXT_SYS_ABILITY_ID, "NetsysExtService"},
    { COMM_DISTRIBUTED_NET_ABILITY_ID, "DistributedNet"},
    { ENTERPRISE_DEVICE_MANAGER_SA_ID, "EnterpriseDeviceManagerService" },
    { I18N_SA_ID, "I18nService" },
    { PARAM_WATCHER_DISTRIBUTED_SERVICE_ID, "ParamWatcher" },
    { SYSPARAM_DEVICE_SERVICE_ID, "SysParamDevice" },
    { TIME_SERVICE_ID, "TimeService" },
    { PLAYER_DISTRIBUTED_SERVICE_ID, "PlayerDistributedService"},
    { AV_CODEC_SERVICE_ID, "AVCodecService"},
    { CAMERA_SERVICE_ID, "CameraService"},
    { MEDIA_KEY_SYSTEM_SERVICE_ID, "MediaKeySystemService"},
    { AUDIO_POLICY_SERVICE_ID, "AudioPolicyService"},
    { DLP_CREDENTIAL_SERVICE_ID, "DlpCreService"},
    { QUICKFIX_ENGINE_SERVICE_ID, "QuickfixEngineService"},
    { DEVICE_ATTEST_PROFILE_SA_ID, "devattest_service" },
    { AIDISPATCHER_ENGINE_SERVICE, "AIDispatcherEngineService" },
    { SECURITY_COMPONENT_SERVICE_ID, "SecurityComponentService" },
    { ADVERTISING_SA_ID, "Advertising" },
    { LOCAL_CODE_SIGN_SERVICE_ID, "LocalCodeSignService" },
    { INTELL_VOICE_SERVICE_ID, "IntellVoiceService" },
    { FILE_ACCESS_SERVICE_ID, "FileAccessService" },
    { MSDP_USER_STATUS_SERVICE_ID, "UserStatusAwarenessService" },
    { NEARLINK_HOST_SYS_ABILITY_ID, "NearLinkHost" },
    { ECOLOGICAL_RULE_MANAGER_SA_ID, "EcologicalRuleManager" },
    { ASSET_SERVICE_ID, "AssetService" },
    { TEST_SERVER_SA_ID, "TestServer" },
    { APP_DOMAIN_VERIFY_MANAGER_SA_ID, "AppDomainVerifyManager" },
    { APP_DOMAIN_VERIFY_AGENT_SA_ID, "AppDomainVerifyAgent" },
    { AOT_COMPILER_SERVICE_ID, "AotCompilerService" },
    { EL5_FILEKEY_MANAGER_SERVICE_ID, "El5FilekeyManager"},
    { COMM_FIREWALL_MANAGER_SYS_ABILITY_ID, "NetFirewallManager" },
    { SUBSYS_WEBVIEW_SYS_UPDATE_SERVICE_ID, "AppFwkUpdateService"},
};

namespace HiviewDFX {
void DumpUtils::IgnorePipeQuit()
{
    signal(SIGPIPE, SIG_IGN); // protect from pipe quit
}

void DumpUtils::IgnoreStdoutCache()
{
    setvbuf(stdout, nullptr, _IONBF, 0); // never cache stdout
}

void DumpUtils::IgnoreOom()
{
    setpriority(PRIO_PROCESS, 0, TOP_PRIORITY); // protect from OOM
}

void DumpUtils::SetAdj(int adj)
{
    int fd = FdToWrite(FILE_CUR_OOM_ADJ);
    if (fd < 0) {
        return;
    }
    dprintf(fd, "%d", adj);
    close(fd);
}

void DumpUtils::BoostPriority()
{
    IgnoreOom();
    IgnorePipeQuit();
    IgnoreStdoutCache();
    SetAdj(TOP_OOM_ADJ);
}

std::string DumpUtils::ErrnoToMsg(const int &error)
{
    const int bufSize = 128;
    char buf[bufSize] = {0};
    strerror_r(error, buf, bufSize);
    return buf;
}

int DumpUtils::FdToRead(const std::string &file)
{
    char path[PATH_MAX] = {0};
    if (realpath(file.c_str(), path) == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "realpath, no such file. path=[%{public}s], file=[%{public}s]",
            path, file.c_str());
        return -1;
    }

    if (file != std::string(path)) {
        DUMPER_HILOGI(MODULE_COMMON, "fail to check consistency. path=[%{public}s], file=[%{public}s]",
            path, file.c_str());
    }

    int fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK));
    if (fd == -1) {
        DUMPER_HILOGE(MODULE_COMMON, "open [%{public}s] %{public}s", path, ErrnoToMsg(errno).c_str());
    }
    return fd;
}

int DumpUtils::FdToWrite(const std::string &file)
{
    std::string split = "/";
    auto pos = file.find_last_of(split);
    if (pos == std::string::npos) {
        DUMPER_HILOGE(MODULE_COMMON, "file path:[%{public}s] error", file.c_str());
        return -1;
    }
    std::string tempPath = file.substr(0, pos + 1);
    std::string name = file.substr(pos + 1);

    char path[PATH_MAX] = {0};
    if (realpath(tempPath.c_str(), path) != nullptr) {
        std::string fileName = path + split + name;
        int fd = -1;
        if (access(fileName.c_str(), F_OK) == 0) {
            fd = open(fileName.c_str(), O_WRONLY);
            if (lseek(fd, 0, SEEK_END) == -1) {
                DUMPER_HILOGE(MODULE_COMMON, "lseek fail fd:%{public}d, errno:%{public}d", fd, errno);
            }
        } else {
            fd = TEMP_FAILURE_RETRY(open(fileName.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC | O_NOFOLLOW,
                                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
        }
        if (fd == -1) {
            DUMPER_HILOGE(MODULE_COMMON, "open [%{public}s] %{public}s",
                fileName.c_str(), ErrnoToMsg(errno).c_str());
        }
        return fd;
    }
    DUMPER_HILOGD(MODULE_COMMON, "realpath, no such file. path=[%{public}s], tempPath=[%{public}s]",
        path, tempPath.c_str());
    return -1;
}

bool DumpUtils::FileWriteable(const std::string &file)
{
    int fd = FdToWrite(file);
    if (fd >= 0) {
        close(fd);
        return true;
    }
    return false;
}

bool DumpUtils::CheckProcessAlive(uint32_t pid)
{
    char path[PATH_MAX] = {0};
    char pathPid[PATH_MAX] = {0};
    int ret = sprintf_s(pathPid, PATH_MAX, "/proc/%u", pid);
    if (ret < 0) {
        return false;
    }
    if (realpath(pathPid, path) != nullptr) { // path exist
        if (access(path, F_OK) == 0) {        // can be read
            return true;
        }
    }
    return false;
}

void DumpUtils::RemoveDuplicateString(std::vector<std::string> &strList)
{
    std::vector<std::string> tmpVtr;
    std::set<std::string> tmpSet;
    for (auto &it : strList) {
        auto ret = tmpSet.insert(it);
        if (ret.second) {
            tmpVtr.push_back(it);
        }
    }
    strList = tmpVtr;
}

int DumpUtils::StrToId(const std::string &name)
{
    int id = -1;
    auto iter = std::find_if(saNameMap_.begin(), saNameMap_.end(), [&](const std::pair<int, std::string> &item) {
        return name.compare(item.second) == 0;
    });
    if (iter == saNameMap_.end()) {
        if (!StrToInt(name, id)) { // Decimal string
            return -1; // invalid ability ID
        }
    } else {
        id = iter->first;
    }
    return id;
}

std::string DumpUtils::ConvertSaIdToSaName(const std::string &saIdStr)
{
    int saId = StrToId(saIdStr);
    auto iter = saNameMap_.find(saId);
    if (iter == saNameMap_.end()) {
        return saIdStr;
    }
    return iter->second;
}

bool DumpUtils::DirectoryExists(const std::string &path)
{
    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) == 0) {
        return S_ISDIR(fileInfo.st_mode);
    }
    return false;
}

bool DumpUtils::PathIsValid(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

bool DumpUtils::CopyFile(const std::string &src, const std::string &des)
{
    std::ifstream fin(src);
    std::ofstream fout(des);
    if ((!fin.is_open()) || (!fout.is_open())) {
        return false;
    }
    fout << fin.rdbuf();
    if (fout.fail()) {
        fout.clear();
    }
    fout.flush();
    return true;
}

bool DumpUtils::IsCommercialVersion()
{
    bool isCommercialVersion = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "commercial";
    return isCommercialVersion;
}
} // namespace HiviewDFX
} // namespace OHOS
