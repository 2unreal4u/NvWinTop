#pragma once
#include <nvml.h>
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <mutex>

struct GpuMetrics {
    unsigned int index;
    std::wstring name;
    unsigned int gpuUtil;
    unsigned int memUtil;
    unsigned int temperature;
    unsigned int fanSpeed;
    double powerUsage;
    unsigned int powerLimit;
    unsigned int coreClock;
    unsigned int memClock;
    unsigned long long totalMemory;
    unsigned long long usedMemory;
};

struct ProcessInfo {
    unsigned int gpuIndex;
    unsigned int pid;
    std::wstring name;
    unsigned long long memoryUsed;
    unsigned int gpuUtil;
};

class GpuMonitor {
public:
    static constexpr size_t HISTORY_SIZE = 120; // 2 minutes of history at 1s intervals

    GpuMonitor();
    ~GpuMonitor();

    bool initialize();
    void update();
    
    const std::vector<GpuMetrics>& getCurrentMetrics() const { return m_currentMetrics; }
    const std::vector<std::deque<GpuMetrics>>& getMetricsHistory() const { return m_metricsHistory; }
    const std::vector<ProcessInfo>& getProcessInfo() const { return m_processInfo; }

private:
    std::vector<GpuMetrics> m_currentMetrics;
    std::vector<std::deque<GpuMetrics>> m_metricsHistory;
    std::vector<ProcessInfo> m_processInfo;
    std::mutex m_mutex;
    bool m_initialized;
};
