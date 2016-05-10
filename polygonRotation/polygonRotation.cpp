// compile command
// cl.exe polygon_rotation.cpp /EHsc user32.lib kernel32.lib gdi32.lib opengl32.lib glu32.lib
//

#include<windows.h>
#include<gl/gl.h>
#include<gl/glu.h>

HGLRC hRenderingContext = NULL; // Permanent rendering context.
HDC hDeviceContext = NULL; // Private GDI device context.
HWND hWindow = NULL; // Window handle.
HINSTANCE hInstance = NULL; // Application instance.
TCHAR className[] = TEXT("OpenGlWindow");
TCHAR windowTitle[] = TEXT("Polygon Rotation");

bool keys[256]; // Used for keyboard routine.
bool active = TRUE; // Windows active flag. TRUE by default.
bool fullscreen = TRUE; // Full screen flag. TRUE by default.

GLfloat rtri; // Rotation angle for triangle.
GLfloat rquad; // Rotation angle for quad.

int windowWidth = 640;
int windowHeight = 480;
int windowWidthFullscreen = 1366;
int windowHeightFullscreen = 768;
int bitsPerColor = 32;

float rotationDirection = 1.0f; // Rotation direction. 1: Clockwise, -1: anticlockwise.

// Window event callback method.
LRESULT CALLBACK WndProc(HWND hWindow, UINT iMessage, WPARAM wParam, LPARAM lParam);

// Resize and initialize GL window.
GLvoid resizeGLScene(GLsizei width, GLsizei height)
{
    if(height == 0) // To prevent divide by 0.
    {
        height = 1;
    }

    glViewport(0, 0, width, height); // Reset current viewport.
    glMatrixMode(GL_PROJECTION); // Select the projection matrix.
    glLoadIdentity(); // Reset projection matrix.

    // Calculate aspect ratio of window
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW); // Select matrix mode as model-view.
    glLoadIdentity();
}

// Setup OpenGL
int initOpenGL(GLvoid)
{
    glShadeModel(GL_SMOOTH); // Enable smooth shading.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set background color.
    glClearDepth(1.0f); // Setup depth buffer.
    glEnable(GL_DEPTH_TEST); // Enable depth testing.
    glDepthFunc(GL_LEQUAL); // Type of depth test to do.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really nice perspective calculation.
    return TRUE; // Everything is ok.
}

GLvoid drawTriangle(GLvoid)
{
    glLoadIdentity(); // Reset current model-view matrix.
    glTranslatef(-1.5f, 0.0f, -6.0f); // Move left by 1.5 units and into screen by 6.0 units.
    glRotatef(rtri, 0.0f, 1.0f, 0.0f); // Rotate the triangle on Y-Axis.
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color.
    glVertex3f(0.0f, 1.0f, 0.0f); // Top point.
    glColor3f(0.0f, 1.0f, 0.0f); // Green color.
    glVertex3f(-1.0f, -1.0f, 0.0f); // Bottom left point.
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color.
    glVertex3f(1.0f, -1.0f, 0.0f); // Bottom right point.
    glEnd();

    if(rtri >= 360.0f)
    {
        rtri = 0.0f;
    }

    rtri += (0.2f * rotationDirection); // Increase the rotation variable;
}

GLvoid drawSquare(GLvoid)
{
    glLoadIdentity(); // Reset current model-view matrix.
    glTranslatef(1.5f, 0.0f, -6.0f); // Move right by 1.5 units and into screen by 6.0 units.
    glRotatef(rquad, 1.0f, 0.0f, 0.0f); // Rotate the quad on X-Axis.
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 1.0f); // Blue color.
    glVertex3f(-1.0f, 1.0f, 0.0f); // Top left point.
    glVertex3f(1.0f, 1.0f, 0.0f); // Top right point.
    glVertex3f(1.0f, -1.0f, 0.0f); // Bottom right point.
    glVertex3f(-1.0f, -1.0f, 0.0f); // Bottom left point.
    glEnd();

    if(rquad <= -360.0f)
    {
        rquad = 0.0f;
    }

    rquad -= (0.15f * rotationDirection); // Decrease the rotation variable.
}

