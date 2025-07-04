#include "PreCompile.h"
#include "Nimet.h"

#include "UI.h"
#include "ResourceWatcher.h"
#include "CameraController.h"
#include "CurrentScene.h"
#include "Log.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "resource.h"

HINSTANCE hInstance;

HRESULT STDMETHODCALLTYPE DragDrop::Drop( 
  /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
  /* [in] */ DWORD grfKeyState,
  /* [in] */ POINTL pt,
  /* [out][in] */ __RPC__inout DWORD *pdwEffect)
{
  FORMATETC fmtetc;
  fmtetc.cfFormat = CF_HDROP;
  fmtetc.ptd = NULL;
  fmtetc.dwAspect = DVASPECT_CONTENT;
  fmtetc.lindex = -1;
  fmtetc.tymed = TYMED_HGLOBAL;

  // user has dropped on us -- get the CF_HDROP data from drag source
  STGMEDIUM medium;
  HRESULT hr = pDataObj->GetData(&fmtetc, &medium);

  if (!FAILED(hr))
  {
    // grab a pointer to the data
    HGLOBAL HFiles = medium.hGlobal;
    HDROP HDrop = (HDROP)GlobalLock(HFiles);

    UINT uNumFiles;
    TCHAR szNextFile[MAX_PATH];

    // Get the # of files being dropped.
    uNumFiles = DragQueryFile(HDrop, -1, NULL, 0);

    for(UINT uFile = 0; uFile < uNumFiles; uFile++)
    {
      // Get the next filename from the HDROP info.
      if(DragQueryFile(HDrop, uFile, szNextFile, MAX_PATH) > 0)
      {
        Nimet::get().FileDropped(boost::filesystem::path(szNextFile));
        int s = 2;
      }

    }

    // call the helper routine which will notify the Form
    // of the drop
    // HandleDrop(HDrop);

    // release the pointer to the memory
    GlobalUnlock(HFiles);
    ReleaseStgMedium(&medium);
  }
  else
  {
    *pdwEffect = DROPEFFECT_NONE;
    return hr;
  }
  return NOERROR; 
}



void showWin32Console()
{
  static const WORD MAX_CONSOLE_LINES = 500;
  int hConHandle;
  long lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;
  // allocate a console for this app
  AllocConsole();

  HWND hwnd = NULL;
  SetConsoleTitle(L"Nimet");

  /* disable the close button for now */
  while(!hwnd)
  {
    hwnd = FindWindow(NULL, L"Nimet");
    Sleep(100);
  }

  //DeleteMenu(GetSystemMenu(hwnd, 0), SC_CLOSE, MF_BYCOMMAND);

  // set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;
  coninfo.dwSize.X = 1024;
  coninfo.dwMaximumWindowSize.X = 1024;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
  // redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "w");
  *stdout= *fp;
  setvbuf(stdout, NULL, _IONBF, 0);
  // redirect unbuffered STDIN to the console
  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "r");
  *stdin= *fp;
  setvbuf(stdin, NULL, _IONBF, 0);
  // redirect unbuffered STDERR to the console
  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "w");
  *stderr= *fp;
  setvbuf(stderr, NULL, _IONBF, 0);
  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
  // point to console as well
  std::ios::sync_with_stdio();
}

void OnDropFiles(HDROP hdrop)
{
  UINT uNumFiles;
  TCHAR szNextFile[MAX_PATH];

  // Get the # of files being dropped.
  uNumFiles = DragQueryFile(hdrop, -1, NULL, 0);

  for(UINT uFile = 0; uFile < uNumFiles; uFile++)
  {
    // Get the next filename from the HDROP info.
    if(DragQueryFile(hdrop, uFile, szNextFile, MAX_PATH) > 0)
    {
      boost::filesystem::path p(szNextFile);



    }
  }

  // Free up memory.
  DragFinish(hdrop);
}

#endif

