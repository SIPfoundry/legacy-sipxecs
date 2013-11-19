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

#define INT8_MIN         (-127-1)
#define INT16_MIN        (-32767-1)
#define INT32_MIN        (-2147483647-1)
#define INT64_MIN        (-9223372036854775807LL-1LL)

#define INT8_MAX         +127
#define INT16_MAX        +32767
#define INT32_MAX        +2147483647
#define INT64_MAX        +9223372036854775807LL

#define UINT8_MAX         255
#define UINT16_MAX        65535
#define UINT32_MAX        4294967295U
#define UINT64_MAX        18446744073709551615ULL

#include <switch.h>
#include <switch_version.h>
#include <private/switch_core_pvt.h>
   
#define FS_DEFAULT_CONF_DIR SIPX_DBDIR
#define FS_DEFAULT_MOD_DIR FREESWITCH_MODDIR
#define FS_DEFAULT_LOG_DIR SIPX_LOGDIR
#define FS_DEFAULT_RUN_DIR SIPX_RUNDIR
#define FS_DEFAULT_DATA_DIR  FS_DEFAULT_CONF_DIR
#define DEFAULT_ESL_PORT 2022
#define DEFAULT_FS_ESL_PORT 2020
#define FS_DEFAULT_CODEC_PREFERENCE "PCMU,PCMA,opus,VP8,H264"


extern std::string freeswitch_xml;

