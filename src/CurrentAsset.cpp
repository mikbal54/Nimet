#include "PreCompile.h"
#include "CurrentAsset.h"

#include "Nimet.h"
#include "CurrentScene.h"
#include "ResourceWatcher.h"
#include "CameraController.h"
#include "UI.h"
#include "DotSceneLoader.h"

std::wstring CurrentAsset::GetName()
{
  if(entity)
  {
    return Ogre::StringConverter::UTF8toUTF16(entity->getMesh()->getName());
  }

  return L"";
} 

bool CurrentAsset::OpenScene(const boost::filesystem::path &pathToMesh)
{
  DotSceneLoader loader;

  Ogre::SceneNode *tempNode = Nimet::get().mSceneMgr->getRootSceneNode()->createChildSceneNode();

  bool r = loader.parseDotScene(pathToMesh.wstring(), "General", Nimet::get().mSceneMgr, tempNode);

  if(!r)
  {
    CurrentScene::destroySceneNode(tempNode);
    return false;
  }

  DestroyCurrent();

  assetType = AssetType::Scene;

  int polyCount = 0;
  int lightCount = 0;
  int entityCount = 0;

  {
    std::list<Ogre::Entity*> entities;
    GetAllChildEntities(tempNode, entities);
    for(auto &i : entities)
      polyCount += GetEntityPolygonCount(i);
    entityCount = entities.size();
  }

  GetNumberOfLights(tempNode, lightCount);

  {
    std::wstringstream ss;
    ss << L"ShowMessage('Scene opened! ";
    ss << pathToMesh.wstring();
    ss << L"', 3000, 'green');";

    ss << L"$('#ScenePolygonCount').text('" <<  polyCount << "');";
    ss << L"$('#SceneEntityCount').text('" <<  entityCount << "');";
    ss << L"$('#SceneLightCount').text('" <<  lightCount << "');";

    std::wstring ws = ss.str();
    std::replace(ws.begin(), ws.end(), L'\\', L'/');
    UI::get().ExecuteJavascriptWhenLoaded(ws);
  }

  SetWindowTextW(Nimet::get().hwnd, (pathToMesh.leaf().wstring() + std::wstring(L" (") + pathToMesh.wstring() + std::wstring(L") - Nimet")).c_str());

  lastAsset = pathToMesh;

  node = tempNode;

  return true;
}

bool CurrentAsset::OpenMesh(const boost::filesystem::path &pathToMesh)
{
  Ogre::Entity *newEntity = 0;

  std::string name = Ogre::StringConverter::UTF16toUTF8(pathToMesh.leaf().wstring());
  Ogre::StringUtil::toLowerCase(name);

  try
  {
    asset_count++;
    std::stringstream ss;
    ss << asset_count;
    newEntity = sceneManager->createEntity(ss.str(), name, "General");
  }
  catch(...)
  {
    std::cout << "Couldn't open mesh: " << name << "\n";
    return false;
  }

  if(newEntity)
  {
    DestroyCurrent();

    assetType = AssetType::Mesh;

    Ogre::ResourceGroupManager::getSingleton().unloadUnreferencedResourcesInGroup("General", false);

    node = Nimet::get().mSceneMgr->getRootSceneNode()->createChildSceneNode();
    entity = newEntity;
    node->attachObject(entity);

    // Cumulative is better for playing multiple animations at the same time
    if(entity->hasSkeleton())
      entity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

    CalculateMeshStats();

    lastAsset = pathToMesh;

    // TODO: decide whether to recous camera or not
    // CameraController::get().ReFocusToMesh();
    // UI::get().UpdateCurrentAsset();

    {
      std::wstring ws = L"ShowMessage('Mesh opened! ";
      ws += pathToMesh.wstring();
      ws += L"', 3000, 'green')";
      std::replace(ws.begin(), ws.end(), L'\\', L'/');
      UI::get().ExecuteJavascript(ws);
    }

    SetWindowTextW(Nimet::get().hwnd, (pathToMesh.leaf().wstring() + std::wstring(L" (") + pathToMesh.wstring() + std::wstring(L") - Nimet")).c_str());

    return true;
  }

  return false;
}