void MyPump()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  // Windows Message Loop (NULL means check all HWNDs belonging to this context)
  MSG msg;

  while(PeekMessage(&msg, Nimet::get().hwnd, 0, 0, PM_REMOVE))
  {
    switch(msg.message)
    {
    case WM_DROPFILES:
      OnDropFiles((HDROP)msg.wParam);
      break;
    default:
      break;
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
  //GLX Message Pump
  Windows::iterator win = _msWindows.begin();
  Windows::iterator end = _msWindows.end();

  Display* xDisplay = 0;// same for all windows

  for (; win != end; win++)
  {
    XID xid;
    XEvent event;

    if (!xDisplay)
      (*win)->getCustomAttribute("XDISPLAY", &xDisplay);

    (*win)->getCustomAttribute("WINDOW", &xid);

    while (XCheckWindowEvent (xDisplay, xid, StructureNotifyMask | VisibilityChangeMask | FocusChangeMask, &event))
    {
      GLXProc(*win, event);
    }

    // The ClientMessage event does not appear under any Event Mask
    while (XCheckTypedWindowEvent (xDisplay, xid, ClientMessage, &event))
    {
      GLXProc(*win, event);
    }
  }
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !defined __OBJC__ && !defined __LP64__
  // OSX Message Pump
  EventRef event = NULL;
  EventTargetRef targetWindow;
  targetWindow = GetEventDispatcherTarget();

  // If we are unable to get the target then we no longer care about events.
  if( !targetWindow ) return;

  // Grab the next event, process it if it is a window event
  while( ReceiveNextEvent( 0, NULL, kEventDurationNoWait, true, &event ) == noErr )
  {
    // Dispatch the event
    SendEventToEventTarget( event, targetWindow );
    ReleaseEvent( event );
  }
#endif
}

//-------------------------------------------------------------------------------------
Nimet::Nimet(void)
  : mRoot(0),
  mCamera(0),
  mSceneMgr(0),
  mWindow(0),
  mResourcesCfg(Ogre::StringUtil::BLANK),
  mPluginsCfg(Ogre::StringUtil::BLANK),
  keyboard(0),
  mouse(0),
  inputManager(0),
  ui(0),
  overlaySystem(0),
  hwnd(0),
  lastBatchCount(0),
  isActive(true),
  viewport(0),
  restart(false),
  endProgram(false),
  dragDrop(0),
  renderTimeSlice(1000000/settings.framesPerSecond)
{

}
//-------------------------------------------------------------------------------------
Nimet::~Nimet(void)
{
  if(dragDrop)
    delete dragDrop;
}

void Nimet::SetFrameLimit(int limit, bool isCallback)
{
  if(limit <= 1)
    return;

  settings.framesPerSecond = limit;
  renderTimeSlice  = 1000000 / settings.framesPerSecond;

  std::wstringstream ss;

  ss << L"$('#FPSLimitDisplay').text('" << limit << L"');";
  if(!isCallback)
  {
    ss << L"$('#FPSLimit').val(";
    ss << limit;
    ss << L");";
  }

  UI::get().ExecuteJavascriptWhenLoaded(ss.str());
}

void Nimet::SetFrameLimiterStatus(bool status)
{
  settings.frameLimiterStatus = status;

  /**/
  if(settings.frameLimiterStatus)
    UI::get().ExecuteJavascriptWhenLoaded(L"SetFrameLimiter(true);");
  else
    UI::get().ExecuteJavascriptWhenLoaded(L"SetFrameLimiter(false);");

}

void Nimet::ToggleFrameLimiterStatus() 
{
  SetFrameLimiterStatus(!settings.frameLimiterStatus);
}

bool Nimet::go(void)
{
#ifdef _DEBUG
  mPluginsCfg = "plugins_d.cfg";
#else
  mPluginsCfg = "plugins.cfg";
#endif


  {
    Ogre::LogManager *manager = new Ogre::LogManager();
    Ogre::Log *log = manager->createLog("Default", true, true, true);
    log->addListener(&Log::get()); // .addListener(Log::get());

    Log::get().defaultLog = log;
    Log::get().logManager = manager;
  }

  // construct Ogre::Root
  mRoot = new Ogre::Root(mPluginsCfg, "ogre.cfg", "");

  overlaySystem = new Ogre::OverlaySystem();

  if(mRoot->restoreConfig() || mRoot->showConfigDialog())
  {

    mWindow = mRoot->initialise(true, "Nimet");
    // mWindow->setVSyncEnabled(true);
    mWindow->getCustomAttribute("WINDOW", &hwnd);

    {
      OleInitialize(NULL);
      dragDrop = new DragDrop();
      CoLockObjectExternal(dragDrop, true, true);
      // register the Memo as a drop target
      HRESULT h1 = RegisterDragDrop(hwnd, dragDrop); 
    }

    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
  }
  else
  {
    return false;
  }

  //Register as a Window listener
  Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

  // mWindow->setDeactivateOnFocusChange(false);

  SetupOIS();

  mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

  mCamera = mSceneMgr->createCamera("PlayerCam");
  mCamera->setAutoAspectRatio(true);

  {
    viewport = mWindow->addViewport(mCamera);
    mCamera->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));

    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    {
      CurrentSceneSettings &currentSceneSettings = CurrentScene::get().GetCurrentSceneSettings();
      mSceneMgr->setAmbientLight(Ogre::ColourValue(currentSceneSettings.ambientColour.r, currentSceneSettings.ambientColour.g, currentSceneSettings.ambientColour.b));
      mCamera->getViewport()->setBackgroundColour(Ogre::ColourValue(currentSceneSettings.backgroundColour.r, currentSceneSettings.backgroundColour.g, currentSceneSettings.backgroundColour.b));
    }

#ifdef _DEBUG
    HTMLManager::get().Init(keyboard, 9222);
#else
    HTMLManager::get().Init(keyboard);
#endif

    ui = UI::getPtr();
    ui->init(viewport, keyboard);
  }

  CameraController::get().init(mCamera, keyboard, mouse);

  CurrentAsset::get().sceneManager = mSceneMgr;
  CurrentAsset::get().node = mSceneMgr->getRootSceneNode()->createChildSceneNode();

  resourceWatcher = &ResourceWatcher::get();

  resourceWatcher->AddScene(mSceneMgr);
  resourceWatcher->init();

  LoadSettings();

  {
    boost::filesystem::path meshPath("../../Assets/Mesh"); 
    boost::filesystem::path materialPath("../../Assets/Material");
    resourceWatcher->AddResourceDirectoryToQueue(meshPath, false);
    resourceWatcher->AddResourceDirectoryToQueue(materialPath, false);
  }

  resourceWatcher->EntityReloaded = std::bind(&CurrentAsset::EntityReloaded, &CurrentAsset::get(), std::placeholders::_1, std::placeholders::_2);

  if(!parameters.empty())
  {
    using namespace boost::filesystem;
    path file(parameters[0]);
    if(is_regular_file(file))
    {
      path dir = file.parent_path();
      resourceWatcher->AddResourceDirectoryToQueue(dir, false);
    }
  }

  resourceWatcher->ParseQueuedResourceDirectories();

  if(!parameters.empty())
  { 
    using namespace boost::filesystem;
    path file(parameters[0]);

    FileDropped(file);
  }

  CameraController::get().ApplySettings();

  mSceneMgr->addRenderQueueListener(overlaySystem);

  resourceWatcher->ResourceReloadedCallback = std::bind(&Nimet::ResouceReloaded, this, std::placeholders::_1, std::placeholders::_2);

  resourceWatcher->ShaderCompileFailed = std::bind(&Nimet::ShaderCompileFailed, this, std::placeholders::_1);

  resourceWatcher->ResourceModifiedActionCallback = std::bind(&Nimet::ResourceModifiedActionCallback, this, std::placeholders::_1, std::placeholders::_2);

  Ogre::Timer timeSinceLastFrame;
  while(true)
  {
    if(endProgram)
      return false;


    MyPump();

    if(mWindow->isClosed())
      return false;

    keyboard->capture();
    mouse->capture();

    if(lastBatchCount != mWindow->getBatchCount())
    {
      if(UI::get().UpdateBatchCount(mWindow->getBatchCount()))
        lastBatchCount = mWindow->getBatchCount();
    }

    resourceWatcher->update();

    CurrentAsset::get().Update(0.0010f);

    CameraController::get().Update(0.010f);

    HTMLManager::get().Update();

    // make sure window also rendered in the background
    if(!mWindow->isActive())
      mWindow->update();



    if(settings.frameLimiterStatus)
    {
      timeSinceLastFrame.reset();
      long t = 0;
      if(!mRoot->renderOneFrame())
        return false;
      else
      {
        t = timeSinceLastFrame.getMicroseconds();
        if( t < renderTimeSlice ) 
          std::this_thread::sleep_for(std::chrono::microseconds( std::chrono::microseconds(renderTimeSlice - t) ));

      }
    }
    else
    {
      if(!mRoot->renderOneFrame())
        return false;
    }
  }

  // never comes here
  return true;
}

