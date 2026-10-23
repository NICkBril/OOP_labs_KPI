#include "framework.h"
#include "Lab3.h"
#include "lab3_app.h"

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

    Lab3App app(hInstance);

    return app.Run(nCmdShow);
}
