#include "PreCompile.h"
#include "Log.h"

#include "UI.h"
#include "Nimet.h"
#include <berkelium/WeakString.hpp>

void Log::messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String& logName, bool& skipThisMessage)
{

  std::wstring wsTmp = Ogre::StringConverter::UTF8toUTF16(message);

  if(file.is_open())
  {
    file << wsTmp.c_str() << "\n";
    file.close();
  }
  else
  {
    try
    {
      file.open("Output.log", std::ios_base::out | std::ios_base::app );
      file << wsTmp.c_str() << "\n";
      file.close();
    }
    catch(...)
    {
      std::cout << "Could not open Output.log. Already open or no permission.\n";
    }
  }

  
  if(!Nimet::get().isActive)
      return;

  for(unsigned int i = 0; i < wsTmp.size(); ++i)
  {
    switch(wsTmp[i])
    {
    case '\\':
      wsTmp[i] = '/';
      break;
    case '\n':
      wsTmp[i] = ' ';
      break;
    case '\"':
      wsTmp[i] = '\'';
      break;
    default:
      break;
    }
  }

  if(lml == Ogre::LML_NORMAL && message.length() > 8)
  {
    if(message.substr(0, 4) == "WARN")
    {
      if(initialLog)
        warnings.push_back(wsTmp);
      else
        WriteToWarningWindow(wsTmp);
      return;
    }
  }

  switch(lml)
  {
  case Ogre::LML_CRITICAL:
    if(initialLog)
      errors.push_back(wsTmp);
    else
      WriteToErrorWindow(wsTmp);
    break;
  default:
    break;
  }

  if(initialLog)
    info.push_back(wsTmp);
  else
   WriteToInfoWindow(wsTmp);

}

void Log::destroy()
{
  logManager->destroyLog(defaultLog);
  delete logManager;
}

void Log::WriteToErrorWindow(const std::wstring &msg)
{
  std::wstring ss;
  ss += L"$('#ErrorLogWindow').append(\"";
  ss += L"<div class='logEntryError'>";
  ss += msg;
  ss += L"</div>";
  ss += L"\");";
  UI::get().ExecuteJavascript(Berkelium::WideString::point_to(ss));
}

void Log::WriteToWarningWindow(const std::wstring &msg)
{
  std::wstring ss;
  ss += L"$('#WarningLogWindow').append(\"";
  ss += L"<div class='logEntryWarning'>";
  ss += msg;
  ss += L"</div>";
  ss += L"\");";
  UI::get().ExecuteJavascript(Berkelium::WideString::point_to(ss));
}

void Log::WriteToInfoWindow(const std::wstring &msg)
{
  std::wstring ss;
  ss += L"$('#InfoLogWindow').append(\"";
  ss += L"<div class='logEntryInfo'>";
  ss += msg;
  ss += L"</div>";
  ss += L"\");";
  UI::get().ExecuteJavascript(Berkelium::WideString::point_to(ss));
}

void Log::SetLogToUI()
{
  std::wstring ss;

  ss += L"$('#ErrorLogWindow').append(\"";

  for(auto &it : errors)
  {
    ss += L"<div class='logEntryError'>";
    ss += it;
    ss += L"</div>";
  }
  ss += L"\");";

  ss += L"$('#WarningLogWindow').append(\"";

  for(auto &it : warnings)
  {
    ss += L"<div class='logEntryWarning'>";
    ss += it;
    ss += L"</div>";
  }
  ss += L"\");";

  ss += L"$('#InfoLogWindow').append(\"";

  for(auto &it : info)
  {
    ss += L"<div class='logEntryInfo'>";
    ss += it;
    ss += L"</div>";
  }

  ss +=  L"\");";


  if(!ss.empty())
    UI::get().ExecuteJavascript(Berkelium::WideString::point_to(ss));

  errors.clear();
  warnings.clear();
  info.clear();

  initialLog = false;
}
