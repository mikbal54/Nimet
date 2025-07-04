#pragma once

class LightInfo
{
public:

  LightInfo()
    : position(10, 0, 0),
    direction(-1, 0, 0),
    diffuseColour(1, 1, 1, 1),
    specularColour(0, 0, 0, 1),
    lightType(Ogre::Light::LT_POINT),
    innerAngle(45),
    outerAngle(90),
    fallOff(1)
  {

  }

  Ogre::Light::LightTypes lightType;

  Ogre::Vector3 position;
  Ogre::Vector3 direction;
  Ogre::Degree innerAngle;
  Ogre::Degree outerAngle;
  Ogre::Real fallOff;

  Ogre::ColourValue diffuseColour;
  Ogre::ColourValue specularColour;

  ~LightInfo()
  {
  }

  std::string GetType() const
  {

    switch(lightType)
    {
    case Ogre::Light::LT_POINT:
      return "Point";
    case Ogre::Light::LT_DIRECTIONAL:
      return "Directional";
    case Ogre::Light::LT_SPOTLIGHT:
      return "Spot";
    }

    return "";
  }

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & BOOST_SERIALIZATION_NVP(lightType);
    ar & BOOST_SERIALIZATION_NVP(position);
    ar & BOOST_SERIALIZATION_NVP(direction);
    ar & BOOST_SERIALIZATION_NVP(innerAngle);
    ar & BOOST_SERIALIZATION_NVP(outerAngle);
    ar & BOOST_SERIALIZATION_NVP(fallOff);
    ar & BOOST_SERIALIZATION_NVP(diffuseColour);
    ar & BOOST_SERIALIZATION_NVP(specularColour);
  }

};

class SceneLight
{
public:

  SceneLight()
    : light(0),
    id(0),
    node(0),
    billboard(0),
    spotlightVisual(0)
  {
  }

  ~SceneLight();

  int id;

  Ogre::Light *light;
  Ogre::SceneNode *node;

  Ogre::BillboardSet *billboard;

  Ogre::ManualObject* spotlightVisual;

  void SetDirection(Ogre::Vector3 &dir);

};

class AxisArrows
{
public:

  AxisArrows();

  ~AxisArrows();

  Ogre::SceneNode *node;

};

class Grid
{
public:

  Grid();

  ~Grid();

  Ogre::SceneNode *node;

};

class ResourceDirectories
{
public:

  // Utf8
  std::set<std::string> directories;
  std::set<std::string> cfgFiles;
  
  void PopulateDirectories();
  void AddDirectortiesToQueue();
  
  bool LoadResourceCfgFile(const boost::filesystem::path &filePath);

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & BOOST_SERIALIZATION_NVP(directories);
    ar & BOOST_SERIALIZATION_NVP(cfgFiles);
  }

};

class CurrentSceneSettings
{
public:

  CurrentSceneSettings()
    : ambientColour(0.3f, 0.3f, 0.3f, 1),
    backgroundColour(0.1f, 0.1f, 0.1f, 1),
    boneSize(1),
    normalsSize(1),
    isOnTop(false),
    grid(false),
    axes(false)
  {
  }

  std::vector<LightInfo> lights;
  Ogre::ColourValue ambientColour;
  Ogre::ColourValue backgroundColour;
  float boneSize;
  float normalsSize;

  bool isOnTop;
  bool grid;
  bool axes;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & BOOST_SERIALIZATION_NVP(ambientColour);
    ar & BOOST_SERIALIZATION_NVP(boneSize);
    ar & BOOST_SERIALIZATION_NVP(normalsSize);
    ar & BOOST_SERIALIZATION_NVP(lights);
    ar & BOOST_SERIALIZATION_NVP(isOnTop);
    ar & BOOST_SERIALIZATION_NVP(grid);
    ar & BOOST_SERIALIZATION_NVP(axes);
  }

  void PopulateLights(std::vector<SceneLight*> &sceneLights);
  void UpdateUI();

};

class CurrentScene
{
private:

  friend class Nimet;
  friend class UI;

  CurrentScene()
    : lightIdCount(0),
    axisArrows(0),
    grid(0)
  {
  }

  int lightIdCount;

  std::vector<SceneLight*> lights;

  CurrentSceneSettings settings;
  

  SceneLight *GetLight(int id);

  AxisArrows *axisArrows;
  Grid *grid;

public:
  
  ResourceDirectories resourceDirectories;

  static CurrentScene &get()
  {
    static CurrentScene instance;
    return instance;
  }

  static void destroyAllAttachedMovableObjects(Ogre::SceneNode* node);
  /** Destroy sceneNode, all its children and attached*/
  static void destroySceneNode(Ogre::SceneNode* node);

  CurrentSceneSettings &GetCurrentSceneSettings()
  {
    return settings;
  }

  void SaveSettings();
  void SaveResourceSettings();
  void LoadSettings();
  void LoadResourceSettings();
  
  void ApplySettings();

  void SetAlwaysOnTop(bool val);
  bool ToggleAlwaysOnTop();

  /** Clears Scene.*/
  void ClearScene();

  int AddLight(const LightInfo &info);
  void RemoveLight(int id);

  void ToggleAxisArrows();
  void SetAxisArrowsVisible(bool isShown);

  void CurrentScene::SetGrid(bool val);
  bool ToggleGrid();

  void LightPositionChanged(int id, char c, float newPos);
  void LightDirectionChanged(int id, char c, float newDir);
  void LightDiffuseLightChanged(int id, const Ogre::ColourValue &newColour);
  void LightSpecularLightChanged(int id, const Ogre::ColourValue &newColour);
  void LightInnerAngleChanged(int id, float newAngle);
  void LightOuterAngleChanged(int id, float newAngle);
  void LightFalloffChanged(int id, float newAngle);

};
