#pragma once
#include <windows.h>
#include <memory>
#include "gpu_monitor.hpp"
#include "graph_renderer.hpp"

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool create();
    void show(int nCmdShow);
    HWND handle() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onPaint();
    void onTimer();
    void onResize();

    HWND m_hwnd;
    std::unique_ptr<GpuMonitor> m_gpuMonitor;
    std::unique_ptr<GraphRenderer> m_renderer;
    bool m_isActive;
};