void CurrentAsset::Destroy()
{
  if(assetType == AssetType::Mesh)
  {
    if(entity)
    {
      bonesEntities.clear();
      boneNames.clear();

      if(normalsNode)
        CurrentScene::destroySceneNode(normalsNode);
      if(node)
        CurrentScene::destroySceneNode(node);

      normalsNode = 0;
      node = 0;
      entity = 0;
    }
    else
      std::cout << "Error: type is mesh but there is no mesh loaded!\n";
  }
  else if(assetType == AssetType::Scene)
  {
    CurrentScene::destroySceneNode(node);
    node = 0;
  }

  Ogre::ResourceGroupManager::getSingleton().unloadUnreferencedResourcesInGroup("General");
}

void CurrentAsset::DestroyCurrent()
{
  Destroy();
}

void CurrentAsset::CalculateMeshStats()
{

  Ogre::AxisAlignedBox box = entity->getBoundingBox();
  center = box.getCenter();
  min = box.getMinimum();
  max = box.getMaximum();

  ///
  /// Vertex Count
  ///

  polygonCount.clear();
  vertexCount.clear();
  bool added_shared = false;

  Ogre::Mesh *mesh = entity->getMesh().get();

  for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
  {
    vertexCount.push_back(0);
    Ogre::SubMesh* submesh = mesh->getSubMesh(i);

    polygonCount.push_back(GetSubMeshPolygonCount(submesh));

    // We only need to add the shared vertices once
    if(submesh->useSharedVertices)
    {
      if(!added_shared)
      {
        vertexCount.back() += mesh->sharedVertexData->vertexCount;
        added_shared = true;
      }
    }
    else
    {
      vertexCount.back() += submesh->vertexData->vertexCount;
    }

  }

}

int CurrentAsset::ToggleAnimation(const std::string &name)
{
  if(entity)
  {
    Ogre::AnimationState *anim = entity->getAnimationState(name);
    for(size_t i = 0; i < animationStates.size(); ++i)
    {
      if(animationStates[i] == anim)
      {
        anim->setLoop(false);
        anim->setEnabled(false);
        animationStates.erase(animationStates.begin() + i);
        return 2;
      }
    }

    anim->setLoop(true);
    anim->setEnabled(true);
    animationStates.push_back(anim);

    return 1;
  }

  return -1;
}

void CurrentAsset::Update(float timeSinceLastFrame)
{
  for(size_t i = 0; i < animationStates.size(); ++i)
    animationStates[i]->addTime(timeSinceLastFrame);
}

void CurrentAsset::ShowBones(bool isShown)
{
  // destroy previous presentation
  bonesEntities.clear();

  if(isShown && entity && entity->hasSkeleton())
  {
    Ogre::SkeletonInstance *skeleton = entity->getSkeleton();

    size_t size = skeleton->getNumBones();

    float boneSize = CurrentScene::get().GetCurrentSceneSettings().boneSize;

    for(size_t i = 0; i < size; ++i)
    {

      Ogre::Bone *bone = skeleton->getBone(i);

      CurrentAsset::BoneVisuals &visual = bonesEntities[bone->getName()];
      visual.bone = bone;
      visual.entity = Nimet::get().mSceneMgr->createEntity("Bone.mesh");
      visual.tagPoint = entity->attachObjectToBone(bone->getName(), visual.entity);
      visual.tagPoint->setScale(Ogre::Vector3(entity->getBoundingRadius() * 0.015f * boneSize));
      visual.entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);

      ShowBoneLinks(visual);
    }

  }

}

void CurrentAsset::ShowBoneLinks(BoneVisuals &boneVisuals)
{
  int numChildren = boneVisuals.bone->numChildren();
  if(numChildren)
  {
    Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
    Ogre::Bone *parent = boneVisuals.bone;
    float radius = GetBoundingRadius();
    float boneSize = CurrentScene::get().GetCurrentSceneSettings().boneSize;

    Ogre::Bone::ChildNodeIterator it = parent->getChildIterator();
    while(it.hasMoreElements())
    {
      Ogre::Bone *child = static_cast<Ogre::Bone*>(it.getNext());

      if(dynamic_cast<Ogre::TagPoint*>(child))
        continue;

      Ogre::Vector3 dif(parent->_getDerivedOrientation().Inverse() * (child->_getDerivedPosition() - parent->_getDerivedPosition()));
      float diffLength = dif.normalise();

      if(diffLength < 0.001f)
        continue;

      Ogre::Quaternion rotationToChild(Ogre::Vector3::UNIT_Z.getRotationTo(dif));

      boneVisuals.toChildren.push_back(BoneVisuals::BoneLinkVisual());
      BoneVisuals::BoneLinkVisual &link = boneVisuals.toChildren.back();

      link.entity = mgr->createEntity("BoneLink.mesh");
      link.tagPoint = entity->attachObjectToBone(parent->getName(), link.entity, rotationToChild);
      link.tagPoint->setScale(0.01f * radius * boneSize, 0.01f * radius * boneSize, diffLength);
      link.tagPoint->setInheritScale(false);

      // after all. but before bone
      link.entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE - 1);

    }
  }
}


