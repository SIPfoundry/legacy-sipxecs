#include "sipx/bridge/FreeSwitchRunner.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

#if 0

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <poll.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <cstdio>
#include <switch.h>
#endif
   
#define FS_DEFAULT_CONF_DIR SIPX_DBDIR
#define FS_DEFAULT_MOD_DIR FREESWITCH_MODDIR
#define FS_DEFAULT_LOG_DIR SIPX_LOGDIR
#define FS_DEFAULT_RUN_DIR SIPX_RUNDIR
#define FS_DEFAULT_DATA_DIR  FS_DEFAULT_CONF_DIR
#define DEFAULT_ESL_PORT 2022
#define DEFAULT_FS_ESL_PORT 2020
#define FS_DEFAULT_CODEC_PREFERENCE "PCMU,PCMA" /*"PCMU,PCMA,opus,VP8,H264"*/
#define FS_DEFAULT_SIP_PORT 5066
#define FS_DEFAULT_SIP_ADDRESS "$${local_ip_v4}"
#define FS_INIT_WAIT 5000
#define FS_PROCESS "freeswitch"

namespace sipx {
namespace bridge {

std::string FreeSwitchRunner::_gFreeswitchXml;
static FreeSwitchRunner* gpInstance = 0;

#if 0
static switch_core_flag_t gFlags = SCF_CALIBRATE_CLOCK | SCF_USE_CLOCK_RT;
#endif

FreeSwitchRunner* FreeSwitchRunner::instance()
{
  if (!gpInstance)
    gpInstance = new FreeSwitchRunner();
  
  return gpInstance;
}

void FreeSwitchRunner::delete_instance()
{
  delete gpInstance;
  gpInstance = 0;
}

static bool verify_directory(const std::string path)
{
  if (!boost::filesystem::exists(path.c_str()))
  {
    if (!boost::filesystem::create_directories(path.c_str()))
    {
      std::cerr << "Unable to create data directory " << path.c_str();
      return false;
    }
  }
  
  return true;
}

static void string_replace(std::string& str, const char* what, const char* with)
{
  size_t pos = 0;
  while((pos = str.find(what, pos)) != std::string::npos)
  {
     str.replace(pos, strlen(what), with);
     pos += strlen(with);
  }
}

FreeSwitchRunner::FreeSwitchRunner() :
  _applicationName(),
  _configDirectory(FS_DEFAULT_CONF_DIR),
  _modDirectory(FS_DEFAULT_MOD_DIR),
  _logDirectory(FS_DEFAULT_LOG_DIR),
  _runDirectory(FS_DEFAULT_RUN_DIR),
  _dataDirectory(FS_DEFAULT_DATA_DIR),
  _storageDirectory(FS_DEFAULT_DATA_DIR),
  _scriptsDirectory(FS_DEFAULT_DATA_DIR),
  _htdocsDirectory(FS_DEFAULT_DATA_DIR),
  _recordingsDirectory(FS_DEFAULT_DATA_DIR),
  _grammarDirectory(FS_DEFAULT_DATA_DIR),
  _certsDirectory(FS_DEFAULT_DATA_DIR),
  _soundsDirectory(FS_DEFAULT_DATA_DIR),
  _pSwitchThread(0),
  _eslPort(DEFAULT_ESL_PORT),
  _fsEslPort(DEFAULT_FS_ESL_PORT),
  _codecPreference(FS_DEFAULT_CODEC_PREFERENCE),
  _sipPort(FS_DEFAULT_SIP_PORT),
  _sipAddress(FS_DEFAULT_SIP_ADDRESS),
  _pProcess(0)
{
}

FreeSwitchRunner::~FreeSwitchRunner()
{
  delete _pSwitchThread;
}

void FreeSwitchRunner::run(bool noconsole)
{
  assert(!_pSwitchThread);
  if (noconsole)
  {
    _startupScript << " -nc";
    start();
  }
  else
  {
    _startupScript << " -c";
    OSS::Exec::Command cmd;
    cmd.execute(_startupScript.str());
  }
}

void FreeSwitchRunner::start()
{
  delete _pProcess;
  _pProcess = new OSS::Exec::Process(FS_PROCESS, _startupScript.str(), _shutdownScript.str(), _pidFile);
  _pProcess->deadProcHandler = boost::bind(&FreeSwitchRunner::onDeadProcess, this, _1);
  _pProcess->setInitializeWait(FS_INIT_WAIT);
  _pProcess->executeAndMonitor();
}

OSS::Exec::Process::Action FreeSwitchRunner::onDeadProcess(int consecutiveHits)
{
  if (consecutiveHits > 10)
    return OSS::Exec::Process::ProcessBackoff;
  else
    return OSS::Exec::Process::ProcessRestart;
}

bool FreeSwitchRunner::initialize()
{  
  _startupScript.clear();
  _startupScript << FREESWITCH_PATH << " ";
  _startupScript << "-nosql" << " ";
  _startupScript << "-nonat" << " ";
  _startupScript << "-nonatmap" << " "; 
  
  if (_applicationName.empty() || _configDirectory.empty())
  {
    std::cerr << "Application Name and Config Directory must be provided" << std::endl;
    return false;
  }
  
  std::ostringstream configPath;
  configPath << _configDirectory << "/" << _applicationName;
  if (!verify_directory(configPath.str()))
    return false;
  
  _startupScript << "-conf " << configPath.str() << " ";
  _startupScript << "-mod " << _modDirectory << " ";
  
  std::ostringstream logDir;
  logDir << _logDirectory << "/" << _applicationName;
  if (!verify_directory(logDir.str()))
    return false;
  _startupScript << "-log " << logDir.str() << " ";
  
  std::ostringstream runDir;
  runDir << _runDirectory << "/" << _applicationName;
  if (!verify_directory(runDir.str()))
    return false;
  _startupScript << "-run " << runDir.str() << " ";
  
  _pidFile = runDir.str() + std::string("/") + std::string(FS_PROCESS ".pid");
  
  std::ostringstream dbDir;
  dbDir << _dataDirectory << "/" << _applicationName << "/db";
  if (!verify_directory(dbDir.str()))
    return false;
  _startupScript << "-db " << dbDir.str() << " ";
  
  std::ostringstream storageDir;
  storageDir << _storageDirectory << "/" << _applicationName << "/storage";
  if (!verify_directory(storageDir.str()))
    return false;
  _startupScript << "-storage " << storageDir.str() << " ";
  
  std::ostringstream scriptsDir;
  scriptsDir << _scriptsDirectory << "/" << _applicationName << "/scripts";
  if (!verify_directory(scriptsDir.str()))
    return false;
  _startupScript << "-scripts " << scriptsDir.str() << " ";
  
  std::ostringstream htdocsDir;
  htdocsDir << _htdocsDirectory << "/" << _applicationName << "/htdocs";
  if (!verify_directory(htdocsDir.str()))
    return false;
  _startupScript << "-htdocs " << htdocsDir.str() << " ";
  
  std::ostringstream recordingsDir;
  recordingsDir << _recordingsDirectory << "/" << _applicationName << "/recordings";
  if (!verify_directory(recordingsDir.str()))
    return false;
  _startupScript << "-recordings " << recordingsDir.str() << " ";
  
  std::ostringstream grammarDir;
  grammarDir << _grammarDirectory << "/" << _applicationName << "/grammar";
  if (!verify_directory(grammarDir.str()))
    return false;
  _startupScript << "-grammar " << grammarDir.str() << " ";
  
  std::ostringstream certsDir;
  certsDir << _certsDirectory << "/" << _applicationName << "/certs";
  if (!verify_directory(certsDir.str()))
    return false;
  _startupScript << "-certs " << certsDir.str() << " ";
  
  std::ostringstream soundsDir;
  soundsDir << _soundsDirectory << "/" << _applicationName << "/sounds";
  if (!verify_directory(soundsDir.str()))
    return false;
  _startupScript << "-sounds " << soundsDir.str() << " ";
     
  _shutdownScript << "kill -9 `cat " << _pidFile << "`";
  
  return generateConfig();
}

 
void FreeSwitchRunner::stop()
{
  if (_pProcess)
    _pProcess->shutDown();

  if (_pSwitchThread)
    _pSwitchThread->join();
	return;
}

bool FreeSwitchRunner::generateConfig()
{
  std::string eslPort = boost::lexical_cast<std::string>(_eslPort);
  std::string sipPort = boost::lexical_cast<std::string>(_sipPort);
  
  string_replace(FreeSwitchRunner::_gFreeswitchXml, "@ESL_PORT@", eslPort.c_str());
  
  string_replace(FreeSwitchRunner::_gFreeswitchXml, "@SIP_PORT@", sipPort.c_str());
  
  string_replace(FreeSwitchRunner::_gFreeswitchXml, "@SIP_ADDRESS@", _sipAddress.c_str());
  
  std::string fsEslPort = boost::lexical_cast<std::string>(_fsEslPort);
  string_replace(FreeSwitchRunner::_gFreeswitchXml, "@FS_ESL_PORT@", fsEslPort.c_str());
  
  string_replace(FreeSwitchRunner::_gFreeswitchXml, "@APPLICATION_NAME@", _applicationName.c_str());
  
  string_replace(FreeSwitchRunner::_gFreeswitchXml, "@CODEC_PREFERENCE@", _codecPreference.c_str());
  
  std::ostringstream configPath;
  configPath << _configDirectory << "/" << _applicationName << "/freeswitch.xml";
  boost::filesystem::remove(configPath.str().c_str());
  
  std::ofstream configFile(configPath.str().c_str());
  if (!configFile.is_open())
    return false;
  configFile << FreeSwitchRunner::_gFreeswitchXml;
  return true;
}

}} // sipx::bridge



