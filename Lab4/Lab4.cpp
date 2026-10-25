#include "framework.h"
#include "Lab4.h"
#include "lab4_app.h"

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPWSTR,
    int nCmdShow
)
{
    INITCOMMONCONTROLSEX controls = {};

    controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    controls.dwICC = ICC_BAR_CLASSES;

    if (!InitCommonControlsEx(&controls)) {
        return 0;
    }

    Lab4App app(hInstance);

    return app.Run(nCmdShow);
}