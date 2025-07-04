#include "PreCompile.h"
#include "CurrentScene.h"

#include "Nimet.h"
#include "UI.h"

void CurrentScene::destroyAllAttachedMovableObjects(Ogre::SceneNode* node)
{
  // Destroy all the attached objects
  Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

  while(itObject.hasMoreElements())
    node->getCreator()->destroyMovableObject(itObject.getNext());

  // Recurse to child SceneNodes
  Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

  while(itChild.hasMoreElements())
  {
    Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
    destroyAllAttachedMovableObjects(pChildNode);
  }
}

void CurrentScene::destroySceneNode(Ogre::SceneNode* node)
{
  destroyAllAttachedMovableObjects(node);
  node->removeAndDestroyAllChildren();
  node->getCreator()->destroySceneNode(node);
}

int CurrentScene::AddLight(const LightInfo &info)
{
  SceneLight *l = new SceneLight();
  Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
  l->light = mgr->createLight();
  l->node = mgr->getRootSceneNode()->createChildSceneNode();

  float radius = CurrentAsset::get().GetBoundingRadius();
  if(radius > 5.0f)
    radius = 5.0f;

  if(info.lightType != Ogre::Light::LT_DIRECTIONAL)
  {

    l->billboard = mgr->createBillboardSet();
    Ogre::Billboard* myBillboard = l->billboard->createBillboard(Ogre::Vector3(0, 0, 0));
    myBillboard->setDimensions(radius * 0.3f, radius * 0.3f);
    l->billboard->setMaterialName("Sun");
    l->node->attachObject(l->billboard);
  }

  if(info.lightType == Ogre::Light::LT_SPOTLIGHT)
  {
    l->spotlightVisual = mgr->createManualObject();

    l->spotlightVisual->begin("spot", Ogre::RenderOperation::OT_LINE_STRIP);
    l->spotlightVisual->position(0, 0, 0);
    l->spotlightVisual->position(info.direction.x * radius, info.direction.y * radius, info.direction.z * radius);
    l->spotlightVisual->end();

    l->node->attachObject(l->spotlightVisual);
  }

  l->node->attachObject(l->light);
  l->node->setPosition(info.position);

  l->light->setDirection(info.direction);
  l->light->setType(info.lightType);
  l->light->setDiffuseColour(info.diffuseColour);
  l->light->setSpecularColour(info.specularColour);
  l->light->setSpotlightFalloff(info.fallOff);
  l->light->setSpotlightInnerAngle(info.innerAngle);
  l->light->setSpotlightOuterAngle(info.outerAngle);

  l->id = ++lightIdCount;
  lights.push_back(l);

  return lightIdCount;
}

void CurrentScene::ClearScene()
{

  for(auto i : lights)
  {
    delete i;
  }

  if(axisArrows)
  {
    delete axisArrows;
    axisArrows = 0;
  }

  if(grid)
  {
    delete grid;
    grid = 0;
  }

}

SceneLight * CurrentScene::GetLight(int id)
{
  for(auto i : lights)
  {
    if(i->id == id)
      return i;
  }

  return 0;
}

void CurrentScene::LightPositionChanged(int id, char c, float newPos)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  Ogre::Vector3 pos = light->node->getPosition();

  std::stringstream ss;

  switch(c)
  {
  case 'x':
    pos.x = newPos;
    ss << "$('#light_position_x_" << id << "').val('" << newPos << "')";
    break;
  case 'y':
    pos.y = newPos;
    ss << "$('#light_position_y_" << id << "').val('" << newPos << "')";
    break;
  case 'z':
    pos.z = newPos;
    ss << "$('#light_position_z_" << id << "').val('" << newPos << "')";
    break;
  }

  light->node->setPosition(pos);
  UI::get().ExecuteJavascript(ss.str());
}

void CurrentScene::LightDirectionChanged(int id, char c, float newDir)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  Ogre::Vector3 dir = light->light->getDirection();

  std::stringstream ss;

  switch(c)
  {
  case 'x':
    dir.x = newDir;
    ss << "$('#light_direction_x_" << id << "').val('" << newDir << "')";
    break;
  case 'y':
    dir.y = newDir;
    ss << "$('#light_direction_y_" << id << "').val('" << newDir << "')";
    break;
  case 'z':
    dir.z = newDir;
    ss << "$('#light_direction_z_" << id << "').val('" << newDir << "')";
    break;
  }

  light->SetDirection(dir);

  UI::get().ExecuteJavascript(ss.str());
}