void Nimet::windowResized(Ogre::RenderWindow* rw)
{
  if(ui)
  {
    const OIS::MouseState &ms = mouse->getMouseState();
    ms.width = rw->getWidth();
    ms.height = rw->getHeight();

    /* This two will trigger window.onresize */
    ui->ResizeTexture(rw->getWidth(), rw->getHeight());
    ui->ResizeOverlay(rw->getWidth(), rw->getHeight());

  }
}

void Nimet::FileDropped(const boost::filesystem::path &filePath)
{
  if(boost::filesystem::is_regular_file(filePath))
  {
    if(filePath.leaf().extension().string() == ".mesh")
    {
      boost::filesystem::path parent_path = filePath.parent_path();
      ResourceWatcher::get().AddResourceDirectoryToQueue(parent_path, false);
      ResourceWatcher::get().ParseScriptsInPath(parent_path);

      CurrentAsset::get().OpenMesh(filePath);
      CameraController::get().ReFocusToMesh();

      UI::get().UpdateCurrentAsset();
      UI::get().UpdateDirectories();

    }
    else if(filePath.leaf().extension().string() == ".cfg")
    {
      bool r = CurrentScene::get().resourceDirectories.LoadResourceCfgFile(filePath);

      if(r)
      {
        ResourceWatcher::get().ParseQueuedResourceDirectories();
        UI::get().UpdateDirectories();
        CurrentAsset::get().ForceReload();
      }

    }
    else if(filePath.leaf().extension().string() == ".scene")
    {
      boost::filesystem::path parent_path = filePath.parent_path();
      ResourceWatcher::get().AddResourceDirectoryToQueue(parent_path, false);
      ResourceWatcher::get().ParseQueuedResourceDirectories();

      CurrentAsset::get().OpenScene(filePath);
      UI::get().UpdateCurrentAsset();

    }
  }
  else
  {
    bool r = ResourceWatcher::get().AddResourceDirectory(filePath);
    if(r)
    {
      UI::get().AddDirectory(filePath);
      CurrentAsset::get().ForceReload();
    }
  }

}

