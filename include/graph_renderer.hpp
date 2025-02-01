#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <memory>
#include <vector>
#include "gpu_monitor.hpp"

class GraphRenderer {
public:
    GraphRenderer();
    ~GraphRenderer();

    bool initialize(HWND hwnd);
    void render(const std::vector<GpuMetrics>& currentMetrics,
               const std::vector<std::deque<GpuMetrics>>& history);
    void resize();

private:
    void createDeviceResources();
    void drawGraph(const D2D1_RECT_F& rect, const std::deque<GpuMetrics>& history,
                  const wchar_t* title, float currentValue, const wchar_t* units);
    
    HWND m_hwnd;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pBrushText;
    ID2D1SolidColorBrush* m_pBrushGraph;
    ID2D1SolidColorBrush* m_pBrushBackground;
    ID2D1SolidColorBrush* m_pBrushRed;
    ID2D1SolidColorBrush* m_pBrushYellow;
    ID2D1SolidColorBrush* m_pBrushSeparator;
    ID2D1SolidColorBrush* m_pBrushNvidiaGreen;
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;
    IDWriteTextFormat* m_pTitleFormat;
};
