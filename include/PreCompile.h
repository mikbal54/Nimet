#include <OGRE/OgreRoot.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreConfigFile.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreTagPoint.h>
#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreWindowEventUtilities.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>

#include <OIS.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/unordered_map.hpp> // required for filesystem::path hash
#include <boost/lexical_cast.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>

#include <boost/locale.hpp>

#include "External/MovableText/MovableText.h"
#include "ogreHTML.h"

#include <functional>
#include <thread>
#include <shellapi.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#include <fcntl.h>
#include <io.h>
#include <ShlObj.h>
#endif

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, Ogre::Vector3 & g, const unsigned int version)
{
    ar & make_nvp("x", g.x);
    ar & make_nvp("y", g.y);
    ar & make_nvp("z", g.z);
}

template<class Archive>
void serialize(Archive & ar, Ogre::ColourValue & g, const unsigned int version)
{
    ar & make_nvp("r", g.r);
    ar & make_nvp("g", g.g);
    ar & make_nvp("b", g.b);
    ar & make_nvp("a", g.a);
}

template<class Archive>
void serialize(Archive & ar, Ogre::Quaternion & g, const unsigned int version)
{
    ar & make_nvp("w", g.w);
    ar & make_nvp("x", g.x);
    ar & make_nvp("y", g.y);
    ar & make_nvp("z", g.z);
}

/**/

template<class Archive>
void load(Archive & ar, Ogre::Degree & g, const unsigned int version)
{
  float r = 0;
  ar & BOOST_SERIALIZATION_NVP(r);
  g = Ogre::Degree(r);
}

template<class Archive>
void save(Archive & ar, const Ogre::Degree & g, const unsigned int version)
{
  float r = g.valueDegrees();
  ar & BOOST_SERIALIZATION_NVP(r) ;
}


} // namespace serialization
} // namespace boost


BOOST_SERIALIZATION_SPLIT_FREE(Ogre::Degree);
