#include "PreCompile.h"
#include "CameraController.h"

#include "Nimet.h"
#include "CurrentAsset.h"
#include "UI.h"

void CameraController::ChangeCameraMode(CameraControllerSettings::CameraMode mode)
{
  if(settings.cameraMode == mode)
    return;

  if(mode == CameraControllerSettings::Pivot)
    ReFocusToMesh();

  settings.cameraMode = mode;

}

void CameraController::UpdateUI()
{
  std::wstringstream ss;

  ss << "$('#PivotSpeedX').val('" << settings.orbitalSpeed.x << "');";
  ss << "$('#PivotSpeedY').val('" << settings.orbitalSpeed.y << "');";
  ss << "$('#PivotSpeedZ').val('" << settings.orbitalSpeed.z << "');";


  ss << "SetSelectBox('CameraTypeSelect', '" << settings.GetModeAsString() << "');";

  ss << "$('#CameraSpeed').val('" << settings.cameraSpeed << "');";
  int v = (int)(settings.cameraSpeed * 100);
  if(v < 10)
    ss << "$('#CameraSpeedValue').text('00' + " << v << " + '%')";
  else if(v < 100)
    ss << "$('#CameraSpeedValue').text('0' + " << v << " + '%')";
  else
    ss << "$('#CameraSpeedValue').text(" << v << " + '%')";

  UI::get().ExecuteJavascript(Berkelium::WideString::point_to(ss.str()));
}

void CameraController::init(Ogre::Camera *camera, OIS::Keyboard *keyboard, OIS::Mouse *mouse)
{
  this->camera = camera;

  cameraNode = Nimet::get().mSceneMgr->getRootSceneNode()->createChildSceneNode();
  cameraNode->setPosition(0, 0, 0);
  cameraNode->attachObject(camera);
  cameraNode->setFixedYawAxis(true);

  this->mouse = mouse;
  this->keyboard = keyboard;

  camera->setPosition(Ogre::Vector3(0, 0, 30));
  camera->setNearClipDistance(0.2f);
  camera->lookAt(0, 0, 0);
}

void CameraController::ResetCameraOrientation()
{
  if(settings.cameraMode == CameraControllerSettings::Fixed)
  {
    CurrentAsset::get().ResetOrientation();
  }
  else if(settings.cameraMode == CameraControllerSettings::Pivot || settings.cameraMode == CameraControllerSettings::Orbital)
  {
    ReFocusToMesh();
  }
  else if(settings.cameraMode == CameraControllerSettings::Free)
  {
    float radius = CurrentAsset::get().GetBoundingRadius();

    if(radius < 1)
      camera->setPosition(Ogre::Vector3(0, 0, 1));
    else
      camera->setPosition(Ogre::Vector3(0, 0, radius * 2));

    cameraNode->resetOrientation();
    camera->lookAt(Ogre::Vector3::ZERO);
  }

}

void CameraController::MousePressed(const OIS::MouseEvent &arg, int id)
{
  if(id == OIS::MouseButtonID::MB_Left)
  {
    if(!UI::get().IsOverAnElement(arg.state.X.abs, arg.state.Y.abs))
    {
      mouseMode = Rotating;
    }
  }
}

void CameraController::MouseReleased(const OIS::MouseEvent &arg, int id)
{
  if(id == OIS::MouseButtonID::MB_Left)
  {
    mouseMode = None;
  }
}

void CameraController::SaveSettings()
{

  std::ofstream ofs("../../Settings/CameraSettings.xml");

  if(!ofs.is_open())
  {
    std::cout << "Can not save ../../Settings/CameraSettings.xml\n";
    return;
  }

  boost::archive::xml_oarchive ia(ofs);

  try
  {
    ia << BOOST_SERIALIZATION_NVP(settings);
  }
  catch(...)
  {
    std::cout << "Invalid xml file. ../../Settings/CameraSettings.xml\n";
  }
}

void CameraController::LoadSettings()
{
  std::ifstream ifs("../../Settings/CameraSettings.xml");

  if(!ifs.is_open())
  {
    std::cout << "Can not open ../../Settings/CameraSettings.xml\n";
    return;
  }

  boost::archive::xml_iarchive ia(ifs);

  try
  {
    ia >> BOOST_SERIALIZATION_NVP(settings);
  }
  catch(...)
  {
    std::cout << "Invalid xml file. ../../Settings/CameraSettings.xml\n";
    return;
  }

  ApplySettings();

}

void CameraController::ApplySettings()
{
  ReFocusToMesh();
}

