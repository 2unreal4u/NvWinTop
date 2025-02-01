#include "gpu_monitor.hpp"
#include <windows.h>
#include <psapi.h>
#include <algorithm>

GpuMonitor::GpuMonitor() : m_initialized(false) {}

GpuMonitor::~GpuMonitor() {
    if (m_initialized) {
        nvmlShutdown();
    }
}

bool GpuMonitor::initialize() {
    if (m_initialized) return true;

    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) return false;

    unsigned int deviceCount = 0;
    result = nvmlDeviceGetCount(&deviceCount);
    if (result != NVML_SUCCESS) return false;

    m_metricsHistory.resize(deviceCount);
    m_initialized = true;
    return true;
}

void GpuMonitor::update() {
    if (!m_initialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    unsigned int deviceCount = 0;
    nvmlDeviceGetCount(&deviceCount);

    m_currentMetrics.clear();
    m_processInfo.clear();

    for (unsigned int i = 0; i < deviceCount; ++i) {
        nvmlDevice_t device;
        if (nvmlDeviceGetHandleByIndex(i, &device) != NVML_SUCCESS) continue;

        GpuMetrics metrics = {};
        metrics.index = i;

        // Get device name
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        if (nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE) == NVML_SUCCESS) {
            metrics.name = std::wstring(name, name + strlen(name));
        }

        // Get utilization
        nvmlUtilization_t utilization;
        if (nvmlDeviceGetUtilizationRates(device, &utilization) == NVML_SUCCESS) {
            metrics.gpuUtil = utilization.gpu;
            metrics.memUtil = utilization.memory;
        }

        // Get temperature
        unsigned int temp;
        if (nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS) {
            metrics.temperature = temp;
        }

        // Get fan speed
        unsigned int fanSpeed;
        if (nvmlDeviceGetFanSpeed(device, &fanSpeed) == NVML_SUCCESS) {
            metrics.fanSpeed = fanSpeed;
        }

        // Get power usage
        unsigned int power;
        if (nvmlDeviceGetPowerUsage(device, &power) == NVML_SUCCESS) {
            metrics.powerUsage = power / 1000.0; // Convert from milliwatts to watts
        }

        // Get clock speeds
        unsigned int clock;
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clock) == NVML_SUCCESS) {
            metrics.coreClock = clock;
        }
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_MEM, &clock) == NVML_SUCCESS) {
            metrics.memClock = clock;
        }

        // Get memory info
        nvmlMemory_t memInfo;
        if (nvmlDeviceGetMemoryInfo(device, &memInfo) == NVML_SUCCESS) {
            metrics.totalMemory = memInfo.total;
            metrics.usedMemory = memInfo.used;
        }

        m_currentMetrics.push_back(metrics);

        // Update history
        m_metricsHistory[i].push_back(metrics);
        if (m_metricsHistory[i].size() > HISTORY_SIZE) {
            m_metricsHistory[i].pop_front();
        }

        // Get process information
        unsigned int processCount = 0;
        nvmlProcessInfo_t processes[32];
        if (nvmlDeviceGetComputeRunningProcesses(device, &processCount, processes) == NVML_SUCCESS) {
            for (unsigned int p = 0; p < processCount; ++p) {
                ProcessInfo procInfo;
                procInfo.gpuIndex = i;
                procInfo.pid = processes[p].pid;
                procInfo.memoryUsed = processes[p].usedGpuMemory;

                // Get process name
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[p].pid);
                if (hProcess) {
                    wchar_t processName[MAX_PATH];
                    if (GetModuleBaseNameW(hProcess, NULL, processName, MAX_PATH)) {
                        procInfo.name = processName;
                    }
                    CloseHandle(hProcess);
                }

                m_processInfo.push_back(procInfo);
            }
        }
    }
}
