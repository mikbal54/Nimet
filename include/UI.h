#pragma once

class CurrentAsset;
class LightInfo;

class UI : public HTMLonOverlay
{
protected:

  friend class Nimet;

  Ogre::Viewport *viewport;

  UI::UI() : isLoaded(false), viewport(0) { }

  static UI *instance;

  bool isLoaded;
public:

  static UI &get()
  {
    if(!instance)
      instance = new UI();

    return *instance;
  }

  static UI *getPtr()
  {
    if(!instance)
      instance = new UI();

    return instance;
  }

  void init(Ogre::Viewport *viewport, OIS::Keyboard *keyboard); 

  void onLoadHandler(Berkelium::Window *window);

  void onJavascriptCallback(const std::string &funcName, Berkelium::Script::Variant *args, size_t numArgs);

  void ClearCurrentAsset();
  void UpdateCurrentAsset();
  
  
  bool IsLoaded() { return isLoaded; }
  bool IsActive() 
  { 
    return window ? true : false;
  }

  void UI::SetAlwaysOnTop(bool val);
  
  void UI::SetGrid(bool val);
  
  bool UpdateBatchCount(int batchCount);

  void AddLight(int id, const LightInfo &info);
  
  void AddDirectory(const boost::filesystem::path &p);

  void UpdateDirectories();
  // EVENTS
  
  ////
  
  void LightPositionChanged(const std::string &elemId, const JSObject &obj);
  void LightDirectionChanged(const std::string &elemId, const JSObject &obj);
  void LightDiffuseColourChanged(const std::string &elemId, const JSObject &obj);
  void LightSpecularColourChanged(const std::string &elemId, const JSObject &obj);
  void LightInnerAngleChanged(const std::string &elemId, const JSObject &obj);
  void LightOuterAngleChanged(const std::string &elemId, const JSObject &obj);
  void LightFalloffAngleChanged(const std::string &elemId, const JSObject &obj);
  void LightRemove(const std::string elemId, const JSObject &obj);

  void AmbientChanged(const std::string elemId, const JSObject &obj);

  void LightAddButtonPressed(const std::string &elemId, const JSObject &obj);
  
  void RefreshButtonPressed(const std::string &elemId, const JSObject &obj);
  
  
  /** Camera Options */
  
  void CameraRefreshPressed(const std::string elemId, const JSObject &obj);
  void CameraTypeChanged(const std::string elemId, const JSObject &obj);
  
  void CameraSpeedChanged(const std::string elemId, const JSObject &obj);
  
  void CameraPivotSpeedXChanged(const std::string elemId, const JSObject &obj);
  void CameraPivotSpeedYChanged(const std::string elemId, const JSObject &obj);
  void CameraPivotSpeedZChanged(const std::string elemId, const JSObject &obj);
  
  //    
  void ExportPNG(const std::string &elemId, const JSObject &obj);
  
  /** Options */
  void FPSLimitChanged(const std::string &elemId, const JSObject &obj);
  void ShowAxesPressed(const std::string &elemId, const JSObject &obj);
  void BoneVisualsPressed(const std::string &elemId, const JSObject &obj);
  void ShowGridPressed(const std::string &elemId, const JSObject &obj);
  void ShowBoneNamesPressed(const std::string &elemId, const JSObject &obj);
  void AlwaysOnTopClicked(const std::string &elemId, const JSObject &obj);
  void BoneSizeChanged(const std::string &elemId, const JSObject &obj);
  void ShowNormalsPressed(const std::string &elemId, const JSObject &obj);
  void NormalsSizeChanged(const std::string &elemId, const JSObject &obj);
  void WireframeClicked(const std::string &elemId, const JSObject &obj);
  void BackgroundColourChanged(const std::string &elemId, const JSObject &obj);
  void OpenLogFileClicked(const std::string &elemId, const JSObject &obj);
  
  ~UI();
};
