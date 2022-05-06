#include <Unknwn.h>
#undef GetCurrentTime
#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.Windows.ApplicationModel.DynamicDependency.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Dispatching.h>


#include <windows.foundation.h>
#include <winrt/Windows.Foundation.h>

#include <microsoft.ui.xaml.media.dxinterop.h>

#include <inspectable.h>

#include <functional>

#include <d3d11_1.h>


// Undocumented static initializer for MddBootstrap; using it instead of documented
// APIs for now, to be closer to what Visual Studio does
#include <MddBootstrapAutoInitializer.cpp>

namespace muxc = winrt::Microsoft::UI::Xaml::Controls;

#include <EGL/egl.h>
#include <GLES2/gl2.h>

// The following properties can be set on the CoreApplication to support additional
// ANGLE configuration options.
//
// The Visual Studio sample templates provided with this version of ANGLE have examples
// of how to set these property values.

//
// Property: EGLNativeWindowTypeProperty
// Type: IInspectable
// Description: Set this property to specify the window type to use for creating a surface.
//              If this property is missing, surface creation will fail.
//
const wchar_t EGLNativeWindowTypeProperty[] = L"EGLNativeWindowTypeProperty";

//
// Property: EGLRenderSurfaceSizeProperty
// Type: Size
// Description: Set this property to specify a preferred size in pixels of the render surface.
//              The render surface size width and height must be greater than 0.
//              If this property is set, then the render surface size is fixed.
//              The render surface will then be scaled to the window dimensions.
//              If this property is missing, a default behavior will be provided.
//              The default behavior uses the window size if a CoreWindow is specified or
//              the size of the SwapChainPanel control if one is specified.
//
const wchar_t EGLRenderSurfaceSizeProperty[] = L"EGLRenderSurfaceSizeProperty";

//
// Property: EGLRenderResolutionScaleProperty
// Type: Single
// Description: Use this to specify a preferred scale for the render surface compared to the window.
//              For example, if the window is 800x480, and:
//                - scale is set to 0.5f then the surface will be 400x240
//                - scale is set to 1.2f then the surface will be 960x576
//              If the window resizes or rotates then the surface will resize accordingly.
//              EGLRenderResolutionScaleProperty and EGLRenderSurfaceSizeProperty cannot both be set.
//              The scale factor should be > 0.0f.
//
const wchar_t EGLRenderResolutionScaleProperty[] = L"EGLRenderResolutionScaleProperty";


#pragma comment(lib, "libEGL.lib")
#pragma comment(lib, "libGLESv2.lib")

#if 1

EGLDisplay display;
EGLContext context;
EGLSurface surface;

static bool initializeAngle(EGLNativeWindowType win)
{
    // setup EGL
    EGLint configAttribList[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     8,
        EGL_STENCIL_SIZE,   8,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    EGLint surfaceAttribList[] = {
        EGL_NONE, EGL_NONE
    };

    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig config;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    EGLint error;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    error = eglGetError();

    auto eglres = eglInitialize(display, &majorVersion, &minorVersion);
    error = eglGetError();


    eglChooseConfig(display, configAttribList, &config, 1, &numConfigs);
    error = eglGetError();

    context = eglCreateContext(display, config, NULL, NULL);
    error = eglGetError();

    surface = eglCreateWindowSurface(display, config, win, NULL);
    error = eglGetError();

    EGLint width;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    error = eglGetError();

    eglMakeCurrent(display, surface, surface, context);
    error = eglGetError();

    return true;
}

GLuint LoadShader(GLenum type, const char* shaderSrc)
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if (shader == 0)
        return 0;

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void draw()
{
    GLchar vShaderStr[] =
        "attribute vec4 vPosition;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vPosition;  \n"
        "}                            \n";

    GLchar fShaderStr[] =
        "precision mediump float;\n"
        "void main()                                  \n"
        "{                                            \n"
        "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
        "}                                            \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    // Create the program object
    programObject = glCreateProgram();

    if (programObject == 0)
        return;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Bind vPosition to attribute 0
    glBindAttribLocation(programObject, 0, "vPosition");

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);


    GLfloat vVertices[] =
    { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f };



    // Use the program object
    glUseProgram(programObject);

    // Load the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

#endif

//winrt::com_ptr<ID3D11Device> device;
//winrt::com_ptr<ID3D11DeviceContext> context;
//winrt::com_ptr<IDXGISwapChain1> swapChain;
//winrt::com_ptr<ID3D11RenderTargetView> rtView;

#if 0
void initDx()
{
    D3D_FEATURE_LEVEL featureLevels[] =
    {
     D3D_FEATURE_LEVEL_11_1,
     D3D_FEATURE_LEVEL_11_0 ,
     D3D_FEATURE_LEVEL_10_1,
     D3D_FEATURE_LEVEL_10_0,
     D3D_FEATURE_LEVEL_9_3,
     D3D_FEATURE_LEVEL_9_2,
     D3D_FEATURE_LEVEL_9_1
    };
    D3D_FEATURE_LEVEL featureLevel;

    winrt::check_hresult(
        D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE, 0, 0,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
        device.put(), &featureLevel, context.put())
        );
    
    DXGI_SWAP_CHAIN_DESC1 dxgi = {};
    dxgi.BufferCount = 1;									// バッファの数
    dxgi.Width = 1920;		// バッファの横幅
    dxgi.Height = 1080;		// バッファの縦幅
    dxgi.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// カラーフォーマット
    dxgi.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;	// バッファの使い方 Usage => 使用方法
    dxgi.SampleDesc.Count = 1;								// マルチサンプリングのサンプル数(未使用は1)
    dxgi.SampleDesc.Quality = 0;							// マルチサンプリングの品質(未使用は0)
    dxgi.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    dxgi.BufferCount = 2;
    dxgi.Scaling = DXGI_SCALING_STRETCH;
    dxgi.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    dxgi.Flags = 0;

    winrt::com_ptr<IDXGIDevice> dxgiDevice = device.as<IDXGIDevice>();
    winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
    dxgiDevice->GetAdapter(dxgiAdapter.put());

    winrt::com_ptr<IDXGIFactory2> dxgiFactory;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), dxgiFactory.put_void());

    winrt::check_hresult(
        dxgiFactory->CreateSwapChainForComposition(device.get(), &dxgi, nullptr, swapChain.put())
        );




}
#endif