int CurrentAsset::GetEntityPolygonCount(Ogre::Entity *entity)
{
  int polyCount = 0;

  int subNumber = entity->getNumSubEntities();

  for(int i = 0; i < subNumber; ++i)
    polyCount += GetSubMeshPolygonCount(entity->getSubEntity(i)->getSubMesh());

  return polyCount;
}

void CurrentAsset::GetAllChildEntities(Ogre::SceneNode *node, std::list<Ogre::Entity*> &children )
{
  auto attachedIterator = node->getAttachedObjectIterator();
  while(attachedIterator.hasMoreElements())
  {
    Ogre::MovableObject *m = attachedIterator.getNext();

    if(dynamic_cast<Ogre::Entity*>(m))
      children.push_back(static_cast<Ogre::Entity*>(m));
  }


  auto iter = node->getChildIterator();

  while(iter.hasMoreElements())
  {
    Ogre::Node *n = iter.getNext();
    if(dynamic_cast<Ogre::SceneNode*>(n))
    {
      Ogre::SceneNode *sceneNode = static_cast<Ogre::SceneNode*>(n);

      GetAllChildEntities(sceneNode, children);
    }
  }
}

void CurrentAsset::GetNumberOfLights(Ogre::SceneNode *node, int &count)
{

  auto attachedIterator = node->getAttachedObjectIterator();
  while(attachedIterator.hasMoreElements())
  {
    Ogre::MovableObject *m = attachedIterator.getNext();

    if(dynamic_cast<Ogre::Light*>(m))
      count++;
  }


  auto iter = node->getChildIterator();
  while(iter.hasMoreElements())
  {
    Ogre::Node *n = iter.getNext();
    if(dynamic_cast<Ogre::SceneNode*>(n))
    {
      Ogre::SceneNode *sceneNode = static_cast<Ogre::SceneNode*>(n);

      GetNumberOfLights(sceneNode, count);
    }
  }
}

int CurrentAsset::GetSubMeshPolygonCount(Ogre::SubMesh* mesh)
{
  int count = 0;
  Ogre::RenderOperation op;
  mesh->_getRenderOperation(op);

  switch(op.operationType)
  {
  case Ogre::RenderOperation::OT_TRIANGLE_LIST:
    count = op.indexData->indexCount / 3;
    break;
  case Ogre::RenderOperation::OT_TRIANGLE_STRIP:
  case Ogre::RenderOperation::OT_TRIANGLE_FAN:
    count = op.indexData->indexCount - 2;
    break;
  default:
    break;
  }

  return count;
}

bool CurrentAsset::ToggleBoneVisuals()
{
  if(assetType != AssetType::Mesh)
    return false;

  if(!entity->hasSkeleton())
    return false;

  if(bonesEntities.empty())
  {
    ShowBones(true);
    return true;
  }
  else
  {
    ShowBones(false);
    return false;
  }
}

bool CurrentAsset::ToggleBoneNames()
{
  if(assetType != AssetType::Mesh)
    return false;

  if(!entity->hasSkeleton())
    return false;

  if(boneNames.empty())
  {
    ShowBoneNames(true);
    return true;
  }
  else
  {
    ShowBoneNames(false);
    return false;
  }
}

void CurrentAsset::ShowBoneNames(bool isShown)
{
  if(assetType != AssetType::Mesh)
    return;

  if(isShown)
  {

    if(isShown && entity && entity->hasSkeleton())
    {
      Ogre::SkeletonInstance *skeleton = entity->getSkeleton();

      size_t size = skeleton->getNumBones();

      for(size_t i = 0; i < size; ++i)
      {

        Ogre::Bone *bone = skeleton->getBone(i);

        BoneName &boneName = boneNames[bone->getName()];
        boneName.init(bone);
      }

    }

  }
  else
  {
    boneNames.clear();
  }
}

