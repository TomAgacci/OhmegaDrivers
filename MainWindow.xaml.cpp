#include "MainWindow.xaml.h"
#include "Gamma.h"
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;

WORD gRamp[3][256];
float gGamma = 2.2f;

namespace GammaWinUI
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        BuildGammaRamp(gGamma, gRamp);
    }

    void MainWindow::GammaSlider_ValueChanged(IInspectable const&, RoutedEventArgs const&)
    {
        gGamma = (float)GammaSlider().Value();
        BuildGammaRamp(gGamma, gRamp);
        ApplyGammaRamp(gRamp);
        GammaCanvas().Invalidate();
    }

    void MainWindow::GammaCanvas_Draw(CanvasControl const& sender, CanvasDrawEventArgs const& args)
    {
        auto ds = args.DrawingSession();
        float width = sender.ActualWidth();
        float height = sender.ActualHeight();
        float barWidth = width / 256.0f;

        for (int i = 0; i < 256; ++i)
        {
            float R = gRamp[0][i] / 65535.0f;
            float G = gRamp[1][i] / 65535.0f;
            float B = gRamp[2][i] / 65535.0f;

            auto color = Windows::UI::Color{
                255,
                (uint8_t)(R * 255),
                (uint8_t)(G * 255),
                (uint8_t)(B * 255)
            };

            ds.FillRectangle(i * barWidth, 0, barWidth, height, color);
        }
    }
}
