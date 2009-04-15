/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ***********************************************************************
 *
 * 
 *
 * SB Configuration File Parser - implementation
 *
 **********************************************************************
 */

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#include <iostream>
using namespace std;

#include "ConfigFile.h"          // Header for these functions
#include <cstdio>
#include <string>
#include <cstring>


#define READ_BUFFER_SIZE 2048

extern "C"
VXIplatformResult ParseConfigLine(char* buffer, VXIMap *configArgs)
{
    char *end;
    const char *configKey, *configType, *configValue;

    /* Skip blank lines or those starting with comments */
    if((buffer[0] == '\0') || (buffer[0] == CONFIG_FILE_COMMENT_ID) ||
       (strspn(buffer, CONFIG_FILE_SEPARATORS) == strlen(buffer))) {
        return VXIplatform_RESULT_SUCCESS;
    }

    /* Get the configuration key */
    configKey = buffer;
    if(strchr(CONFIG_FILE_SEPARATORS, configKey[0]) != NULL) {
       return VXIplatform_RESULT_FAILURE;
    }
    end = strpbrk(configKey, CONFIG_FILE_SEPARATORS);
    if(end == NULL) {
        return VXIplatform_RESULT_FAILURE;
    }
    *end = '\0'; /* terminate configKey */

    /* Get the configuration type */
    configType = end + 1;
    configType = &configType[strspn(configType, CONFIG_FILE_SEPARATORS)];
    if(configType[0] == '\0') {
        return VXIplatform_RESULT_FAILURE;
    }
    end = strpbrk(configType, CONFIG_FILE_SEPARATORS);
    if(end == NULL) {
        return VXIplatform_RESULT_FAILURE;
    }
    *end = '\0'; /* terminate configType */

    /* Get the configuration value, stripping trailing whitespace */
    configValue = end + 1;
    configValue = &configValue[strspn(configValue, CONFIG_FILE_SEPARATORS)];
    if(configValue[0] == '\0') {
        return VXIplatform_RESULT_FAILURE;
    }
    end = strchr(configValue, '\0');
    while((end > configValue) &&
	  (strchr(CONFIG_FILE_SEPARATORS, *(end - 1)) != NULL))
      end--;
    if(end == configValue) {
        return VXIplatform_RESULT_FAILURE;
    }
    *end = '\0'; /* terminate configValue */

    string expStr("");
    if(configValue != NULL) {
        // Resolve environment variables, if any
        const char *envVarBgn, *envVarEnd;
        const VXIint lenBgn = strlen(CONFIG_FILE_ENV_VAR_BEGIN_ID);
        const VXIint lenEnd = strlen(CONFIG_FILE_ENV_VAR_END_ID);
        const char *ptrValue = configValue;

        while((envVarBgn = strstr(ptrValue, CONFIG_FILE_ENV_VAR_BEGIN_ID)) !=
	      NULL) {

	    if (envVarBgn != ptrValue)
	      expStr.assign (ptrValue, envVarBgn - ptrValue);
	    envVarBgn += lenBgn;
	    if(envVarBgn[0] == 0)
                return VXIplatform_RESULT_FAILURE;

            envVarEnd = strstr(envVarBgn, CONFIG_FILE_ENV_VAR_END_ID);
            if((envVarEnd == NULL) || (envVarEnd == envVarBgn))
                return VXIplatform_RESULT_FAILURE;

            string envStr(envVarBgn, envVarEnd - envVarBgn);
            const char *envVarValue = getenv(envStr.c_str());
            if((envVarValue == NULL) || (envVarValue[0] == 0)) {
	        fprintf (stderr,"ERROR: Environment variable not set, %s\n",
			 envStr.c_str( ));
                return VXIplatform_RESULT_FAILURE;
	    }

            expStr += envVarValue;
            ptrValue = envVarEnd + lenEnd;
        }

        expStr += ptrValue;
        configValue = expStr.c_str();
    }

    VXIint keyLen = strlen(configKey) + 1;
    VXIchar *wConfigKey = new VXIchar [keyLen];
    if(wConfigKey == NULL) {
        return VXIplatform_RESULT_OUT_OF_MEMORY;
    }
    mbstowcs(wConfigKey, configKey, keyLen);

    if(strcmp(configType, "Environment") == 0) { // Set environment variable
        int cfgValLen = configValue ? strlen(configValue) : 0;
        char *envVar = new char [strlen(configKey) + cfgValLen + 2];
        if(envVar == NULL) {
            delete [] wConfigKey;
            wConfigKey = NULL;
            return VXIplatform_RESULT_OUT_OF_MEMORY;
        }
#ifdef WIN32
        sprintf(envVar, "%s=%s", configKey, configValue ? configValue : "");
        _putenv(envVar);
#else
        if(configValue)
            sprintf(envVar, "%s=%s", configKey, configValue);
        else
            strcpy(envVar, configKey); // Remove variable from environment
        putenv(envVar);
#endif
        // 'envVar' must NOT be freed !!!
    }
    else if(strcmp(configType, "VXIPtr") == 0) {
        // Only NULL pointers are supported
        VXIMapSetProperty(configArgs, wConfigKey,
                          (VXIValue *)VXIPtrCreate(NULL));
    }
    else {
        if(configValue == NULL) {
            delete [] wConfigKey;
            wConfigKey = NULL;
            return VXIplatform_RESULT_FAILURE;
        }

        if(strcmp(configType, "VXIInteger") == 0) {
            VXIint32 intValue = (VXIint32)atoi(configValue);
            if((intValue == 0) && (configValue[0] != '0')) {
                delete [] wConfigKey;
                wConfigKey = NULL;
                return VXIplatform_RESULT_FAILURE;
            }
            VXIMapSetProperty(configArgs, wConfigKey, 
                              (VXIValue *)VXIIntegerCreate(intValue));
    }
        else if(strcmp(configType, "VXIFloat") == 0) {
            VXIflt32 fltValue = (VXIflt32)atof(configValue);
             if((fltValue == 0.0) && 
                ((configValue[0] != '0') || (configValue[0] != '.'))) {
                delete [] wConfigKey;
                wConfigKey = NULL;
                return VXIplatform_RESULT_FAILURE;
            }
            VXIMapSetProperty(configArgs, wConfigKey, 
                              (VXIValue *)VXIFloatCreate(fltValue));
        }
        else if(strcmp(configType, "VXIString") == 0) {
            VXIint valLen = strlen(configValue) + 1;
            VXIchar *wConfigValue = new VXIchar [valLen];
            if(wConfigValue == NULL) {
                delete [] wConfigKey;
                wConfigKey = NULL;
                return VXIplatform_RESULT_OUT_OF_MEMORY;
            }
            mbstowcs(wConfigValue, configValue, valLen);

            VXIMapSetProperty(configArgs, wConfigKey, 
                              (VXIValue *)VXIStringCreate(wConfigValue));
            delete [] wConfigValue;
            wConfigValue = NULL;
        }
        else {
            delete [] wConfigKey;
            wConfigKey = NULL;
            return VXIplatform_RESULT_UNSUPPORTED;
        }
    }

    delete [] wConfigKey;
    wConfigKey = NULL;
    return VXIplatform_RESULT_SUCCESS;
}