void Nimet::LoadSettings()
{
  CameraController::get().LoadSettings();
  CurrentScene::get().LoadSettings();
  CurrentScene::get().LoadResourceSettings();

  CurrentScene::get().resourceDirectories.AddDirectortiesToQueue();

  SetFrameLimit(settings.framesPerSecond);
  SetFrameLimiterStatus(settings.frameLimiterStatus);
}

void Nimet::SaveSettings(bool onlyOptions)
{
  {
    boost::filesystem::path dir("../../Settings");
    if(!boost::filesystem::is_directory(dir))
    {
      bool r = boost::filesystem::create_directory(dir);
      if(!r)
      {
        UI::get().ExecuteJavascript(L"ShowMessage('Could not save settings.!', 3000, 'red', 'topCenter');");
        return;
      }
    }
  }

  CameraController::get().SaveSettings();
  CurrentScene::get().SaveSettings();
  if(!onlyOptions)
    CurrentScene::get().SaveResourceSettings();
  if(!onlyOptions)
    UI::get().ExecuteJavascript(L"ShowMessage('Settings Saved!', 3000, 'black', 'topCenter');");
}

void Nimet::ResouceReloaded(ResourceWatcher::ReloadType type, void* ref)
{
  switch(type)
  {
  case ResourceWatcher::ReloadType_Mesh:
    {
      std::wstringstream ss;
      Ogre::Entity *e = static_cast<Ogre::Entity*>(ref);
      ss << L"ShowMessage('Entity Reloaded: " << Ogre::StringConverter::UTF8toUTF16(e->getName()) << "', 2000, 'green', 'bottomRight')";
      UI::get().ExecuteJavascript(ss.str());
    }
  case ResourceWatcher::ReloadType_Material:
    {
      Ogre::Material *mat = static_cast<Ogre::Material*>(ref);

    }
    break;
  case ResourceWatcher::ReloadType_Shader:
    {
      Ogre::GpuProgram *shader = static_cast<Ogre::GpuProgram*>(ref);
      if(shader->hasCompileError())
      {
        ShaderCompileFailed(shader->getName());
        return;
      }
      std::wstringstream ss;
      //ShowMessage(msg, duration, color, position)

      ss << L"ShowMessage('Compile Successful: " << Ogre::StringConverter::UTF8toUTF16(shader->getName()) << "', 2000, 'green', 'bottomRight')";

      UI::get().ExecuteJavascript(ss.str());
    }
    break;
  default:
    break;
  }

}