void CameraController::Update(float t)
{
  timeSinceLastFrame = t;

  switch(settings.cameraMode)
  {
  case CameraControllerSettings::Orbital:
    cameraNode->rotate(Ogre::Quaternion(Ogre::Degree(settings.orbitalSpeed.x * t), Ogre::Vector3(1, 0, 0)), Ogre::Node::TransformSpace::TS_WORLD);
    cameraNode->rotate(Ogre::Quaternion(Ogre::Degree(settings.orbitalSpeed.y * t), Ogre::Vector3(0, 1, 0)), Ogre::Node::TransformSpace::TS_WORLD);
    cameraNode->rotate(Ogre::Quaternion(Ogre::Degree(settings.orbitalSpeed.z * t), Ogre::Vector3(0, 0, 1)), Ogre::Node::TransformSpace::TS_WORLD);
    break;
  case CameraControllerSettings::Free:
    {
      bool isShift = keyboard->isKeyDown(OIS::KC_LSHIFT);
      float shiftMultiplier = 2;
      if(!isShift)
        shiftMultiplier = 1;
      if(keyboard->isKeyDown(OIS::KC_W))
      {
        Ogre::Vector3 pos = camera->getPosition();
        pos += 2.2f * camera->getDirection() * t * settings.cameraSpeed * shiftMultiplier;
        camera->setPosition(pos);
      }

      if(keyboard->isKeyDown(OIS::KC_S))
      {
        Ogre::Vector3 pos = camera->getPosition();
        pos += -2.2f * camera->getDirection() * t * settings.cameraSpeed * shiftMultiplier;
        camera->setPosition(pos);
      }

      if(keyboard->isKeyDown(OIS::KC_A))
      {
        Ogre::Quaternion quat = camera->getOrientation();
        Ogre::Vector3 x, y, z;
        quat.ToAxes(x, y, z);

        Ogre::Vector3 pos = camera->getPosition();
        pos += -2.2f * x * t * settings.cameraSpeed * shiftMultiplier;
        camera->setPosition(pos);
      }

      if(keyboard->isKeyDown(OIS::KC_D))
      {
        Ogre::Quaternion quat = camera->getOrientation();
        Ogre::Vector3 x, y, z;
        quat.ToAxes(x, y, z);

        Ogre::Vector3 pos = camera->getPosition();
        pos += 2.2f * x * t * settings.cameraSpeed * shiftMultiplier;
        camera->setPosition(pos);
      }
    }
    break;
  default:
    break;
  }

}

void CameraController::MouseMoved(const OIS::MouseEvent& arg)
{

  float radius = CurrentAsset::get().GetBoundingRadius();

  if(arg.state.Z.rel != 0)
  {
    if(!UI::get().IsOverAnElement(arg.state.X.abs, arg.state.Y.abs))
    {
      Ogre::Vector3 amount = radius * 6.0f * settings.cameraSpeed * (arg.state.Z.rel / 120.0f) * timeSinceLastFrame * camera->getDirection();

      float distanceAfterCameraMove = (camera->getPosition() + amount).distance(Ogre::Vector3::ZERO);
      if(distanceAfterCameraMove > 100000 || distanceAfterCameraMove < 0.1f)
        return;

      camera->move(amount);
    }

  }

  if(mouseMode == Rotating)
  {

    switch(settings.cameraMode)
    {
    case CameraControllerSettings::Fixed:
      if(mouse->getMouseState().buttonDown(OIS::MB_Left))
      {
        CurrentAsset::get().node->rotate(Ogre::Quaternion(Ogre::Degree(arg.state.X.rel * timeSinceLastFrame * 12.0f * settings.cameraSpeed), Ogre::Vector3(0, 1, 0)));
        CurrentAsset::get().node->rotate(Ogre::Quaternion(Ogre::Degree(arg.state.Y.rel * timeSinceLastFrame * 12.0f * settings.cameraSpeed), Ogre::Vector3(1, 0, 0)), Ogre::Node::TS_PARENT);
      }
      break;
    case CameraControllerSettings::Pivot:
      if(mouse->getMouseState().buttonDown(OIS::MB_Left))
      {
        cameraNode->yaw(Ogre::Degree(-arg.state.X.rel * timeSinceLastFrame * 24.0f * settings.cameraSpeed), Ogre::Node::TS_PARENT);
        cameraNode->pitch(Ogre::Degree(-arg.state.Y.rel * timeSinceLastFrame * 24.0f * settings.cameraSpeed));
      }
      break;
    case CameraControllerSettings::Orbital:
      break;
    case CameraControllerSettings::Free:
      {
        Ogre::Quaternion quat = camera->getOrientation();

        Ogre::Vector3 x, y, z;
        quat.ToAxes(x, y, z);

        if(mouse->getMouseState().buttonDown(OIS::MB_Left))
        {
          camera->yaw(Ogre::Degree(static_cast<float>(arg.state.X.rel)) * -1  * timeSinceLastFrame * 1.0f * settings.cameraSpeed);
          camera->pitch(Ogre::Degree(static_cast<float>(arg.state.Y.rel)) * -1  * timeSinceLastFrame * 1.0f * settings.cameraSpeed);
        }

        if(mouse->getMouseState().buttonDown(OIS::MB_Middle))
        {
          Ogre::Vector3 pos = camera->getPosition();

          pos += x * (float)arg.state.X.rel * radius * timeSinceLastFrame * 0.5f * settings.cameraSpeed;
          pos += y * -(float)arg.state.Y.rel * radius * timeSinceLastFrame * 0.5f * settings.cameraSpeed;

          camera->setPosition(pos);
        }
      }
      break;
    }

  }
}

void CameraController::ReFocusToMesh()
{
  float radius = CurrentAsset::get().GetBoundingRadius();

  if(radius < 1)
    camera->setPosition(Ogre::Vector3(0, 1, 1));
  else
    camera->setPosition(Ogre::Vector3(0, radius * 2, radius * 2));

  cameraNode->resetOrientation();
  camera->lookAt(Ogre::Vector3::ZERO);

}