struct MainWindow : public winrt::Microsoft::UI::Xaml::WindowT<MainWindow>
{
private:
  muxc::NavigationView mNav;

  muxc::Frame mContent;

  muxc::NavigationViewItem mNav1, mNav2, mNav3;

  muxc::SwapChainPanel mPanel;

public:
    void initEGL()
    {
        ::IInspectable* ii;
        auto res = mPanel.as(__uuidof(::IInspectable), (void**)&ii);

        EGLNativeWindowType win = static_cast<EGLNativeWindowType>(ii);

        initializeAngle(win);
  }

  MainWindow()
  {
    Title(L"CMake, C++/WinRT, WinUI 3, and ANGLE Demo App");

    mNav.Header(winrt::box_value(L"CMake, C++/WinRT, and WinUI 3 Demo App"));
    mNav.Content(mContent);
    mNav.IsSettingsVisible(false);

    mNav1.Content(winrt::box_value(L"Item 1"));
    mNav1.Icon(muxc::SymbolIcon(muxc::Symbol::Play));
    mNav.MenuItems().Append(mNav1);

    mNav2.Content(winrt::box_value(L"Item 2"));
    mNav2.Icon(muxc::SymbolIcon(muxc::Symbol::Copy));
    mNav.MenuItems().Append(mNav2);

    mNav3.Content(winrt::box_value(L"Item 3"));
    mNav3.Icon(muxc::SymbolIcon(muxc::Symbol::Paste));
    mNav.MenuItems().Append(mNav3);

    mNav.IsSettingsVisible(true);

    mNav.SelectionChanged({this, &MainWindow::OnSelectionChanged});

    mNav.SelectedItem(mNav1);

//    mNav.Content(mPanel);

    Content(mPanel);

    mPanel.Loaded([this](winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
        initEGL();

        eglMakeCurrent(display, surface, surface, context);
        auto error = eglGetError();


        // Set the viewport
        glViewport(0, 0, 500, 500);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        draw();

        eglSwapBuffers(display, surface);
        error = eglGetError();

    });

#if 0
    initDx();

    auto dispatcherQueue = mPanel.DispatcherQueue();
    auto dependencyObject = mPanel.as<winrt::Microsoft::UI::Xaml::IDependencyObject>();
    dispatcherQueue.HasThreadAccess();
    dispatcherQueue.TryEnqueue([]() {
        });

    auto token = mPanel.SizeChanged([](winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const&) {
        });

    auto swapChainNative = mPanel.as<ISwapChainPanelNative>();
    swapChainNative->SetSwapChain(swapChain.get());

    Content(mPanel);

    winrt::com_ptr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), backBuffer.put_void());

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
    renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    winrt::check_hresult(
        device->CreateRenderTargetView(backBuffer.get(), &renderTargetViewDesc, rtView.put())
    );

    auto rtv = rtView.get();
    context->OMSetRenderTargets(1, &rtv, nullptr);

    const float cl[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(rtView.get(), cl);

    winrt::check_hresult(swapChain->Present(1, 0));
#endif
//    ABI::Windows::Foundation::Collections::IPropertySet is;
#if 0
    winrt::Windows::Foundation::Collections::PropertySet surfaceProperties;
    surfaceProperties.Insert(EGLNativeWindowTypeProperty, mPanel);
    EGLNativeWindowType win = static_cast<EGLNativeWindowType>(winrt::get_abi(surfaceProperties));

    ::IInspectable* ii;
    auto res = mPanel.as(__uuidof(::IInspectable), (void**)&ii);
    ::HSTRING str;
    res = ii->GetRuntimeClassName(&str);

    UINT32 len;
    auto strbuf = WindowsGetStringRawBuffer(str, &len);

    winrt::com_ptr<::IInspectable> x;
    x.attach(ii);
    auto y = x.as<muxc::SwapChainPanel>();


    x.try_as<muxc::SwapChainPanel>();

    Content(y);
#endif


#if 0

    ::HSTRING str;
    res = ii->GetRuntimeClassName(&str);
    
    UINT32 len;
    auto strbuf = WindowsGetStringRawBuffer(str, &len);

    ::ABI::Windows::UI::Xaml::Controls::ISwapChainPanel* is;
    res = mPanel.as(__uuidof(::ABI::Windows::UI::Xaml::Controls::ISwapChainPanel), (void**)&is);

    ::ABI::Microsoft::UI::Xaml::Controls::ISwapChainPanel

    //auto x = mPanel.as<::winrt::Microsoft::UI::Xaml::Controls::ISwapChainPanel>();

    //auto guid = winrt::guid_of<::winrt::Microsoft::UI::Xaml::Controls::ISwapChainPanel>();
    //::winrt::Microsoft::UI::Xaml::Controls::ISwapChainPanel* is;
    //res = mPanel.as(winrt::guid_of<::winrt::Microsoft::UI::Xaml::Controls::ISwapChainPanel>, (void**)&is);

    winrt::com_ptr<::winrt::Microsoft::UI::Xaml::Controls::ISwapChainPanel> ptr{ mPanel.as<::winrt::Microsoft::UI::Xaml::Controls::ISwapChainPanel>() };

//    auto inspectable = mPanel.as<::IInspectable*>();

//    EGLNativeWindowType win = static_cast<EGLNativeWindowType>(winrt::get_abi(mPanel));
#endif
  }

  void OnSelectionChanged(const IInspectable &, const muxc::NavigationViewSelectionChangedEventArgs &args)
  {
    if (args.IsSettingsSelected())
    {
      mContent.Content(winrt::box_value(L"Settings"));
      return;
    }

    const auto &item = args.SelectedItem();
    if (item == mNav1)
    {
      mContent.Content(winrt::box_value(L"Item 1"));
//      initEGL();
      return;

    }

    if (item == mNav2)
    {
      mContent.Content(winrt::box_value(L"Item 2"));
      return;
    }

    if (item == mNav3)
    {
      mContent.Content(winrt::box_value(L"Item 3"));
      return;
    }
  }
};