void Nimet::ShaderCompileFailed(const std::string& shaderName)
{
  std::wstringstream ss;
  ss << L"ShowMessage('Compile Error: " << Ogre::StringConverter::UTF8toUTF16(shaderName) << ", check the log!', 3000, 'red', 'bottomRight')";
  UI::get().ExecuteJavascript(ss.str());
}

void Nimet::ResourceModifiedActionCallback(ResourceWatcher::ResourceModificationType type, const boost::filesystem::path& path)
{
  std::wstringstream ss;
  //function ShowMessage(msg, duration, color, position)
  ss << L"ShowMessage('";
  switch(type)
  {
  case ResourceWatcher::ResourceAdded:
    ss << L"Resource Added: " << path.leaf().wstring() << L"', 3000, 'black', 'bottomRight');";
    break;
  case ResourceWatcher::ResourceModified:
    ss << L"Resource Modified: " << path.leaf().wstring() << L"', 3000, 'black', 'bottomRight');";
    break;
  case ResourceWatcher::ResourceRemoved:
    ss << L"Resource Removed: " << path.leaf().wstring() << L"', 3000, 'black', 'bottomRight');";
    break;
  default:
    break;
  }

  UI::get().ExecuteJavascript(ss.str());
}

void Nimet::windowMoved(Ogre::RenderWindow* rw)
{
  if(ui)
  {
    ui->ResizeTexture(rw->getWidth(), rw->getHeight());
    ui->ResizeOverlay(rw->getWidth(), rw->getHeight());
  }
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

bool Nimet::keyReleased(const OIS::KeyEvent&arg)
{
  HTMLManager::get().KeyReleased(arg);

  if(arg.key == OIS::KC_RETURN)
  {
    if(keyboard->isKeyDown(OIS::KC_LMENU))
    {
      if(mWindow->isFullScreen())
        mWindow->setFullscreen(false, mWindow->getWidth(), mWindow->getHeight());
      else
        mWindow->setFullscreen(true, mWindow->getWidth(), mWindow->getHeight());

      windowResized(mWindow);
    }
  }

  return true;
}

void Nimet::SetupOIS()
{
  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
  OIS::ParamList pl;
  size_t windowHnd = 0;
  std::ostringstream windowHndStr;

  mWindow->getCustomAttribute("WINDOW", &windowHnd);
  windowHndStr << windowHnd;
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

  //show hardware mouse
#ifdef WIN32
  pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_BACKGROUND")));
  pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#endif

  inputManager = OIS::InputManager::createInputSystem(pl);

  keyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject(OIS::OISKeyboard, true));
  mouse = static_cast<OIS::Mouse*>(inputManager->createInputObject(OIS::OISMouse, true));

  unsigned int width, height, depth;
  int left, top;
  mWindow->getMetrics(width, height, depth, left, top);

  const OIS::MouseState &ms = mouse->getMouseState();
  ms.width = width;
  ms.height = height;

  keyboard->setEventCallback(this);
  mouse->setEventCallback(this);

}

