#include "graph_renderer.hpp"
#include <algorithm>
#include <cmath>

using std::max;
using std::min;
using std::ceil;

GraphRenderer::GraphRenderer()
    : m_hwnd(nullptr)
    , m_pD2DFactory(nullptr)
    , m_pRenderTarget(nullptr)
    , m_pBrushText(nullptr)
    , m_pBrushGraph(nullptr)
    , m_pBrushBackground(nullptr)
    , m_pDWriteFactory(nullptr)
    , m_pTextFormat(nullptr)
    , m_pTitleFormat(nullptr)
    , m_pBrushRed(nullptr)
    , m_pBrushYellow(nullptr)
    , m_pBrushSeparator(nullptr)
    , m_pBrushNvidiaGreen(nullptr)
{}

GraphRenderer::~GraphRenderer() {
    if (m_pBrushText) m_pBrushText->Release();
    if (m_pBrushGraph) m_pBrushGraph->Release();
    if (m_pBrushBackground) m_pBrushBackground->Release();
    if (m_pBrushRed) m_pBrushRed->Release();
    if (m_pBrushYellow) m_pBrushYellow->Release();
    if (m_pBrushSeparator) m_pBrushSeparator->Release();
    if (m_pBrushNvidiaGreen) m_pBrushNvidiaGreen->Release();
    if (m_pRenderTarget) m_pRenderTarget->Release();
    if (m_pD2DFactory) m_pD2DFactory->Release();
    if (m_pTextFormat) m_pTextFormat->Release();
    if (m_pTitleFormat) m_pTitleFormat->Release();
    if (m_pDWriteFactory) m_pDWriteFactory->Release();
}

bool GraphRenderer::initialize(HWND hwnd) {
    m_hwnd = hwnd;

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) return false;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );
    if (FAILED(hr)) return false;

    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        12.0f,
        L"en-us",
        &m_pTextFormat
    );
    if (FAILED(hr)) return false;

    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"en-us",
        &m_pTitleFormat
    );
    if (FAILED(hr)) return false;

    createDeviceResources();
    return true;
}

void GraphRenderer::createDeviceResources() {
    if (!m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
        props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);

        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(m_hwnd, size);

        HRESULT hr = m_pD2DFactory->CreateHwndRenderTarget(
            props,
            hwndProps,
            &m_pRenderTarget
        );

        if (SUCCEEDED(hr)) {
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.1f, 0.1f, 0.1f), &m_pBrushBackground);
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x76/255.0f, 0xb9/255.0f, 0x00/255.0f), &m_pBrushNvidiaGreen);
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.8f, 0.0f), &m_pBrushGraph);
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.2f, 0.2f), &m_pBrushRed);
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.2f), &m_pBrushYellow);
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.4f, 0.4f), &m_pBrushSeparator);
        }
    }
}

void GraphRenderer::resize() {
    if (m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        m_pRenderTarget->Resize(size);
    }
}

