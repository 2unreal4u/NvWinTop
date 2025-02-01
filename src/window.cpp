#include "window.hpp"
#include <windowsx.h>

MainWindow::MainWindow() : m_hwnd(nullptr), m_isActive(false) {
    m_gpuMonitor = std::make_unique<GpuMonitor>();
    m_renderer = std::make_unique<GraphRenderer>();
}

MainWindow::~MainWindow() {
    if (m_isActive) {
        KillTimer(m_hwnd, 1);
    }
}

bool MainWindow::create() {
    if (!m_gpuMonitor->initialize()) {
        MessageBoxW(nullptr, L"Failed to initialize GPU monitoring.\nMake sure you have NVIDIA drivers installed.", 
                   L"Error", MB_ICONERROR);
        return false;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"NvWinTopWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        0,
        L"NvWinTopWindow",
        L"NvWinTop - NVIDIA GPU Monitor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!m_hwnd) return false;

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (!m_renderer->initialize(m_hwnd)) {
        MessageBoxW(nullptr, L"Failed to initialize graphics renderer.", L"Error", MB_ICONERROR);
        return false;
    }

    // Start update timer (1 second interval)
    SetTimer(m_hwnd, 1, 1000, nullptr);
    m_isActive = true;

    return true;
}

void MainWindow::show(int nCmdShow) {
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window) {
        return window->handleMessage(uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            onPaint();
            return 0;

        case WM_SIZE:
            onResize();
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                onTimer();
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
}

void MainWindow::onPaint() {
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    
    const auto& currentMetrics = m_gpuMonitor->getCurrentMetrics();
    const auto& history = m_gpuMonitor->getMetricsHistory();
    m_renderer->render(currentMetrics, history);
    
    EndPaint(m_hwnd, &ps);
}

void MainWindow::onTimer() {
    m_gpuMonitor->update();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::onResize() {
    m_renderer->resize();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}