bool Nimet::keyPressed(const OIS::KeyEvent &arg)
{
  HTMLManager::get().KeyPressed(arg);

  if(arg.key == OIS::KC_ESCAPE)
  {
    endProgram = true;
    restart = false;
  }

  return true;
}

bool Nimet::IsWindowActive()
{
  HWND activeWindow = GetForegroundWindow();
  if(activeWindow != 0)
  {
    if(activeWindow != hwnd)
      return false;
  }

  return true;
}

bool Nimet::mouseMoved(const OIS::MouseEvent& arg)
{
  OIS::MouseEvent& _arg = const_cast<OIS::MouseEvent&>(arg);

  if(!mWindow->isActive())
    const_cast<OIS::MouseState&>(_arg.state).Z.rel = 0;
  if(!IsWindowActive())
    const_cast<OIS::MouseState&>(_arg.state).Z.rel = 0;

  CameraController::get().MouseMoved(_arg);

  HTMLManager::get().MouseMoved(_arg);
  return true;
}

bool Nimet::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
  if(!mWindow->isActive())
    return true;

  // check if this Nimet window is the actual active window
  if(!IsWindowActive())
    return true;
  if(arg.state.X.abs == 0 || mWindow->getWidth() == arg.state.X.abs)
    return true;

  CameraController::get().MousePressed(arg, id);

  HTMLManager::get().MousePressed(id);
  return true;
}

bool Nimet::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
  CameraController::get().MouseReleased(arg, id);

  HTMLManager::get().MouseReleased(id);
  return true;
}

void Nimet::Destroy()
{
  isActive = false;

  ResourceWatcher::get().destroy();

  delete ui;

  HTMLManager::get().Destroy();

  delete overlaySystem;

  delete mRoot;

  Log::get().destroy();
}

std::wstring ToWString(const std::string& strText)
{
  std::wstring wstrResult;

  wstrResult.resize(strText.length());

  typedef std::codecvt<wchar_t, char, mbstate_t> widecvt;

  std::locale locGlob;

  std::locale::global(locGlob);

  const widecvt& cvt(std::use_facet<widecvt>(locGlob));

  mbstate_t State;

  const char* cTemp;
  wchar_t* wTemp;

  cvt.in(State, &strText[0], &strText[0] + strText.length(), cTemp, (wchar_t*)&wstrResult[0], &wstrResult[0] + wstrResult.length(), wTemp);

  return wstrResult;
}

#ifdef __cplusplus
extern "C"
{
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  //int main()
  INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT cmdShow)
#else
  //int main(int argc, char *argv[])
#endif
  {

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _DEBUG
    showWin32Console();
#endif
    hInstance = hInst;

    TCHAR path[2048] = { 0 };
    GetModuleFileName(NULL, path, 2048);
    boost::filesystem::path p(path);
    SetCurrentDirectory(p.parent_path().wstring().c_str());

    // Create application object
    Nimet &app = Nimet::get();

    int params;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &params);
    if(params > 1)
    {
      std::wstring w = args[1];

      app.parameters.push_back(w);

      if(app.parameters[0].length() > 2)
      {

        if(app.parameters[0][0] == L'\"')
          app.parameters[0].erase(app.parameters[0].begin());
        if(app.parameters[0].back() == L'\"')
          app.parameters[0].erase(app.parameters[0].begin() + app.parameters[0].length() - 1);

        boost::filesystem::path p(app.parameters[0]);

        if(boost::filesystem::is_regular_file(p))
          int a = 0;
      }
    }

    try
    {
      app.go();
    }
    catch(Ogre::Exception& e)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBox(NULL, std::wstring(e.getFullDescription().begin(), e.getFullDescription().end()).c_str(), L"An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
      std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
#endif
    }

    if(app.restart)
    {
      std::wstring param = L"\"" + CurrentAsset::get().lastAsset.wstring() + L"\"";
      ShellExecute(0, 0, p.wstring().c_str(), param.c_str(), p.parent_path().wstring().c_str(), SW_SHOW);
    }

    app.SaveSettings(true);
    CurrentScene::get().ClearScene();
    app.Destroy();

    return 0;
  }

}