void CurrentScene::LightDiffuseLightChanged(int id, const Ogre::ColourValue &newColour)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  light->light->setDiffuseColour(newColour);
}

void CurrentScene::LightSpecularLightChanged(int id, const Ogre::ColourValue &newColour)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  light->light->setSpecularColour(newColour);
}

void CurrentScene::LightInnerAngleChanged(int id, float newAngle)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  std::stringstream ss;

  ss << "$('#light_inner_" << id << "').val('" << newAngle << "')";

  light->light->setSpotlightInnerAngle(Ogre::Degree(newAngle));

  UI::get().ExecuteJavascript(ss.str());
}

void CurrentScene::LightOuterAngleChanged(int id, float newAngle)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  std::stringstream ss;

  ss << "$('#light_outer_" << id << "').val('" << newAngle << "')";

  light->light->setSpotlightOuterAngle(Ogre::Degree(newAngle));

  UI::get().ExecuteJavascript(ss.str());
}

void CurrentScene::SetAxisArrowsVisible(bool isShown)
{
  if(isShown)
  {
    if(axisArrows)
      delete axisArrows;

    axisArrows = new AxisArrows();
    settings.axes = true;
  }
  else
  {
    if(axisArrows)
    {
      delete axisArrows;
      axisArrows = 0;
      settings.axes = false;
    }
  }
}

void CurrentScene::ToggleAxisArrows()
{
  if(axisArrows)
  {
    SetAxisArrowsVisible(false);
    UI::get().ExecuteJavascript("SetAxesVisible(false)");
  }
  else
  {
    SetAxisArrowsVisible(true);
    UI::get().ExecuteJavascript("SetAxesVisible(true)");
  }
}

void CurrentScene::SetGrid(bool val)
{
  if(val)
  {
    if(grid)
      return;

    grid = new Grid();
    settings.grid = true;
  }
  else
  {
    if(grid)
    {
      delete grid;
      grid = nullptr;
      settings.grid = false;
    }
  }
}

bool CurrentScene::ToggleGrid()
{
  if(grid)
  {
    SetGrid(false);
    return false;
  }
  else
  {
    SetGrid(true);
    return true;
  }
}

void CurrentScene::SetAlwaysOnTop(bool val)
{
  if(val)
  {
    SetWindowPos(Nimet::get().hwnd,       // handle to window
    HWND_TOPMOST,  // placement-order handle
    0,     // horizontal position
    0,      // vertical position
    0,  // width
    0, // height
    SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE // window-positioning options
    );
  }
  else
  {
    SetWindowPos(Nimet::get().hwnd,       // handle to window
    HWND_NOTOPMOST,  // placement-order handle
    0,     // horizontal position
    0,      // vertical position
    0,  // width
    0, // height
    SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE // window-positioning options
    );
  }

  settings.isOnTop = val;
}

bool CurrentScene::ToggleAlwaysOnTop()
{
  if(settings.isOnTop)
  {
    SetAlwaysOnTop(false);
  }
  else
  {
    SetAlwaysOnTop(true);
  }

  return settings.isOnTop;
}

void CurrentScene::SaveResourceSettings()
{
  std::ofstream ofs("../../Settings/ResourceDirectories.xml");

  if(!ofs.is_open())
  {
    std::cout << "Can not save ../../Settings/ResourceDirectories.xml\n";
    return;
  }

  resourceDirectories.PopulateDirectories();

  try
  {
    boost::archive::xml_oarchive ia(ofs);
    ia << boost::serialization::make_nvp("resourceDirectories", resourceDirectories);
  }
  catch(...)
  {
    std::cout << "Invalid xml file. ../../Settings/ResourceDirectories.xml\n";
  }
}

void CurrentScene::SaveSettings()
{
  settings.PopulateLights(lights);

  std::ofstream ofs("../../Settings/SceneSettings.xml");

  if(!ofs.is_open())
  {
    std::cout << "Can not save ../../Settings/SceneSettings.xml\n";
    return;
  }

  boost::archive::xml_oarchive ia(ofs);

  try
  {
    ia << boost::serialization::make_nvp("settings", settings);
    ia << boost::serialization::make_nvp("nimetSettings", Nimet::get().settings);
  }
  catch(...)
  {
    std::cout << "Invalid xml file. ../../Settings/SceneSettings.xml\n";
  }
}