struct MyApp : public winrt::Microsoft::UI::Xaml::ApplicationT<MyApp, winrt::Microsoft::UI::Xaml::Markup::IXamlMetadataProvider>
{
private:
  winrt::Microsoft::UI::Xaml::XamlTypeInfo::XamlControlsXamlMetaDataProvider mXamlControlsMetaDataProvider;

public:
  MyApp()
  {
    ::winrt::Windows::Foundation::Uri resourceLocator{L"ms-appx:///App.xaml"};
    ::winrt::Microsoft::UI::Xaml::Application::LoadComponent(*this, resourceLocator);

    UnhandledException([this](IInspectable const &, winrt::Microsoft::UI::Xaml::UnhandledExceptionEventArgs const &e)
                       {
        if (IsDebuggerPresent()) {
            auto errorMessage = e.Message();
            __debugbreak();
        } });
  }

  void OnLaunched(const winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs &)
  {
      winrt::make<MainWindow>().Activate();
  }

  winrt::Microsoft::UI::Xaml::Markup::IXamlType
  GetXamlType(const winrt::hstring &fullName)
  {
    return mXamlControlsMetaDataProvider.GetXamlType(fullName);
  }

  winrt::Microsoft::UI::Xaml::Markup::IXamlType
  GetXamlType(const winrt::Windows::UI::Xaml::Interop::TypeName &type)
  {
    return mXamlControlsMetaDataProvider.GetXamlType(type);
  }

  winrt::com_array<winrt::Microsoft::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions()
  {
    return mXamlControlsMetaDataProvider.GetXmlnsDefinitions();
  }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  winrt::init_apartment(winrt::apartment_type::single_threaded);

  winrt::Microsoft::UI::Xaml::Application::Start(
      [](auto &&)
      { winrt::make<MyApp>(); });
  return 0;
}