void CurrentAsset::ResetOrientation()
{
  if(entity)
  {
    node->resetOrientation();
  }
}

void CurrentAsset::ShowAnimationDetails(const std::string& name)
{
  if(entity)
  {
    if(entity->hasSkeleton())
    {
      Ogre::SkeletonInstance *skeleton = entity->getSkeleton();

      if(skeleton->hasAnimation(name))
      {
        Ogre::Animation *anim = skeleton->getAnimation(name);

        std::stringstream ss;
        ss << "ShowAnimationDetails(new AnimationDetails(";
        ss << "'" << anim->getName() << "',";
        ss << anim->getNumNodeTracks() << ",";

        ss << "[";
        Ogre::Animation::NodeTrackIterator it = anim->getNodeTrackIterator();
        for(;;)
        {
          Ogre::NodeAnimationTrack *node = it.getNext();

          if(node->getAssociatedNode())
            ss << "new BoneInfo('" << node->getAssociatedNode()->getName() << "'," << node->getNumKeyFrames() << ")";

          if(it.hasMoreElements())
            ss << ",";
          else
            break;
        }
        ss << "]))";

        UI::get().ExecuteJavascript(ss.str());
      }
    }
  }
}

void CurrentAsset::ApplyBoneSize()
{
  if(assetType != AssetType::Mesh)
    return;

  float size = CurrentScene::get().GetCurrentSceneSettings().boneSize;
  float radius = entity->getBoundingRadius();
  for(auto it = bonesEntities.begin(); it != bonesEntities.end(); ++it)
  {
    it->second.tagPoint->setScale(Ogre::Vector3(radius * 0.015f * size));

    for(auto &child : it->second.toChildren)
    {
      Ogre::Vector3 prevScale = child.tagPoint->getScale();

      child.tagPoint->setScale(0.01f * radius * size, 0.01f * radius * size, prevScale.z);

    }
  }

  for(auto &it : boneNames)
  {
    it.second.tagPoint->setScale(Ogre::Vector3(radius * 0.02f * size));
  }
}

void CurrentAsset::ShowNormals(bool isShown)
{
  if(!entity)
    return;

  if(isShown)
  {
    if(normalsNode)
      CurrentScene::destroySceneNode(normalsNode);

    size_t vertex_count, index_count;

    Ogre::Vector3* vertices;
    unsigned long* indices;

    getMeshInformation(entity->getMesh().get(), vertex_count, vertices, index_count, indices);

    Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
    Ogre::ManualObject *obj = mgr->createManualObject();
    obj->begin("Normal", Ogre::RenderOperation::OT_LINE_LIST);

    int count = 0;
    float normalsSize = CurrentScene::get().GetCurrentSceneSettings().normalsSize;
    for(size_t i = 0; i < index_count; i += 3)
    {
      Ogre::Vector3 ver1 = vertices[indices[i]];
      Ogre::Vector3 ver2 = vertices[indices[i + 1]];
      Ogre::Vector3 ver3 = vertices[indices[i + 2]];

      Ogre::Vector3 dir = (ver2 - ver1).crossProduct(ver3 - ver1);
      dir.normalise();

      Ogre::Vector3 pos = (ver1 + ver2 + ver3) / 3;
      obj->position(pos);
      obj->position(pos + (dir * GetBoundingRadius() * 0.02f * normalsSize));
      obj->index(count);
      obj->index(count + 1);

      //skip one
      count += 2;
    }

    obj->end();

    normalsNode = node->createChildSceneNode();
    normalsNode->attachObject(obj);

    delete[] vertices;
    delete[] indices;
  }
  else
  {
    if(normalsNode)
    {
      CurrentScene::destroySceneNode(normalsNode);
      normalsNode = 0;
    }
  }
}

bool CurrentAsset::ToggleNormals()
{
  if(assetType != AssetType::Mesh)
    return false;

  if(normalsNode)
  {
    ShowNormals(false);
    return false;
  }
  else
  {
    ShowNormals(true);
    return true;
  }
}

