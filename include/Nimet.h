#pragma once

#include "ResourceWatcher.h"
#include "CurrentAsset.h"

class UI;

class DragDrop : public IDropTarget  
{
private:
  long m_nRefCount; 
public:

  HRESULT __stdcall QueryInterface(
    REFIID riid ,
    void **ppObj)
  {
    // tell other objects about our capabilities
    if (riid == IID_IUnknown || riid == IID_IDropTarget)
    {
      *ppObj = this;
      AddRef();
      return NOERROR;
    }
    *ppObj = NULL;
    return ResultFromScode(E_NOINTERFACE); 
  }

  ULONG __stdcall AddRef()
  {
    return InterlockedIncrement(&m_nRefCount) ;
  }

  ULONG __stdcall Release()
  {
    long nRefCount=0;
    nRefCount=InterlockedDecrement(&m_nRefCount) ;
    if (nRefCount == 0) delete this;
    return nRefCount;
  }

  HRESULT STDMETHODCALLTYPE DragEnter( 
    /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ __RPC__inout DWORD *pdwEffect)
  {
    return 0;
  }

  HRESULT STDMETHODCALLTYPE DragOver( 
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ __RPC__inout DWORD *pdwEffect)
  {
    return 0;
  }

  HRESULT STDMETHODCALLTYPE DragLeave( void)
  {   
    return 0;   
  }

  HRESULT STDMETHODCALLTYPE Drop( 
    /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ __RPC__inout DWORD *pdwEffect);

};

struct NimetSettings
{
  NimetSettings():
    frameLimiterStatus(false),
    framesPerSecond(60)
  {}


  bool frameLimiterStatus;
  int framesPerSecond;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & BOOST_SERIALIZATION_NVP(frameLimiterStatus);
    ar & BOOST_SERIALIZATION_NVP(framesPerSecond);
  }
};

class Nimet: public OIS::KeyListener, OIS::MouseListener, Ogre::WindowEventListener
{
private:

  Nimet(void);

public:


  static Nimet &get()
  {
    static Nimet instance;
    return instance;
  }

  virtual ~Nimet(void);
  bool go(void);

  void LoadSettings();
  void SaveSettings(bool onlyOptions = false);
  void FileDropped(const boost::filesystem::path &filePath);

  void SetFrameLimit(int limit, bool isCallback = false);
  void SetFrameLimiterStatus(bool status);
  void ToggleFrameLimiterStatus();

  std::vector<std::wstring> parameters;

  Ogre::SceneManager* mSceneMgr;

  Ogre::RenderWindow* mWindow;

  Ogre::Camera* mCamera;

  Ogre::Viewport *viewport;

  DragDrop *dragDrop;

  bool isActive;

  bool endProgram;

  NimetSettings settings;

  long renderTimeSlice;

#ifdef WIN32
  HWND hwnd;
#endif

  bool restart;

  int lastBatchCount;

  void Destroy();
protected:


  void ResourceModifiedActionCallback(ResourceWatcher::ResourceModificationType type, const boost::filesystem::path &path);
  void ShaderCompileFailed(const std::string &shaderName);
  void ResouceReloaded(ResourceWatcher::ReloadType type, void *ref);

  void SetupOIS();

  bool IsWindowActive();

  bool keyReleased(const OIS::KeyEvent &);
  bool keyPressed(const OIS::KeyEvent &);

  bool mouseMoved(const OIS::MouseEvent &arg);
  bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
  bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

  void windowResized(Ogre::RenderWindow* rw);
  void windowMoved(Ogre::RenderWindow* rw);

  ResourceWatcher *resourceWatcher;

  UI *ui;

  OIS::InputManager *inputManager;
  OIS::Keyboard *keyboard;
  OIS::Mouse *mouse;
  Ogre::OverlaySystem *overlaySystem;

  Ogre::Root *mRoot;
  Ogre::String mResourcesCfg;
  Ogre::String mPluginsCfg;
};

