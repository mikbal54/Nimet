#pragma once

class Log: public Ogre::LogListener
{
private:

  Log()
    : logManager(0),
    defaultLog(0),
    initialLog(true)
  {
    try
    {
      file.open("Output.log", std::ios_base::out | std::ios_base::trunc );
      file.close();
    }
    catch(...)
    {
      std::cout << "Could not open Output.log. Already open or no permission.\n";
    }
  }

  std::vector<std::wstring> errors;
  std::vector<std::wstring> warnings;
  std::vector<std::wstring> info;
  std::wofstream file;

public:

  Ogre::LogManager *logManager;
  Ogre::Log *defaultLog;
  bool initialLog;

  static Log &get()
  {
    static Log instance;
    return instance;
  }

  void destroy();

  void messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName, bool& skipThisMessage);

  void SetLogToUI();

  void WriteToErrorWindow(const std::wstring &msg);

  void WriteToWarningWindow(const std::wstring &msg);

  void WriteToInfoWindow(const std::wstring &msg);
};