void CurrentAsset::ForceReload()
{
  if(entity)
  {
    auto it = ResourceWatcher::get().GetResourcePath(entity->getMesh()->getName());

    if(!it)
      return; // TODO: log an error

    if(OpenMesh(*it))
      UI::get().UpdateCurrentAsset();
  }
}

float CurrentAsset::GetBoundingRadius()
{
  if(entity)
  {
    return entity->getBoundingRadius();
  }

  return 1;
}

CurrentAsset::BoneVisuals::BoneLinkVisual::~BoneLinkVisual()
{
  if(tagPoint)
  {
    Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
    entity->detachFromParent();
    mgr->destroyEntity(entity);
  }
}

CurrentAsset::BoneVisuals::~BoneVisuals()
{
  // calls destructor of BoneLinkVisual
  toChildren.clear();

  if(tagPoint)
  {
    Ogre::SceneManager *mgr = Nimet::get().mSceneMgr;
    entity->detachFromParent();
    mgr->destroyEntity(entity);
  }
}

CurrentAsset::BoneName::~BoneName()
{
  if(text)
  {
    text->detachFromParent();
    delete text;
  }
}

void CurrentAsset::BoneName::init(Ogre::Bone *bone)
{
  text = new Ogre::MovableText(bone->getName() + "_mt", bone->getName(), "BoneText");
  text->setCharacterHeight(1);
  text->showOnTop(true);
  text->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_CENTER);
  text->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
  tagPoint = CurrentAsset::get().entity->attachObjectToBone(bone->getName(), text);
  tagPoint->setScale(Ogre::Vector3(CurrentAsset::get().GetBoundingRadius() * 0.02f * CurrentScene::get().GetCurrentSceneSettings().boneSize));
}

void CurrentAsset::getMeshInformation(Ogre::Mesh* mesh, size_t &vertex_count, Ogre::Vector3* &vertices, size_t &index_count, unsigned long* &indices, const Ogre::Vector3 &position,
                                      const Ogre::Quaternion &orient, const Ogre::Vector3 &scale)
{
  bool added_shared = false;
  size_t current_offset = 0;
  size_t shared_offset = 0;
  size_t next_offset = 0;
  size_t index_offset = 0;

  vertex_count = index_count = 0;

  // Calculate how many vertices and indices we're going to need
  for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
  {
    Ogre::SubMesh* submesh = mesh->getSubMesh(i);
    // We only need to add the shared vertices once
    if(submesh->useSharedVertices)
    {
      if(!added_shared)
      {
        vertex_count += mesh->sharedVertexData->vertexCount;
        added_shared = true;
      }
    }
    else
    {
      vertex_count += submesh->vertexData->vertexCount;
    }
    // Add the indices
    index_count += submesh->indexData->indexCount;
  }

  // Allocate space for the vertices and indices
  vertices = new Ogre::Vector3[vertex_count];
  indices = new unsigned long[index_count];

  added_shared = false;

  // Run through the submeshes again, adding the data into the arrays
  for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
  {
    Ogre::SubMesh* submesh = mesh->getSubMesh(i);

    Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

    if((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
    {
      if(submesh->useSharedVertices)
      {
        added_shared = true;
        shared_offset = current_offset;
      }

      const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

      Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

      unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

      // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
      //  as second argument. So make it float, to avoid trouble when Ogre::Real will
      //  be comiled/typedefed as double:
      //Ogre::Real* pReal;
      float* pReal;

      for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
      {
        posElem->baseVertexPointerToElement(vertex, &pReal);
        Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
        vertices[current_offset + j] = (orient * (pt * scale)) + position;
      }

      vbuf->unlock();
      next_offset += vertex_data->vertexCount;
    }

    Ogre::IndexData* index_data = submesh->indexData;

    if(index_data->indexBuffer.isNull())
    {
      //TODO: log
      std::cout << "Can not show normals, mesh doesn't have index buffer\n";
      return;
    }

    size_t numTris = index_data->indexCount / 3;
    Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

    bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

    unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

    size_t offset = (submesh->useSharedVertices) ? shared_offset : current_offset;

    if(use32bitindexes)
    {
      for(size_t k = 0; k < numTris * 3; ++k)
      {
        indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
      }
    }
    else
    {
      for(size_t k = 0; k < numTris * 3; ++k)
      {
        indices[index_offset++] = static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset);
      }
    }

    ibuf->unlock();
    current_offset = next_offset;
  }
}