extern "C"
VXIplatformResult ParseConfigFile (VXIMap **configArgs, const char *fileName)
{
    VXIplatformResult platformResult = VXIplatform_RESULT_SUCCESS;
    char buffer[READ_BUFFER_SIZE];
    VXIint lineNum = 0;
    char *readResult;

    if((configArgs == NULL) || (fileName == NULL)) {
        return VXIplatform_RESULT_INVALID_ARGUMENT;
    }

    *configArgs = VXIMapCreate();
    if(*configArgs == NULL) {
        cerr << "ERROR: Not enough memory to create config map" << endl;
        return VXIplatform_RESULT_OUT_OF_MEMORY;
    }

    FILE *configFile = fopen(fileName, "r");
    if(configFile == NULL) {
        cerr << "ERROR: Cannot open file '" << fileName << "'" << endl;
        return VXIplatform_RESULT_SYSTEM_ERROR;
    }

    readResult = fgets(buffer, READ_BUFFER_SIZE, configFile);

    while(readResult != NULL) {
        lineNum++;
        platformResult = ParseConfigLine(buffer, *configArgs);

        if(platformResult == VXIplatform_RESULT_FAILURE) {
            cerr << "WARNING: Syntax error on line " << lineNum
                 << " in file '" << fileName << "'" << endl;
            platformResult = VXIplatform_RESULT_SUCCESS;
        }
        else if(platformResult == VXIplatform_RESULT_UNSUPPORTED) {
            cerr << "WARNING: Unsupported data on line " << lineNum
                 << " in file '" << fileName << "'" << endl;
            platformResult = VXIplatform_RESULT_SUCCESS;
        }
        else if(platformResult != VXIplatform_RESULT_SUCCESS) {
            break;
        }

        readResult = fgets(buffer, READ_BUFFER_SIZE, configFile);
    }

    if((readResult == NULL) && !feof(configFile)) {
        cerr << "ERROR: Failed reading file '" << fileName << "'" << endl;
        platformResult = VXIplatform_RESULT_SYSTEM_ERROR;
    }

    fclose(configFile);
    return platformResult;
}

