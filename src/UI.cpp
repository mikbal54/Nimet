#include "PreCompile.h"
#include "UI.h"

#include "CurrentAsset.h"
#include "CurrentScene.h"
#include "Nimet.h"
#include "CameraController.h"
#include "Log.h"
#include "ResourceWatcher.h"

#include <string>

std::string NIMET_VERSION = "1.2.0";

UI *UI::instance = 0;

void UI::init(Ogre::Viewport *viewport, OIS::Keyboard *keyboard)
{
  this->viewport = viewport;

  HTMLonOverlayDescription desc;
  desc.name = "ogreAssistant";
  desc.posX = 0;
  desc.posY = 0;
  desc.textureWidth = viewport->getActualWidth();
  desc.textureHeight = viewport->getActualHeight();
  desc.overlayWidth = viewport->getActualWidth();
  desc.overlayHeight = viewport->getActualHeight();
  Init(desc);

  BrowseLocalHTML("../../Assets/UI/UI.html");
  HTMLManager::get().Add(this);
  SetTransparency(true);

  Focus();
  /// EVENTS
}

void UI::onLoadHandler(Berkelium::Window *window)
{
  UpdateCurrentAsset();

  Bind(L"cpp_PlayButtonPressed");
  Bind(L"cpp_AnimationDetailsPressed");
  Bind(L"cpp_OpenResource");
  Bind(L"cpp_OpenShader");
  Bind(L"cpp_DirectoryRemoveClicked");
  Bind(L"cpp_DirectoryRecursiveClicked");
  Bind(L"cpp_HardReset");

  jsHelper.AddCustomEvent("change", "AmbientColor", std::bind(&UI::AmbientChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnChangeCallback("PivotSpeedX", std::bind(&UI::CameraPivotSpeedXChanged, this, std::placeholders::_1, std::placeholders::_2));
  jsHelper.AddOnChangeCallback("PivotSpeedY", std::bind(&UI::CameraPivotSpeedYChanged, this, std::placeholders::_1, std::placeholders::_2));
  jsHelper.AddOnChangeCallback("PivotSpeedZ", std::bind(&UI::CameraPivotSpeedZChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnChangeCallback("CameraSpeed", std::bind(&UI::CameraSpeedChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddCustomEvent("change", "CameraTypeSelect", std::bind(&UI::CameraTypeChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddCustomEvent("click", "lightAdd", std::bind(&UI::LightAddButtonPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("CameraReset", std::bind(&UI::CameraRefreshPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("refreshButton", std::bind(&UI::RefreshButtonPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("ExportPNG", std::bind(&UI::ExportPNG, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("Axes", std::bind(&UI::ShowAxesPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("BoneVisuals", std::bind(&UI::BoneVisualsPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("BoneNames", std::bind(&UI::ShowBoneNamesPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnChangeCallback("BoneSize", std::bind(&UI::BoneSizeChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("GridToggle", std::bind(&UI::ShowGridPressed, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("Normals", std::bind(&UI::ShowNormalsPressed, this, std::placeholders::_1, std::placeholders::_2));
  jsHelper.AddOnChangeCallback("NormalsSize", std::bind(&UI::NormalsSizeChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("Wireframe", std::bind(&UI::WireframeClicked, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddCustomEvent("change", "BackgroundColour", std::bind(&UI::BackgroundColourChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("AlwaysOnTop", std::bind(&UI::AlwaysOnTopClicked, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("OpenLogFile", std::bind(&UI::OpenLogFileClicked, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddOnClickCallback("SaveSettings", [](const std::string&, const JSObject&)
  {
    Nimet::get().SaveSettings();
  });

  jsHelper.AddOnClickCallback("HardReset", [](const std::string&, const JSObject&)
  {
    Nimet::get().endProgram = true;
    Nimet::get().restart = true;
  });

  jsHelper.AddOnClickCallback("DeleteOgreCfg", [](const std::string&, const JSObject&)
  {
    if(!remove("ogre.cfg"))
      UI::get().ExecuteJavascript("ShowMessage('Removed Ogre.cfg!', 3000, 'green')");
    else
      UI::get().ExecuteJavascript("ShowMessage('Ogre.cfg already deleted!', 3000, 'black')");
  });

  jsHelper.AddOnClickCallback("FrameLimiter", [](const std::string&, const JSObject&)
  {
    Nimet::get().ToggleFrameLimiterStatus();
  });

  jsHelper.AddOnChangeCallback("FPSLimit", std::bind(&UI::FPSLimitChanged, this, std::placeholders::_1, std::placeholders::_2));


  // because onLoadHandler called after window.onload
  ExecuteJavascript("ElementModified('rightBar'); ElementModified('leftBar');");

  ExecuteJavascript("$('#version').text('"+ NIMET_VERSION +"')");

  CurrentScene::get().ApplySettings();

  CameraController::get().UpdateUI();
  CurrentScene::get().settings.UpdateUI();
  UpdateDirectories();

  Log::get().SetLogToUI();

  isLoaded = true;
}

void UI::ShowAxesPressed(const std::string& elemId, const JSObject& obj)
{
  CurrentScene::get().ToggleAxisArrows();
}

void UI::BoneVisualsPressed(const std::string& elemId, const JSObject& obj)
{
  bool r = CurrentAsset::get().ToggleBoneVisuals();

  if(r)
    ExecuteJavascript("SetBoneVisuals(true)");
  else
    ExecuteJavascript("SetBoneVisuals(false)");
}

void UI::SetGrid(bool val)
{
  if(val)
    ExecuteJavascript("SetGridVisible(true)");
  else
    ExecuteJavascript("SetGridVisible(false)");
}

void UI::ShowGridPressed(const std::string& elemId, const JSObject& obj)
{
  bool r = CurrentScene::get().ToggleGrid();

  SetGrid(r);
}

void UI::ShowBoneNamesPressed(const std::string& elemId, const JSObject& obj)
{
  bool r = CurrentAsset::get().ToggleBoneNames();

  if(r)
    ExecuteJavascript("SetBoneNames(true)");
  else
    ExecuteJavascript("SetBoneNames(false)");
}

void UI::RefreshButtonPressed(const std::string& elemId, const JSObject& obj)
{
  CurrentAsset::get().ForceReload();
}

void UI::ExportPNG(const std::string& elemId, const JSObject& obj)
{
  std::string s = Nimet::get().mWindow->writeContentsToTimestampedFile("", ".png");
  ExecuteJavascript("ShowMessage('Exported: " + s + "', 2000, 'black')");
}

bool UI::UpdateBatchCount(int batchCount)
{
  if(isLoaded)
  {
    std::stringstream ss;
    // -1 because we don't want UI to count as one

    if(CurrentAsset().get().assetType == CurrentAsset::Mesh)
      ss << "$('#BatchCount').text('" << batchCount - 1 << "')";
    else if(CurrentAsset().get().assetType == CurrentAsset::Scene)
      ss << "$('#SceneBatchCount').text('" << batchCount - 1 << "')";

    ExecuteJavascript(ss.str());

    return true;
  }

  return false;
}

void UI::CameraTypeChanged(const std::string elemId, const JSObject& obj)
{
  if(obj.value.str == L"Fixed")
    CameraController::get().ChangeCameraMode(CameraControllerSettings::Fixed);
  else if(obj.value.str == L"Pivot")
    CameraController::get().ChangeCameraMode(CameraControllerSettings::Pivot);
  else if(obj.value.str == L"Orbital")
    CameraController::get().ChangeCameraMode(CameraControllerSettings::Orbital);
  else if(obj.value.str == L"Free")
    CameraController::get().ChangeCameraMode(CameraControllerSettings::Free);
}

void UI::CameraRefreshPressed(const std::string elemId, const JSObject& obj)
{
  CameraController::get().ResetCameraOrientation();
}

void UI::CameraPivotSpeedXChanged(const std::string elemId, const JSObject& obj)
{
  float f = 0.0f;
  try
  {
    f = std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }
  CameraController::get().settings.orbitalSpeed.x = f;
  CameraController::get().UpdateUI();
}

void UI::CameraPivotSpeedYChanged(const std::string elemId, const JSObject& obj)
{
  float f = 0.0f;
  try
  {
    f = std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }
  CameraController::get().settings.orbitalSpeed.y = f;
  CameraController::get().UpdateUI();
}

void UI::CameraPivotSpeedZChanged(const std::string elemId, const JSObject& obj)
{
  float f = 0.0f;
  try
  {
    f = std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }
  CameraController::get().settings.orbitalSpeed.z = f;
  CameraController::get().UpdateUI();
}

void UI::CameraSpeedChanged(const std::string elemId, const JSObject& obj)
{
  float f = 0;
  try
  {
    f = std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CameraController::get().settings.cameraSpeed = f;

  std::wstringstream ss;
  int v = (int)(f * 100);

  if(v < 10)
    ss << "$('#CameraSpeedValue').text('00' + " << (int)(f * 100) << " + '%')";
  else if(v < 100)
    ss << "$('#CameraSpeedValue').text('0' + " << (int)(f * 100) << " + '%')";
  else
    ss << "$('#CameraSpeedValue').text(" << (int)(f * 100) << " + '%')";

  ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));

}

void UI::FPSLimitChanged(const std::string &elemId, const JSObject &obj)
{
  int f = 0;
  try
  {
    f = (int)std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  Nimet::get().SetFrameLimit(f, true);
}

void UI::BoneSizeChanged(const std::string& elemId, const JSObject& obj)
{
  float f = 0;
  try
  {
    f = std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().GetCurrentSceneSettings().boneSize = f;
  CurrentAsset::get().ApplyBoneSize();
}

void UI::ShowNormalsPressed(const std::string& elemId, const JSObject& obj)
{
  bool r = CurrentAsset::get().ToggleNormals();
  if(r)
    ExecuteJavascript(Berkelium::WideString::point_to(L"SetNormals(true)"));
  else
    ExecuteJavascript(Berkelium::WideString::point_to(L"SetNormals(false)"));
}

void UI::NormalsSizeChanged(const std::string& elemId, const JSObject& obj)
{
  float val = 1;
  try
  {
    val = std::stof(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().GetCurrentSceneSettings().normalsSize = val;
  if(CurrentAsset::get().normalsNode)
    CurrentAsset::get().ShowNormals(true);
}

void UI::WireframeClicked(const std::string& elemId, const JSObject& obj)
{
  if(Nimet::get().mCamera->getPolygonMode() == Ogre::PM_SOLID)
  {
    Nimet::get().mCamera->setPolygonMode(Ogre::PM_WIREFRAME);
    ExecuteJavascript(Berkelium::WideString::point_to(L"SetWireframeMode(true)"));
  }
  else
  {
    Nimet::get().mCamera->setPolygonMode(Ogre::PM_SOLID);
    ExecuteJavascript(Berkelium::WideString::point_to(L"SetWireframeMode(false)"));
  }
}

void UI::BackgroundColourChanged(const std::string& elemId, const JSObject& obj)
{
  Ogre::Viewport *viewport = Nimet::get().mCamera->getViewport();

  Ogre::ColourValue c;
  c.r = obj.GetChild(L"r")->GetFloat() / 255;
  c.g = obj.GetChild(L"g")->GetFloat() / 255;
  c.b = obj.GetChild(L"b")->GetFloat() / 255;

  CurrentScene::get().GetCurrentSceneSettings().backgroundColour = c;
  viewport->setBackgroundColour(c);
}

void UI::SetAlwaysOnTop(bool val)
{
  if(val)
    ExecuteJavascript(Berkelium::WideString::point_to(L"SetAlwaysOnTop(true)"));
  else
    ExecuteJavascript(Berkelium::WideString::point_to(L"SetAlwaysOnTop(false)"));
}

void UI::AlwaysOnTopClicked(const std::string& elemId, const JSObject& obj)
{
  bool r = CurrentScene::get().ToggleAlwaysOnTop();
  SetAlwaysOnTop(r);
  if(r)
    UI::get().ExecuteJavascript("ShowMessage('Now Always On Top!', 3000, 'green')");
  else
    UI::get().ExecuteJavascript("ShowMessage('Not On Top!', 3000, 'red')");
}

void UI::OpenLogFileClicked(const std::string& elemId, const JSObject& obj)
{
  ShellExecute(0, 0, L"Output.log", 0, 0, SW_SHOW);
}

void UI::AddDirectory(const boost::filesystem::path& p)
{
  std::wstring parent = p.parent_path().wstring();
  std::wstring leaf = p.leaf().wstring();
  std::wstring path = p.wstring();

  std::replace(path.begin(), path.end(), '\\', '/');
  std::replace(parent.begin(), parent.end(), '\\', '/');

  std::wstringstream ss;
  ss << "AddDirectory(";
  ss << "{";
  ss << "title:\"" << leaf << "\",";
  ss << "key:\"" << path << "\",";
  ss << "parent:\"" << parent << "\" } );";

  ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));

}

void UI::UpdateDirectories()
{
  std::wstringstream ss;
  ss << "ClearDirectories();";
  for(auto &it : ResourceWatcher::get().directories)
  {
    std::wstring path = it.first.wstring();

    // ignore default ones
    if(path == L"..\\..\\Assets\\Mesh")
      continue;
    if(path == L"..\\..\\Assets\\Material")
      continue;

    std::wstring parent = it.first.parent_path().wstring();
    std::wstring leaf = it.first.leaf().wstring();

    std::replace(path.begin(), path.end(), '\\', '/');
    std::replace(parent.begin(), parent.end(), '\\', '/');

    ss << "AddDirectory(";
    ss << "{";
    ss << "title:\"" << leaf << "\",";
    ss << "key:\"" << path << "\",";
    ss << "parent:\"" << parent << "\" } );";

  }

  ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));
}

UI::~UI()
{
  HTMLManager::get().Remove(this);
  window = 0;
}

void UI::UpdateCurrentAsset()
{
  if(!window)
    return;

  ClearCurrentAsset();

  CurrentAsset &currentAsset = CurrentAsset::get();
  if(currentAsset.assetType == CurrentAsset::Mesh)
  {

    if(!currentAsset.entity)
    {
      std::cout << "INTERNAL ERROR: type is mesh, but there is no entity loaded!\n";
      return;
    }

    std::wstringstream ss;

    size_t vertexCount = 0;
    for(size_t i = 0; i < currentAsset.vertexCount.size(); ++i)
      vertexCount += currentAsset.vertexCount[i];

    size_t polygonCount = 0;
    for(size_t i = 0; i < currentAsset.polygonCount.size(); ++i)
      polygonCount += currentAsset.polygonCount[i];

    // General
    ss << L"$('#MeshName').html('" << currentAsset.GetName() << L"');";
    ss << L"$('#Vertices').html('" << vertexCount << L"');";
    ss << L"$('#Polygons').html('" << polygonCount << L"');";

    if(currentAsset.entity->hasEdgeList())
      ss << L"$('#EdgeList').html('true');";
    else
      ss << L"$('#EdgeList').html('false');";
    //

    // Geometry
    ss << L"$('#Center').html('[" << currentAsset.center.x << L", " << currentAsset.center.y << L", " << currentAsset.center.z << L"]');";
    ss << L"$('#Width').html('" << currentAsset.max.x - currentAsset.min.x << L"');";
    ss << L"$('#Height').html('" << currentAsset.max.y - currentAsset.min.y << L"');";
    ss << L"$('#Depth').html('" << currentAsset.max.z - currentAsset.min.z << L"');";
    //

    // SubEntities
    size_t num = currentAsset.entity->getNumSubEntities();
    for(size_t i = 0; i < num; ++i)
    {
      Ogre::SubEntity *e = currentAsset.entity->getSubEntity(i);
      ss << L"AddSubEntity(new SubEntity(";
      ss << i << ",'";
      ss << Ogre::StringConverter::UTF8toUTF16(e->getMaterialName()) << L"',";
      ss << currentAsset.polygonCount[i] << L",";
      ss << currentAsset.vertexCount[i] << L",";
      ss << e->getSubMesh()->blendIndexToBoneIndexMap.size() << L",";
      if(e->getSubMesh()->useSharedVertices)
        ss << L"'true'));"; // bone count
      else
        ss << L"'false'));"; // bone count

      ss << L"AddMaterialToResources('" << Ogre::StringConverter::UTF8toUTF16(e->getMaterialName()) << L"');";

      Ogre::Material *mat = static_cast<Ogre::Material*>(Ogre::MaterialManager::getSingleton().getByName(e->getMaterialName()).get());

      for(int i = 0; i < mat->getNumTechniques(); ++i)
        for(int y = 0; y < mat->getTechnique(i)->getNumPasses(); ++y)
        {
          Ogre::Pass *p = mat->getTechnique(i)->getPass(y);
          if(p->hasVertexProgram())
            ss << L"AddVertexShaderToResources('" << Ogre::StringConverter::UTF8toUTF16(p->getVertexProgram()->getName()) << L"');";
          if(p->hasFragmentProgram())
            ss << L"AddFragmentShaderToResources('" << Ogre::StringConverter::UTF8toUTF16(p->getFragmentProgram()->getName()) << L"');";

          for(int tex = 0; tex < p->getNumTextureUnitStates(); ++tex)
            ss << L"AddTextureToResources('" << Ogre::StringConverter::UTF8toUTF16(p->getTextureUnitState(tex)->getTextureName()) << L"');";

        }

    }
    //

    // Animations
    if(currentAsset.entity->hasSkeleton())
    {
      Ogre::SkeletonInstance *skeleton = currentAsset.entity->getSkeleton();

      size_t numAnims = skeleton->getNumAnimations();
      for(size_t i = 0; i < numAnims; ++i)
      {
        Ogre::Animation *anim = skeleton->getAnimation(i);
        ss << "AddAnimation( new Animation('" << Ogre::StringConverter::UTF8toUTF16(anim->getName()) << "'," << anim->getLength() << ") );";
      }
    }

    ss << "SetLeftBarContents('Mesh');";

    ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));

  } // if type == Mesh
  else if( currentAsset.assetType == CurrentAsset::Scene )
  {
    std::wstringstream ss; 

    ss << "SetLeftBarContents('Scene');";

    ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));
  }

}

void UI::ClearCurrentAsset()
{
  ExecuteJavascript("ClearSubEntityPanel();ClearAnimationsPanel();ClearResources();");
}

void UI::onJavascriptCallback(const std::string &funcName, Berkelium::Script::Variant *args, size_t numArgs)
{
  if(funcName == "cpp_PlayButtonPressed")
  {
    if(numArgs == 1)
    {
      std::string s;
      HTML::WidestringToString(args[0].toString(), s);
      int r = CurrentAsset::get().ToggleAnimation(s);

      if(r == 1)
        ExecuteJavascript("SetPlayButtonStatus('" + s + "',true)");
      else if(r == 2)
        ExecuteJavascript("SetPlayButtonStatus('" + s + "',false)");
    }
  }
  else if(funcName == "cpp_AnimationDetailsPressed")
  {
    if(numArgs == 1)
    {
      std::string s;
      HTML::WidestringToString(args[0].toString(), s);
      CurrentAsset::get().ShowAnimationDetails(s);
    }
  }
  else if(funcName == "cpp_OpenResource")
  {
    if(numArgs == 1)
    {
      std::string s;
      HTML::WidestringToString(args[0].toString(), s);
      ResourceWatcher::get().OpenResource(s);
    }
  }
  else if(funcName == "cpp_OpenShader")
  {
    if(numArgs == 1)
    {
      std::string s;
      HTML::WidestringToString(args[0].toString(), s);
      ResourceWatcher::get().OpenShaderFile(s);
    }
  }
  else if(funcName == "cpp_DirectoryRemoveClicked")
  {
    if(numArgs == 1)
    {
      std::wstring w(args[0].toString().data()); // this removes trailing \0
      w.resize(args[0].toString().length());
      boost::filesystem::path p(w);
      ResourceWatcher::get().RemoveResourceDirectory(p, true);

      std::wstringstream ss;
      ss << "RemoveDirectory('";
      ss << w;
      ss << "');";

      ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));

    }
  }
  else if(funcName == "cpp_DirectoryRecursiveClicked")
  {
    if(numArgs == 1)
    {
      std::wstring w(args[0].toString().data()); // this removes trailing \0
      w.resize(args[0].toString().length());
      boost::filesystem::path p(w);

      bool r = ResourceWatcher::get().AddResourceDirectory(p, true);

      if(r)
        UpdateDirectories();
    }
  }
  else if(funcName == "cpp_HardReset")
  {
    Nimet::get().endProgram = true;
    Nimet::get().restart = true;
  }

}

void UI::LightAddButtonPressed(const std::string &elemId, const JSObject &obj)
{
  LightInfo info;
  int id = 0;
  if(obj.value.str == L"Point")
  {
    info.lightType = Ogre::Light::LT_POINT;
    id = CurrentScene::get().AddLight(info);
  }
  else if(obj.value.str == L"Directional")
  {
    info.lightType = Ogre::Light::LT_DIRECTIONAL;
    id = CurrentScene::get().AddLight(info);
  }
  else if(obj.value.str == L"Spot")
  {
    info.lightType = Ogre::Light::LT_SPOTLIGHT;
    id = CurrentScene::get().AddLight(info);
  }

  AddLight(id, info);
}

void UI::AddLight(int id, const LightInfo &info)
{
  std::stringstream ss;
  ss << "AddLight(new Light(";
  ss << id << ",";
  ss << "'" << info.GetType() << "',";
  ss << "new Vector3(" << info.position.x << "," << info.position.y << "," << info.position.z << "),";
  ss << "new Vector3(" << info.direction.x << "," << info.direction.y << "," << info.direction.z << "),";
  ss << "new Colour(" << info.diffuseColour.r << "," << info.diffuseColour.g << "," << info.diffuseColour.b << "," << info.diffuseColour.a << "),";
  ss << "new Colour(" << info.specularColour.r << "," << info.specularColour.g << "," << info.specularColour.b << "," << info.specularColour.a << "),";
  ss << info.innerAngle.valueDegrees() << ",";
  ss << info.outerAngle.valueDegrees() << ",";
  ss << info.fallOff << "))";
  ExecuteJavascript(ss.str());

  jsHelper.AddOnClickCallback(std::string("light_remove_") + std::to_string(id), std::bind(&UI::LightRemove, this, std::placeholders::_1, std::placeholders::_2));

  if(info.lightType != Ogre::Light::LT_DIRECTIONAL)
  {
    jsHelper.AddOnChangeCallback(std::string("light_position_x_") + std::to_string(id), std::bind(&UI::LightPositionChanged, this, std::placeholders::_1, std::placeholders::_2));
    jsHelper.AddOnChangeCallback(std::string("light_position_y_") + std::to_string(id), std::bind(&UI::LightPositionChanged, this, std::placeholders::_1, std::placeholders::_2));
    jsHelper.AddOnChangeCallback(std::string("light_position_z_") + std::to_string(id), std::bind(&UI::LightPositionChanged, this, std::placeholders::_1, std::placeholders::_2));
  }

  if(info.lightType != Ogre::Light::LT_POINT)
  {
    jsHelper.AddOnChangeCallback(std::string("light_direction_x_") + std::to_string(id), std::bind(&UI::LightDirectionChanged, this, std::placeholders::_1, std::placeholders::_2));
    jsHelper.AddOnChangeCallback(std::string("light_direction_y_") + std::to_string(id), std::bind(&UI::LightDirectionChanged, this, std::placeholders::_1, std::placeholders::_2));
    jsHelper.AddOnChangeCallback(std::string("light_direction_z_") + std::to_string(id), std::bind(&UI::LightDirectionChanged, this, std::placeholders::_1, std::placeholders::_2));
  }

  jsHelper.AddCustomEvent("change", std::string("light_diffuse_") + std::to_string(id), std::bind(&UI::LightDiffuseColourChanged, this, std::placeholders::_1, std::placeholders::_2));

  jsHelper.AddCustomEvent("change", std::string("light_specular_") + std::to_string(id), std::bind(&UI::LightSpecularColourChanged, this, std::placeholders::_1, std::placeholders::_2));

  if(info.lightType == Ogre::Light::LT_SPOTLIGHT)
  {
    jsHelper.AddOnChangeCallback(std::string("light_inner_") + std::to_string(id), std::bind(&UI::LightInnerAngleChanged, this, std::placeholders::_1, std::placeholders::_2));
    jsHelper.AddOnChangeCallback(std::string("light_outer_") + std::to_string(id), std::bind(&UI::LightOuterAngleChanged, this, std::placeholders::_1, std::placeholders::_2));
    jsHelper.AddOnChangeCallback(std::string("light_falloff_") + std::to_string(id), std::bind(&UI::LightFalloffAngleChanged, this, std::placeholders::_1, std::placeholders::_2));
  }
}

void UI::LightPositionChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 18)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(17, elemId.length()));

  float pos = 0;
  try
  {
    pos = boost::lexical_cast<float>(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().LightPositionChanged(id, elemId[15], pos);

}

void UI::LightDirectionChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 19)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(18, elemId.length()));

  float pos = 0;
  try
  {
    pos = boost::lexical_cast<float>(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().LightDirectionChanged(id, elemId[16], pos);
}

void UI::LightDiffuseColourChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 14)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(14, elemId.length()));

  Ogre::ColourValue c;
  c.r = obj.GetChild(L"r")->GetFloat() / 255;
  c.g = obj.GetChild(L"g")->GetFloat() / 255;
  c.b = obj.GetChild(L"b")->GetFloat() / 255;

  CurrentScene::get().LightDiffuseLightChanged(id, c);

}

void UI::LightSpecularColourChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 15)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(15, elemId.length()));

  Ogre::ColourValue c;
  c.r = obj.GetChild(L"r")->GetFloat() / 255;
  c.g = obj.GetChild(L"g")->GetFloat() / 255;
  c.b = obj.GetChild(L"b")->GetFloat() / 255;

  CurrentScene::get().LightSpecularLightChanged(id, c);

}

void UI::LightInnerAngleChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 12)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(12, elemId.length()));

  float pos = 0;
  try
  {
    pos = boost::lexical_cast<float>(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().LightInnerAngleChanged(id, pos);

}

void UI::LightOuterAngleChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 13)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(12, elemId.length()));

  float pos = 0;
  try
  {
    pos = boost::lexical_cast<float>(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().LightOuterAngleChanged(id, pos);
}

void UI::LightFalloffAngleChanged(const std::string &elemId, const JSObject &obj)
{
  if(elemId.size() < 14)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(14, elemId.length()));

  float pos = 0;
  try
  {
    pos = boost::lexical_cast<float>(obj.value.str);
  }
  catch(...)
  {
    return;
  }

  CurrentScene::get().LightFalloffChanged(id, pos);
}

void UI::LightRemove(const std::string elemId, const JSObject &obj)
{
  if(elemId.size() < 14)
    return;

  int id = boost::lexical_cast<int>(elemId.substr(13, elemId.length()));

  CurrentScene::get().RemoveLight(id);
  std::stringstream ss;
  ss << "RemoveLight(" << id << ")";
  ExecuteJavascript(ss.str());
}

void UI::AmbientChanged(const std::string elemId, const JSObject &obj)
{
  Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;

  Ogre::ColourValue c;
  c.r = obj.GetChild(L"r")->GetFloat() / 255;
  c.g = obj.GetChild(L"g")->GetFloat() / 255;
  c.b = obj.GetChild(L"b")->GetFloat() / 255;

  CurrentScene::get().GetCurrentSceneSettings().ambientColour = c;
  mgr->setAmbientLight(c);
}

