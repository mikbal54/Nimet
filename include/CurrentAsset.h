#pragma once

class ResourceWatcher;

class CurrentAsset
{
private:

  CurrentAsset()
      : node(0),
        normalsNode(0),
        entity(0),
        sceneManager(0),
        polygonCount(0),
        assetType(None),
        asset_count(0)
  {

  }

  class BoneVisuals
  {
  public:

    class BoneLinkVisual
    {
    public:

      BoneLinkVisual()
          : tagPoint(0),
            entity(0)
      {
      }

      Ogre::TagPoint *tagPoint;
      Ogre::Entity *entity;

      ~BoneLinkVisual();
    };

    BoneVisuals()
        : tagPoint(0),
          bone(0),
          entity(0)
    {
    }

    Ogre::Bone *bone;
    Ogre::TagPoint *tagPoint;
    Ogre::Entity *entity;

    std::list<BoneLinkVisual> toChildren;

    ~BoneVisuals();
  };

  class BoneName
  {
  public:

    BoneName()
        : text(0),
          tagPoint(0)
    {
    }

    ~BoneName();

    void init(Ogre::Bone *bone);

    Ogre::MovableText *text;
    Ogre::TagPoint *tagPoint;

  };

protected:

  friend class Nimet;
  friend class UI;
  friend class CameraController;

  enum AssetType
  {
    None,
    Mesh,
    Scene
  } assetType;

  boost::unordered_map<std::string, BoneVisuals> bonesEntities;
  boost::unordered_map<std::string, BoneName> boneNames;

  std::vector<size_t> polygonCount;
  std::vector<size_t> vertexCount;

  std::vector<Ogre::AnimationState*> animationStates;

  
  Ogre::Vector3 min;
  Ogre::Vector3 max;
  Ogre::Vector3 center;

  Ogre::SceneManager *sceneManager;

  Ogre::SceneNode *node;
  Ogre::SceneNode *normalsNode;

  // counts assets used
  int asset_count;
  
  void CalculateMeshStats();

  void ShowBones(bool isShown);

  void ShowBoneLinks(BoneVisuals &boneVisuals);

  void ShowBoneNames(bool isShown);

  void getMeshInformation(Ogre::Mesh* mesh, size_t &vertex_count, Ogre::Vector3* &vertices, size_t &index_count, unsigned long* &indices, const Ogre::Vector3 &position = Ogre::Vector3::ZERO,
      const Ogre::Quaternion &orient = Ogre::Quaternion::IDENTITY, const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE);

public:

  static CurrentAsset &get()
  {
    static CurrentAsset instance;
    return instance;
  }
  
  boost::filesystem::path lastAsset;

  Ogre::Entity *entity;
  
  void ForceReload();

  static void GetNumberOfLights(Ogre::SceneNode *node, int &count);

  static void GetAllChildEntities(Ogre::SceneNode *node, std::list<Ogre::Entity*> &children );

  static int GetSubMeshPolygonCount(Ogre::SubMesh *mesh);

  static int GetEntityPolygonCount(Ogre::Entity *entity);

  /** Called when program ends. */
  void Destroy();

  void DestroyCurrent();
  
  void EntityReloaded(Ogre::Entity *oldEntity, Ogre::Entity *newEntity)
  {
    if(oldEntity == entity)
      entity = newEntity;
  }

  void ResetOrientation();

  void ApplyBoneSize();

  bool OpenMesh(const boost::filesystem::path &pathToMesh);

  bool OpenScene(const boost::filesystem::path &pathToMesh);

  void ShowAnimationDetails(const std::string &name);

  /** Toggles animation. Returns 1 if started animation, 2 if stopped animation. -1 if did nothing */
  int ToggleAnimation(const std::string &name);

  bool ToggleBoneVisuals();

  bool ToggleBoneNames();
  
  bool ToggleNormals();
  void ShowNormals(bool isShown);

  /** Call to update current animations */
  void Update(float timeSinceLastFrame);

  std::wstring GetName();

  float GetBoundingRadius();

};
