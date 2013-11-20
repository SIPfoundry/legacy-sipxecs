#include "sipx/bridge/FreeSwitchRunner.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <poll.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <switch.h>

   
#define FS_DEFAULT_CONF_DIR SIPX_DBDIR
#define FS_DEFAULT_MOD_DIR FREESWITCH_MODDIR
#define FS_DEFAULT_LOG_DIR SIPX_LOGDIR
#define FS_DEFAULT_RUN_DIR SIPX_RUNDIR
#define FS_DEFAULT_DATA_DIR  FS_DEFAULT_CONF_DIR
#define DEFAULT_ESL_PORT 2022
#define DEFAULT_FS_ESL_PORT 2020
#define FS_DEFAULT_CODEC_PREFERENCE "PCMU,PCMA,opus" /*"PCMU,PCMA,opus,VP8,H264"*/
#define FS_DEFAULT_SIP_PORT 5066
#define FS_DEFAULT_SIP_ADDRESS "$${local_ip_v4}"


extern std::string freeswitch_xml;

namespace sipx {
namespace bridge {


static FreeSwitchRunner* gpInstance = 0;
static switch_core_flag_t gFlags = SCF_CALIBRATE_CLOCK | SCF_USE_CLOCK_RT;


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
  _enableCoreDumps(false),
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
  _sipAddress(FS_DEFAULT_SIP_ADDRESS)
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
    _pSwitchThread = new boost::thread(boost::bind(&FreeSwitchRunner::switch_loop, this, noconsole));
  else
    switch_loop(noconsole);
}

bool FreeSwitchRunner::initialize()
{  
  if (_enableCoreDumps)
  {
    struct rlimit rlp;
    memset(&rlp, 0, sizeof(rlp));
    rlp.rlim_cur = RLIM_INFINITY;
    rlp.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlp);
  }
  
  if (_applicationName.empty() || _configDirectory.empty())
  {
    std::cerr << "Application Name and Config Directory must be provided" << std::endl;
    return false;
  }
  

  std::ostringstream configPath;
  configPath << _configDirectory << "/" << _applicationName;
  
  if (!verify_directory(configPath.str()))
    return false;

  SWITCH_GLOBAL_dirs.conf_dir = (char *) malloc(configPath.str().size() + 1);
  strcpy(SWITCH_GLOBAL_dirs.conf_dir, configPath.str().c_str());


  SWITCH_GLOBAL_dirs.mod_dir = (char *) malloc(_modDirectory.size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.mod_dir, _modDirectory.c_str());
  

  std::ostringstream logDir;
  logDir << _logDirectory << "/" << _applicationName;
  if (!verify_directory(logDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.log_dir = (char *) malloc(logDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.log_dir, logDir.str().c_str());
 
    
  std::ostringstream runDir;
  runDir << _runDirectory << "/" << _applicationName;
  if (!verify_directory(runDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.run_dir = (char *) malloc(runDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.run_dir, runDir.str().c_str());
  

  std::ostringstream dbDir;
  dbDir << _dataDirectory << "/" << _applicationName << "/db";
  if (!verify_directory(dbDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.db_dir = (char *) malloc(dbDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.db_dir, dbDir.str().c_str());
 
 
  std::ostringstream storageDir;
  storageDir << _storageDirectory << "/" << _applicationName << "/storage";
  if (!verify_directory(storageDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.storage_dir = (char *) malloc(storageDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.storage_dir, storageDir.str().c_str());
  
 
  std::ostringstream scriptsDir;
  scriptsDir << _scriptsDirectory << "/" << _applicationName << "/scripts";
  if (!verify_directory(scriptsDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.script_dir = (char *) malloc(scriptsDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.script_dir, scriptsDir.str().c_str());
  
  std::ostringstream htdocsDir;
  htdocsDir << _htdocsDirectory << "/" << _applicationName << "/htdocs";
  if (!verify_directory(htdocsDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.htdocs_dir = (char *) malloc(htdocsDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.htdocs_dir, htdocsDir.str().c_str());
  
  std::ostringstream recordingsDir;
  recordingsDir << _recordingsDirectory << "/" << _applicationName << "/recordings";
  if (!verify_directory(recordingsDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.recordings_dir = (char *) malloc(recordingsDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.recordings_dir, recordingsDir.str().c_str());
  
  std::ostringstream grammarDir;
  grammarDir << _grammarDirectory << "/" << _applicationName << "/grammar";
  if (!verify_directory(grammarDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.grammar_dir = (char *) malloc(grammarDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.grammar_dir, grammarDir.str().c_str());
  
  std::ostringstream certsDir;
  certsDir << _certsDirectory << "/" << _applicationName << "/certs";
  if (!verify_directory(certsDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.certs_dir = (char *) malloc(certsDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.certs_dir, certsDir.str().c_str());
  
  std::ostringstream soundsDir;
  soundsDir << _soundsDirectory << "/" << _applicationName << "/sounds";
  if (!verify_directory(soundsDir.str()))
    return false;
  
  SWITCH_GLOBAL_dirs.sounds_dir = (char *) malloc(soundsDir.str().size() + 1);
	strcpy(SWITCH_GLOBAL_dirs.sounds_dir, soundsDir.str().c_str());

  
  return generateConfig();
}

void FreeSwitchRunner::switch_loop(bool noconsole)
{

  set_auto_priority();
  
  switch_core_setrlimits();
  
  switch_core_set_globals();
   
  const char* err = 0;
  if (switch_core_init_and_modload(gFlags, noconsole ? SWITCH_FALSE : SWITCH_TRUE, &err) != SWITCH_STATUS_SUCCESS) 
  {
		std::cerr << "Unable to load modules. " << err << std::endl;
    _exit(EXIT_FAILURE);
	}
  
  switch_core_runtime_loop(noconsole);
  
  switch_core_destroy();
}
  
void FreeSwitchRunner::stop()
{
  int32_t arg = 0;
	switch_core_session_ctl(SCSC_SHUTDOWN, &arg);
  if (_pSwitchThread)
    _pSwitchThread->join();
	return;
}

bool FreeSwitchRunner::generateConfig()
{
  std::string eslPort = boost::lexical_cast<std::string>(_eslPort);
  std::string sipPort = boost::lexical_cast<std::string>(_sipPort);
  
  string_replace(freeswitch_xml, "@ESL_PORT@", eslPort.c_str());
  
  string_replace(freeswitch_xml, "@SIP_PORT@", sipPort.c_str());
  
  string_replace(freeswitch_xml, "@SIP_ADDRESS@", _sipAddress.c_str());
  
  std::string fsEslPort = boost::lexical_cast<std::string>(_fsEslPort);
  string_replace(freeswitch_xml, "@FS_ESL_PORT@", fsEslPort.c_str());
  
  string_replace(freeswitch_xml, "@APPLICATION_NAME@", _applicationName.c_str());
  
  string_replace(freeswitch_xml, "@CODEC_PREFERENCE@", _codecPreference.c_str());
  
  std::ostringstream configPath;
  configPath << _configDirectory << "/" << _applicationName << "/freeswitch.xml";
  boost::filesystem::remove(configPath.str().c_str());
  
  std::ofstream configFile(configPath.str().c_str());
  if (!configFile.is_open())
    return false;
  configFile << freeswitch_xml;
  return true;
}

}} // sipx::bridge