// This is where we do drawing.
int drawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear screen and depth buffer.
    drawTriangle();
    drawSquare();
    return TRUE; // Everything is ok.
}

// Clean-up
GLvoid killGLWindow(GLvoid)
{
    // If we are in full-screen mode, exit from full-screen first.
    if(fullscreen)
    {
        ChangeDisplaySettings(NULL, 0); // Switch back to normal mode.
        ShowCursor(TRUE); // Show cursor.
    }

    // If we have rendering context, then release it.
    if(hRenderingContext)
    {
        if(!wglMakeCurrent(NULL, NULL))
        {
            // If context is not released, show error message.
            MessageBox(NULL, TEXT("Failed to release device and rendering context."), TEXT("Error!"), MB_OK | MB_ICONINFORMATION);
        }

        if(!wglDeleteContext(hRenderingContext))
        {
            // Error deleting rendering context.
            MessageBox(NULL, TEXT("Failed to delete rendering context."), TEXT("Error!"), MB_OK | MB_ICONINFORMATION);
        }

        hRenderingContext = NULL; // Set rendering context to NULL.
    }

    // If we have device context, then release it.
    if(hDeviceContext && !ReleaseDC(hWindow, hDeviceContext))
    {
        // Error releasing device context.
        MessageBox(NULL, TEXT("Failed to release device context."), TEXT("Error!"), MB_OK | MB_ICONINFORMATION);
        hDeviceContext = NULL;
    }

    // Destroy window.
    if(hWindow && !DestroyWindow(hWindow))
    {
        // Error in destroying window handle.
        MessageBox(NULL, TEXT("Failed to destroy window handle."), TEXT("Error!"), MB_OK | MB_ICONINFORMATION);
        hWindow = NULL;
    }

    // Unregister class so that we do not get "Window class already registered" error.
    if(!UnregisterClass(className, hInstance))
    {
        // Error in unregistering class.
        MessageBox(NULL, TEXT("Failed to unregister window class."), TEXT("Error!"), MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;
    }
}

// Setup window mode to full-screen.
BOOL switchFullscreen(int width, int height, int bits)
{
    long fullscreenResult;
    int messageBoxResult = -1;

    // If full-screen is enabled, then change window mode to full-screen.
    if(fullscreen)
    {
        DEVMODE dmScreenSettings; // Device mode.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings)); // Clear memory to 0.
        dmScreenSettings.dmSize = sizeof(dmScreenSettings); // Set size. Must be equal to size-of DEVMODE struct.
        dmScreenSettings.dmPelsWidth = width; // Set screen width.
        dmScreenSettings.dmPelsHeight = height; // Set screen height.
        dmScreenSettings.dmBitsPerPel = bits; // Set bits per pixel.
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT; // Set which fields are provided.

        // Try to set full-screen mode.
        // CDS_FULLSCREEN is used to remove taskbar.
        fullscreenResult = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        if(fullscreenResult != DISP_CHANGE_SUCCESSFUL)
        {
            // If unable to set full-screen mode, ask user to continue in window mode.
            messageBoxResult = MessageBox(NULL, TEXT("Full screen mode is not supported. Do you want to continue in window mode?"), TEXT("Full screen error"), MB_YESNO | MB_ICONEXCLAMATION);

            if(messageBoxResult == IDYES)
            {
                // Switch to window mode.
                fullscreen = FALSE;
            }
            else
            {
                // Exit from program.
                MessageBox(NULL, TEXT("Program will now close."), TEXT("Error!"), MB_OK | MB_ICONSTOP);
                return FALSE;
            }
        }
    }

    return TRUE; // Everything is ok.
}

// Setup pixel formatter
BOOL initPixelFormatter(int bits)
{
    GLuint pixelFormat; // Result of search for suitable pixel format.

    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), // Set size.
        1, // Version number.
        PFD_DRAW_TO_WINDOW | // Pixel format must support window drawing.
        PFD_SUPPORT_OPENGL | // Pixel format must support OpenGL drawing.
        PFD_DOUBLEBUFFER, // Pixel format must support double-buffering.
        PFD_TYPE_RGBA, // Pixel format must support RGBA color format.
        bits, // Bits per color.
        0, 0, 0, 0, 0, 0, // Ignore color bits.
        0, // No alpha buffer.
        0, // Ignore shift bit.
        0, // Ignore accumulation buffer.
        0, 0, 0, 0, // Ignore accumulation bits.
        bits, // Z-buffer bits (depth buffer)
        0, // No stencil buffer.
        0, // No auxiliary buffer.
        PFD_MAIN_PLANE, // Main drawing layer.
        0, // Reserved.
        0, 0, 0 // Ignore layer masks.
    };

    hDeviceContext = GetDC(hWindow); // Get device context.

    if(!hDeviceContext)
    {
        // If no device context, exit.
        killGLWindow();
        MessageBox(NULL, TEXT("Failed to create OpenGL device context."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Find a suitable pixel format.
    pixelFormat = ChoosePixelFormat(hDeviceContext, &pfd);

    if(!pixelFormat)
    {
        // If no suitable pixel format found, exit.
        killGLWindow();
        MessageBox(NULL, TEXT("Failed to find suitable PixelFormat."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Apply the selected suitable pixel format.
    if(!SetPixelFormat(hDeviceContext, pixelFormat, &pfd))
    {
        // Failed to set pixel format.
        killGLWindow();
        MessageBox(NULL, TEXT("Failed to set suitable pixel format."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Create a rendering context.
    hRenderingContext = wglCreateContext(hDeviceContext);

    if(!hRenderingContext)
    {
        // If failed to create rendering context, exit.
        killGLWindow();
        MessageBox(NULL, TEXT("Failed to create OpenGL rendering context."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Activate the rendering context.
    if(!wglMakeCurrent(hDeviceContext, hRenderingContext))
    {
        // Error in activating rendering context.
        killGLWindow();
        MessageBox(NULL, TEXT("Failed to activate OpenGL rendering context."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    return TRUE; // Everything is ok.
}

// Register window class
BOOL registerWindowClass()
{
    WNDCLASSEX wndclass; // Extended window class struct.

    hInstance = GetModuleHandle(NULL); // Get instance of our window.

    wndclass.cbSize = sizeof(wndclass); // Set struct size.
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw window horizontal and vertical on move and on device context.
    wndclass.cbClsExtra = 0; // No extra class data.
    wndclass.cbWndExtra = 0; // No extra window data.
    wndclass.lpfnWndProc = (WNDPROC)WndProc; // Window message and event handler.
    wndclass.hIcon = LoadIcon(NULL, IDI_WINLOGO); // Application icon.
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW); // Application cursor.
    wndclass.hbrBackground = NULL; // No background for window context.
    wndclass.hInstance = hInstance; // Application instance.
    wndclass.lpszClassName = className; // Class name.
    wndclass.lpszMenuName = NULL; // No menu.
    wndclass.hIconSm = LoadIcon(NULL, IDI_WINLOGO); // Application small icon.

    // Register class.
    if(!RegisterClassEx(&wndclass))
    {
        MessageBox(NULL, TEXT("Failed to register window class."), TEXT("Error!"), MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    return TRUE; // Everything is ok.
}

BOOL createGLWindow(LPCSTR title, int width, int height, int bits, BOOL fullscreenFlag)
{
    DWORD dwExStyle; // Window extended style.
    DWORD dwStyle; // Window style.
    RECT windowRect; // Window rect.

    // Set window bounds.
    windowRect.left = 0L;
    windowRect.top = 0L;
    windowRect.right = (long)width;
    windowRect.bottom = (long)height;

    fullscreen = fullscreenFlag; // Set full-screen status

    // Register window class.
    if(!registerWindowClass())
    {
        return FALSE;
    }

    // Set full-screen mode.
    if(!switchFullscreen(width, height, bits))
    {
        return FALSE;
    }

    if(fullscreen)
    {
        dwExStyle = WS_EX_APPWINDOW; // Allow window to go beyond taskbar.
        dwStyle = WS_POPUP; // Remove window border.
        ShowCursor(FALSE); // Hide cursor.
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; // Allow window to go beyond taskbar and have 3D look.
        dwStyle = WS_OVERLAPPEDWINDOW; // Normal window style.
    }

    // Set window style and rect.
    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

    // Create window.
    hWindow = CreateWindowEx(dwExStyle, // Extended window style.
        className, // Window class name.
        windowTitle, // Window title.
        WS_CLIPSIBLINGS | // Required for OpenGL.
        WS_CLIPCHILDREN | // Required for OpenGL.
        dwStyle, // Window style.
        0, // Window position x.
        0, // Window position y.
        windowRect.right - windowRect.left, // Adjusted window width.
        windowRect.bottom - windowRect.top, // Adjusted window height.
        NULL, // No parent window.
        NULL, // No menu.
        hInstance, // Application instance.
        NULL); // Do not pass anything to WM_CREATE.

    if(!hWindow)
    {
        // If failed to create window, exit.
        killGLWindow();
        MessageBox(NULL, TEXT("Failed to create window."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Setup pixel format.
    if(!initPixelFormatter(bits))
    {
        return FALSE;
    }

    ShowWindow(hWindow, SW_SHOW); // Show window.
    UpdateWindow(hWindow); // Update window.
    SetForegroundWindow(hWindow); // Slightly higher priority.
    SetFocus(hWindow); // Sets keyboard focus to the window.
    resizeGLScene(width, height); // Setup perspective OpenGL view.

    // Initialize OpenGL
    if(!initOpenGL())
    {
        killGLWindow();
        MessageBox(NULL, TEXT("Initialization failed."), TEXT("Error!"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    return TRUE; // Everything is ok.
}

BOOL toggleFullscreenMode()
{
    if(keys[VK_F11])
    {
        keys[VK_F11] = FALSE;
        killGLWindow();
        fullscreen = !fullscreen;
        if(!createGLWindow(windowTitle, fullscreen ? windowWidthFullscreen : windowWidth, fullscreen ? windowHeightFullscreen : windowHeight, bitsPerColor, fullscreen))
        {
            return FALSE;
        }
    }

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWindow, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch(iMessage)
    {
        case WM_ACTIVATE:
            active = HIWORD(wParam) ? FALSE : TRUE;
            return 0;

        case WM_SYSCOMMAND:
            switch(wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }

            // Use break as we need to have default processing on this messages.
            break;

        case WM_KEYDOWN:
            keys[wParam] = TRUE;
            return 0;

        case WM_KEYUP:
            keys[wParam] = FALSE;
            return 0;

        case WM_SIZE:
            resizeGLScene(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }

    return DefWindowProc(hWindow, iMessage, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    BOOL done = FALSE;
    int messageBoxResult = -1;

    messageBoxResult = MessageBox(NULL, TEXT("Enable full screen mode?"), TEXT("Launch Mode"), MB_YESNO | MB_ICONQUESTION);

    fullscreen = (messageBoxResult == IDYES);

    messageBoxResult = -1;

    if(!createGLWindow(windowTitle, fullscreen ? windowWidthFullscreen : windowWidth, fullscreen ? windowHeightFullscreen : windowHeight, bitsPerColor, fullscreen))
    {
        return 0;
    }

    while(!done)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                done = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(active)
            {
                if(keys[VK_ESCAPE])
                {
                    done = TRUE;
                }
                else
                {
                    drawGLScene();
                    SwapBuffers(hDeviceContext);
                }

                if(keys[VK_F11])
                {
                    if(!toggleFullscreenMode())
                    {
                        return 0;
                    }
                }

                if(keys['T'])
                {
                    keys['T'] = FALSE;
                    rotationDirection *= -1.0f;
                }
            }
        }
    }

    killGLWindow();
    return ((int)msg.wParam);
}