void GraphRenderer::drawGraph(const D2D1_RECT_F& rect, 
                            const std::deque<GpuMetrics>& history,
                            const wchar_t* title,
                            float currentValue,
                            const wchar_t* units) {
    // Draw background and border
    m_pRenderTarget->FillRectangle(rect, m_pBrushBackground);
    m_pRenderTarget->DrawRectangle(rect, m_pBrushSeparator, 1.0f);

    // Calculate max value for scaling
    float maxValue = 100.0f; // Default max for percentages
    if (wcscmp(title, L"Temperature") == 0) {
        maxValue = 100.0f; // Max temp in Celsius
    } else if (wcscmp(title, L"Power Usage") == 0) {
        // Find max power usage in history plus 20% headroom
        maxValue = 0.0f;
        for (const auto& metrics : history) {
            if (wcscmp(title, L"Power Usage") == 0) {
                maxValue = max(maxValue, static_cast<float>(metrics.powerUsage));
            }
        }
        maxValue = ceil(maxValue * 1.2f); // Add 20% headroom and round up
        if (maxValue < 50.0f) maxValue = 50.0f; // Minimum scale for power
    }

    const float graphHeight = rect.bottom - rect.top - 30;
    const float graphWidth = rect.right - rect.left - 45;

    // Draw grid lines
    float scaleStep = graphHeight / 4;
    for (int i = 1; i <= 4; i++) {
        float y = rect.bottom - 5 - (i * scaleStep);
        m_pRenderTarget->DrawLine(
            D2D1::Point2F(rect.left + 5, y),
            D2D1::Point2F(rect.right - 40, y),
            m_pBrushSeparator,
            0.5f
        );
    }

    // Draw scale on the right
    for (int i = 0; i <= 4; i++) {
        float y = rect.bottom - 5 - (i * scaleStep);
        float value = i * (maxValue / 4.0f);
        wchar_t scaleText[8];
        swprintf_s(scaleText, L"%.0f", value);
        
        // Choose text color based on value
        ID2D1SolidColorBrush* textBrush = m_pBrushNvidiaGreen;
        if (value > maxValue * 0.8f) textBrush = m_pBrushRed;
        else if (value > maxValue * 0.6f) textBrush = m_pBrushYellow;
        
        m_pRenderTarget->DrawText(
            scaleText,
            wcslen(scaleText),
            m_pTextFormat,
            D2D1::RectF(rect.right - 35, y - 10, rect.right - 5, y + 10),
            textBrush
        );
    }

    // Draw title and current value
    wchar_t valueText[64];
    if (wcscmp(title, L"Temperature") == 0) {
        swprintf_s(valueText, L"%.0f\u2103", currentValue);
    } else if (wcscmp(title, L"Power Usage") == 0) {
        swprintf_s(valueText, L"%.0fW", currentValue);
    } else {
        swprintf_s(valueText, L"%.1f%%", currentValue);
    }
    
    // Choose text color based on current value
    ID2D1SolidColorBrush* textBrush = m_pBrushNvidiaGreen;
    float normalizedValue = currentValue;
    if (wcscmp(title, L"Power Usage") == 0) {
        normalizedValue = (currentValue / maxValue) * 100.0f;
    }
    
    if (normalizedValue > 80.0f) textBrush = m_pBrushRed;
    else if (normalizedValue > 60.0f) textBrush = m_pBrushYellow;
    
    m_pRenderTarget->DrawText(
        title,
        wcslen(title),
        m_pTitleFormat,
        D2D1::RectF(rect.left + 5, rect.top + 5, rect.right - 70, rect.top + 25),
        textBrush
    );

    m_pRenderTarget->DrawText(
        valueText,
        wcslen(valueText),
        m_pTextFormat,
        D2D1::RectF(rect.right - 70, rect.top + 5, rect.right - 30, rect.top + 25),
        textBrush
    );

    // Draw graph
    if (history.size() < 2) return;

    const float xStep = graphWidth / (history.size() - 1);
    
    std::vector<D2D1_POINT_2F> points;
    points.reserve(history.size());

    float x = rect.left + 5;
    for (const auto& metrics : history) {
        float value = 0.0f;
        if (wcscmp(title, L"GPU Utilization") == 0) value = metrics.gpuUtil;
        else if (wcscmp(title, L"Memory Utilization") == 0) value = metrics.memUtil;
        else if (wcscmp(title, L"Temperature") == 0) value = metrics.temperature;
        else if (wcscmp(title, L"Power Usage") == 0) value = static_cast<float>(metrics.powerUsage);

        // Ensure value is within bounds
        value = max(0.0f, min(value, maxValue));
        
        // Calculate y position
        float y = rect.bottom - 5 - ((value / maxValue) * graphHeight);
        points.push_back(D2D1::Point2F(x, y));
        x += xStep;
    }

    // Draw the line graph with color based on value
    for (size_t i = 1; i < points.size(); ++i) {
        float value = (rect.bottom - points[i].y - 5) / graphHeight * maxValue;
        ID2D1SolidColorBrush* brush = m_pBrushGraph;
        
        float percentage = (value / maxValue) * 100.0f;
        if (percentage > 80.0f) brush = m_pBrushRed;
        else if (percentage > 60.0f) brush = m_pBrushYellow;
        else brush = m_pBrushNvidiaGreen;
        
        m_pRenderTarget->DrawLine(points[i-1], points[i], brush, 2.0f);
    }
}

void GraphRenderer::render(const std::vector<GpuMetrics>& currentMetrics,
                         const std::vector<std::deque<GpuMetrics>>& history) {
    createDeviceResources();

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(0.15f, 0.15f, 0.15f));

    if (currentMetrics.empty() || history.empty()) {
        m_pRenderTarget->EndDraw();
        return;
    }

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    const float width = static_cast<float>(rc.right - rc.left);
    const float height = static_cast<float>(rc.bottom - rc.top);
    
    const float graphWidth = (width - 40) / 2;
    const float graphHeight = (height - 20 - currentMetrics.size() * 40) / (2 * currentMetrics.size());
    
    for (size_t i = 0; i < currentMetrics.size(); ++i) {
        float baseY = 10 + i * (graphHeight * 2 + 40);

        // Draw GPU header with model name
        wchar_t gpuHeader[256];
        swprintf_s(gpuHeader, L"GPU %d: %s", i, currentMetrics[i].name.c_str());
        m_pRenderTarget->DrawText(
            gpuHeader,
            wcslen(gpuHeader),
            m_pTitleFormat,
            D2D1::RectF(10, baseY, width - 10, baseY + 30),
            m_pBrushNvidiaGreen
        );

        // Draw separator line
        m_pRenderTarget->DrawLine(
            D2D1::Point2F(10, baseY + 35),
            D2D1::Point2F(width - 10, baseY + 35),
            m_pBrushSeparator,
            1.0f
        );

        baseY += 40;
        
        // GPU Utilization
        drawGraph(
            D2D1::RectF(10, baseY, 10 + graphWidth, baseY + graphHeight),
            history[i],
            L"GPU Utilization",
            static_cast<float>(currentMetrics[i].gpuUtil),
            L"%"
        );

        // Memory Utilization
        drawGraph(
            D2D1::RectF(20 + graphWidth, baseY, width - 10, baseY + graphHeight),
            history[i],
            L"Memory Utilization",
            static_cast<float>(currentMetrics[i].memUtil),
            L"%"
        );

        // Temperature
        drawGraph(
            D2D1::RectF(10, baseY + graphHeight + 10, 10 + graphWidth, baseY + graphHeight * 2),
            history[i],
            L"Temperature",
            static_cast<float>(currentMetrics[i].temperature),
            L"\u2103"
        );

        // Power Usage
        drawGraph(
            D2D1::RectF(20 + graphWidth, baseY + graphHeight + 10, width - 10, baseY + graphHeight * 2),
            history[i],
            L"Power Usage",
            static_cast<float>(currentMetrics[i].powerUsage),
            L"W"
        );
    }

    m_pRenderTarget->EndDraw();
}