namespace sipx {
namespace bridge {


static FreeSwitchRunner* gpInstance = 0;
static int gSystem_ready = 0;
static switch_core_flag_t gFlags = SCF_CALIBRATE_CLOCK | SCF_USE_CLOCK_RT;
static switch_memory_pool_t* gPool = 0;
switch_file_t* gPid = 0;
char gPidPath[PATH_MAX] = "";	/* full path to the pid file */

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

/* signal handler for when freeswitch is running in background mode.
 * signal triggers the shutdown of freeswitch
# */
static int check_fd(int fd, int ms)
{
	struct pollfd pfds[2] = { { 0 } };
	int s, r = 0, i = 0;

	pfds[0].fd = fd;
	pfds[0].events = POLLIN | POLLERR;
	s = poll(pfds, 1, ms);

	if (s == 0 || s == -1) {
		r = s;
	} else {
		r = -1;

		if ((pfds[0].revents & POLLIN)) {
			if ((i = read(fd, &r, sizeof(r))) > -1) {
				i = write(fd, &r, sizeof(r));
			}
		}
	}
	
	return r;
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
  _codecPreference(FS_DEFAULT_CODEC_PREFERENCE)
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
  if (apr_initialize() != SWITCH_STATUS_SUCCESS) 
  {
    std::cerr << "apr_initialize FAILURE" << std::endl;
    _exit(EXIT_FAILURE);
  }
   
  set_auto_priority();
  
  switch_core_setrlimits();
  
  switch_core_set_globals();
  
  char pid_buffer[32] = "";	/* pid string */
	char old_pid_buffer[32] = "";	/* pid string */
	switch_size_t pid_len, old_pid_len;
  
  pid_t pid = getpid();
  std::ostringstream pidFile;
  pidFile << _applicationName << ".pid";

	memset(pid_buffer, 0, sizeof(pid_buffer));
	switch_snprintf(gPidPath, sizeof(gPidPath), "%s%s%s", SWITCH_GLOBAL_dirs.run_dir, SWITCH_PATH_SEPARATOR, pidFile.str().c_str());
	switch_snprintf(pid_buffer, sizeof(pid_buffer), "%d", pid);
	pid_len = strlen(pid_buffer);

	apr_pool_create(&gPool, NULL);

	switch_dir_make_recursive(SWITCH_GLOBAL_dirs.run_dir, SWITCH_DEFAULT_DIR_PERMS, gPool);

	if (switch_file_open(&gPid, gPidPath, SWITCH_FOPEN_READ, SWITCH_FPROT_UREAD | SWITCH_FPROT_UWRITE, gPool) == SWITCH_STATUS_SUCCESS) {

		old_pid_len = sizeof(old_pid_buffer);
		switch_file_read(gPid, old_pid_buffer, &old_pid_len);
		switch_file_close(gPid);
	}

	if (switch_file_open(&gPid,
						 gPidPath,
						 SWITCH_FOPEN_WRITE | SWITCH_FOPEN_CREATE | SWITCH_FOPEN_TRUNCATE,
						 SWITCH_FPROT_UREAD | SWITCH_FPROT_UWRITE, gPool) != SWITCH_STATUS_SUCCESS) {
		fprintf(stderr, "Cannot open pid file %s.\n", gPidPath);
		_exit(EXIT_FAILURE);
	}

	if (switch_file_lock(gPid, SWITCH_FLOCK_EXCLUSIVE | SWITCH_FLOCK_NONBLOCK) != SWITCH_STATUS_SUCCESS) {
		fprintf(stderr, "Cannot lock pid file %s.\n", gPidPath);
		old_pid_len = strlen(old_pid_buffer);
		if (strlen(old_pid_buffer)) {
			switch_file_write(gPid, old_pid_buffer, &old_pid_len);
		}
		_exit(EXIT_FAILURE);
	}

	switch_file_write(gPid, pid_buffer, &pid_len);
  
  const char* err = 0;
  if (switch_core_init_and_modload(gFlags, noconsole ? SWITCH_FALSE : SWITCH_TRUE, &err) != SWITCH_STATUS_SUCCESS) 
  {
		std::cerr << "Unable to load modules. " << err << std::endl;
    _exit(EXIT_FAILURE);
	}
  
  switch_core_runtime_loop(noconsole);
  
  switch_core_destroy();
	switch_file_close(gPid);
	apr_pool_destroy(gPool);

	if (unlink(gPidPath) != 0) 
  {
		std::cerr << "Failed to delete pid file " << gPidPath << std::endl;
	}
}
  
void FreeSwitchRunner::stop()
{
  int32_t arg = 0;
	switch_core_session_ctl(SCSC_SHUTDOWN, &arg);
  if (_pSwitchThread)
    _pSwitchThread->join();
	return;
}

void FreeSwitchRunner::daemonize(int *fds)
{
	int fd;
	pid_t pid;
	unsigned int sanity = 60;

	if (!fds) {
		switch (fork()) {
		case 0:		/* child process */
			break;
		case -1:
			fprintf(stderr, "Error Backgrounding (fork)! %d - %s\n", errno, strerror(errno));
			exit(EXIT_SUCCESS);
			break;
		default:	/* parent process */
			exit(EXIT_SUCCESS);
		}

		if (setsid() < 0) {
			fprintf(stderr, "Error Backgrounding (setsid)! %d - %s\n", errno, strerror(errno));
			exit(EXIT_SUCCESS);
		}
	}

	pid = switch_fork();

	switch (pid) {
	case 0:		/* child process */
		if (fds) {
			close(fds[0]);
		}
		break;
	case -1:
		fprintf(stderr, "Error Backgrounding (fork2)! %d - %s\n", errno, strerror(errno));
		exit(EXIT_SUCCESS);
		break;
	default:	/* parent process */
		fprintf(stderr, "%d Backgrounding.\n", (int) pid);

		if (fds) {
			char *o;

			close(fds[1]);

			if ((o = getenv("FREESWITCH_BG_TIMEOUT"))) {
				int tmp = atoi(o);
				if (tmp > 0) {
					sanity = tmp;
				}
			}

			do {
				gSystem_ready = check_fd(fds[0], 2000);

				if (gSystem_ready == 0) {
					printf("FreeSWITCH[%d] Waiting for background process pid:%d to be ready.....\n", (int)getpid(), (int) pid);
				}

			} while (--sanity && gSystem_ready == 0);

			shutdown(fds[0], 2);
			close(fds[0]);
			fds[0] = -1;

			
			if (gSystem_ready < 0) {
				printf("FreeSWITCH[%d] Error starting system! pid:%d\n", (int)getpid(), (int) pid);
				kill(pid, 9);
				exit(EXIT_FAILURE);
			}

			printf("FreeSWITCH[%d] System Ready pid:%d\n", (int) getpid(), (int) pid);
		}

		exit(EXIT_SUCCESS);
	}

	if (fds) {
		setsid();
	}
	/* redirect std* to null */
	fd = open("/dev/null", O_RDONLY);
	if (fd != 0) {
		dup2(fd, 0);
		close(fd);
	}

	fd = open("/dev/null", O_WRONLY);
	if (fd != 1) {
		dup2(fd, 1);
		close(fd);
	}

	fd = open("/dev/null", O_WRONLY);
	if (fd != 2) {
		dup2(fd, 2);
		close(fd);
	}
	return;
}

bool FreeSwitchRunner::generateConfig()
{
  std::string eslPort = boost::lexical_cast<std::string>(_eslPort);
  string_replace(freeswitch_xml, "@ESL_PORT@", eslPort.c_str());
  
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



