#pragma once

class CameraControllerSettings
{
public:

  CameraControllerSettings()
      : orbitalSpeed(0, 10, 0),
        cameraSpeed(1.0f),
        cameraMode(Pivot)
  {
  }

  Ogre::Vector3 orbitalSpeed;
  float cameraSpeed;

  enum CameraMode
  {
    Fixed,
    Pivot,
    Orbital,
    Free
  } cameraMode;

  std::wstring GetModeAsString()
  {
    switch(cameraMode)
    {
    case Fixed:
      return L"Fixed";
    case Pivot:
      return L"Pivot";
    case Orbital:
      return L"Orbital";
    case Free:
      return L"Free";
    default:
      return L"Fixed";
    }
  }

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & BOOST_SERIALIZATION_NVP(orbitalSpeed);
    ar & BOOST_SERIALIZATION_NVP(cameraSpeed);
    ar & BOOST_SERIALIZATION_NVP(cameraMode);
  }

};

class CameraController
{
private:

  friend class Nimet;
  friend class UI;

  CameraController()
      : camera(0),
        cameraNode(0),
        keyboard(0),
        mouse(0),
        timeSinceLastFrame(0),
        mouseMode(None)
  {
  }

  CameraControllerSettings settings;

  enum Mode
  {
    None,
    Rotating
  } mouseMode;

  OIS::Keyboard *keyboard;
  OIS::Mouse *mouse;

  float timeSinceLastFrame;

  Ogre::Camera *camera;
  Ogre::SceneNode *cameraNode;

public:

  static CameraController &get()
  {
    static CameraController instance;
    return instance;
  }

  void init(Ogre::Camera *camera, OIS::Keyboard *keyboard, OIS::Mouse *mouse);

  void SaveSettings();
  void LoadSettings();

  /** Refocuses on mesh */
  void ReFocusToMesh();

  void ResetCameraOrientation();

  void ChangeCameraMode(CameraControllerSettings::CameraMode mode);

  void ApplySettings();
  /* Send values to UI*/
  void UpdateUI();

  void MouseReleased(const OIS::MouseEvent &arg, int id);

  void MousePressed(const OIS::MouseEvent &arg, int id);

  void MouseMoved(const OIS::MouseEvent& arg);

  void Update(float t);

};