void CurrentScene::LoadResourceSettings()
{
  std::ifstream ofs("../../Settings/ResourceDirectories.xml");

  if(!ofs.is_open())
  {
    std::cout << "Can not open ../../Settings/ResourceDirectories.xml\n";
    return;
  }

  try
  {
    boost::archive::xml_iarchive ia(ofs);
    ia >> BOOST_SERIALIZATION_NVP(resourceDirectories);
  }
  catch(...)
  {
    std::cout << "Invalid xml file ../../Settings/ResourceDirectories.xml\n";
    return;
  }

}

void CurrentScene::LoadSettings()
{
  std::ifstream ofs("../../Settings/SceneSettings.xml");

  if(!ofs.is_open())
  {
    std::cout << "Can not open ../../Settings/SceneSettings.xml\n";
    return;
  }

  boost::archive::xml_iarchive ia(ofs);

  try
  {
    ia >> boost::serialization::make_nvp("settings", settings);
    ia >> boost::serialization::make_nvp("nimetSettings", Nimet::get().settings);
  }
  catch(...)
  {
    std::cout << "Invalid xml file. ../../Settings/SceneSettings.xml\n";
    return;
  }

}

void CurrentScene::ApplySettings()
{
  ClearScene();

  Nimet::get().mSceneMgr->setAmbientLight(settings.ambientColour);

  Nimet::get().viewport->setBackgroundColour(settings.backgroundColour);

  SetGrid(settings.grid);
  SetAxisArrowsVisible(settings.axes);

  for(auto &i : settings.lights)
  {
    int id = AddLight(i);
    UI::get().AddLight(id, i);
  }

}

void CurrentScene::LightFalloffChanged(int id, float newAngle)
{
  SceneLight *light = GetLight(id);

  if(!light)
  {
    std::cout << "ERROR: Light with id " << id << " doesnt exist.\n";
    return;
  }

  std::stringstream ss;

  ss << "$('#light_falloff_" << id << "').val('" << newAngle << "')";

  light->light->setSpotlightFalloff(newAngle);

  UI::get().ExecuteJavascript(ss.str());
}

void CurrentScene::RemoveLight(int id)
{
  for(size_t i = 0; i < lights.size(); ++i)
  {
    if(lights[i]->id == id)
    {
      delete lights[i];
      lights.erase(i + lights.begin());
    }
  }

}

SceneLight::~SceneLight()
{
  if(node)
  {
    Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;

    light->detachFromParent();
    mgr->destroyLight(light);

    if(billboard)
    {
      billboard->detachFromParent();
      mgr->destroyBillboardSet(billboard);
    }

    if(spotlightVisual)
    {
      spotlightVisual->detachFromParent();
      mgr->destroyManualObject(spotlightVisual);
    }

    mgr->destroySceneNode(node);
  }
}

void SceneLight::SetDirection(Ogre::Vector3 &dir)
{
  dir.normalise();

  if(spotlightVisual)
  {
    Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
    spotlightVisual->detachFromParent();
    mgr->destroyManualObject(spotlightVisual);

    spotlightVisual = mgr->createManualObject();

    spotlightVisual->begin("BaseWhite", Ogre::RenderOperation::OT_LINE_STRIP);
    spotlightVisual->position(0, 0, 0);
    spotlightVisual->position(dir.x * CurrentAsset::get().GetBoundingRadius(), dir.y * CurrentAsset::get().GetBoundingRadius(), dir.z * CurrentAsset::get().GetBoundingRadius());
    spotlightVisual->end();

    node->attachObject(spotlightVisual);
  }

  light->setDirection(dir);
}

AxisArrows::AxisArrows()
{
  Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
  node = mgr->getRootSceneNode()->createChildSceneNode();

  Ogre::Entity *blueArrow = mgr->createEntity("Arrow.mesh");
  blueArrow->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE - 2);
  blueArrow->setMaterialName("BlueArrow");
  node->attachObject(blueArrow);

  Ogre::Entity *redArrow = mgr->createEntity("Arrow.mesh");
  redArrow->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE - 2);
  redArrow->setMaterialName("RedArrow");
  Ogre::SceneNode *redNode = node->createChildSceneNode();
  redNode->attachObject(redArrow);
  redNode->rotate(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y), Ogre::Node::TS_WORLD);

  Ogre::Entity *greenArrow = mgr->createEntity("Arrow.mesh");
  greenArrow->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE - 2);
  greenArrow->setMaterialName("GreenArrow");
  Ogre::SceneNode *greenNode = node->createChildSceneNode();
  greenNode->attachObject(greenArrow);
  greenNode->rotate(Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X), Ogre::Node::TS_WORLD);

}

AxisArrows::~AxisArrows()
{
  if(node)
  {
    CurrentScene::destroySceneNode(node);
  }
}

Grid::Grid()
{
  Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;

  Ogre::MeshManager::getSingleton().createPlane("GridMesh", "General", Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 1000, 1000, 1, 1, false, 1, 1000, 1000, Ogre::Vector3::UNIT_Z);
  Ogre::Entity *e = mgr->createEntity("GridMesh");
  e->setMaterialName("grid");
  node = mgr->getRootSceneNode()->createChildSceneNode();
  node->attachObject(e);
}

Grid::~Grid()
{
  if(node)
    CurrentScene::destroySceneNode(node);
}

void CurrentSceneSettings::PopulateLights(std::vector<SceneLight*>& sceneLights)
{
  lights.clear();

  for(auto i : sceneLights)
  {
    LightInfo info;
    info.diffuseColour = i->light->getDiffuseColour();
    info.specularColour = i->light->getSpecularColour();
    info.fallOff = i->light->getSpotlightFalloff();
    info.innerAngle = i->light->getSpotlightInnerAngle();
    info.outerAngle = i->light->getSpotlightOuterAngle();
    info.lightType = i->light->getType();
    info.direction = i->light->getDirection();
    if(i->node)
      info.position = i->node->_getDerivedPosition();
    lights.push_back(info);
  }
}

void CurrentSceneSettings::UpdateUI()
{
  UI::get().SetGrid(grid);
  UI::get().SetAlwaysOnTop(isOnTop);

  std::wstringstream ss;

  if(axes)
    ss << "SetAxesVisible(true);";
  else
    ss << "SetAxesVisible(false);";

  ss << L"$('#AmbientColor').val('rgb(" << static_cast<int>(ambientColour.r * 256) << L", " << static_cast<int>(ambientColour.g * 256) << L", " << static_cast<int>(ambientColour.b * 256) << L")');";

  ss << L"$('#BoneSize').val(" << boneSize << L");";
  ss << L"$('#NormalsSize').val(" << normalsSize << L");";

  ss << "$('#BackgroundColour').val('rgb(" << backgroundColour.r << ", " << backgroundColour.g << ", " << backgroundColour.b << ")')";

  UI::get().ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));
}

void ResourceDirectories::PopulateDirectories()
{
  directories.clear();
  auto &dirs = ResourceWatcher::get().directories;

  for(auto &i : dirs)
  {
    std::wstring w = i.first.wstring();
    for(size_t i = 0; i < w.size(); ++i)
    {
      if(w[i] == '\\')
        w[i] = '/';
    }
    directories.insert(Ogre::StringConverter::UTF16toUTF8(w));
  }
}

void ResourceDirectories::AddDirectortiesToQueue()
{
  ResourceWatcher &watcher = ResourceWatcher::get();

  // first add directories from .cfg 
  for(auto &i : cfgFiles)
    LoadResourceCfgFile(boost::filesystem::path(i));
  
  // add all directories to Queue
  for(auto &i : directories)
    watcher.AddResourceDirectoryToQueue(boost::filesystem::path(Ogre::StringConverter::UTF8toUTF16(i)), false);

}

bool ResourceDirectories::LoadResourceCfgFile(const boost::filesystem::path& filePath)
{
  using namespace boost::filesystem;

  {
    std::wstring wpath = filePath.wstring();
    for(size_t i = 0; i < wpath.size(); ++i)
      if(wpath[i] == '\\')
        wpath[i] = '/';

    cfgFiles.insert(Ogre::StringConverter::UTF16toUTF8(wpath));
  }

  bool addedADirectory = false;
  boost::filesystem::ifstream ifs(filePath);

  if(!ifs.is_open())
  {
    std::cout << "Could not open " << filePath.string() << "\n";
    return false;
  }

  std::string s;
  while(getline(ifs, s, '\n'))
  {
    auto tokens = Ogre::StringUtil::tokenise(s, "=", "", 0);
    if(tokens.empty())
      continue;

    Ogre::StringUtil::trim(tokens[0]);
    Ogre::StringUtil::toLowerCase(tokens[0]);
    if(tokens[0] == "filesystem") // TODO: add zip handling
    {

      if(tokens.size() < 2)
        continue;

      boost::filesystem::path dir(Ogre::StringConverter::UTF8toUTF16(tokens[1]));

      if(!dir.is_absolute())
        dir = filePath.parent_path() / dir;

      if(!is_directory(dir))
        continue;

      ResourceWatcher::get().AddResourceDirectoryToQueue(dir, false);
      addedADirectory = true;
    }

  }

  return addedADirectory;
}
