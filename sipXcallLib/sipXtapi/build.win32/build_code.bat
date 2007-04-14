@ECHO OFF

REM  
REM  Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
REM  Contributors retain copyright to elements licensed under a Contributor Agreement.
REM  Licensed to the User under the LGPL license.
REM  
REM

ECHO ??? Setting MSVC Vars...
IF NOT "%SIPXPHONE_MSVC_BASE%" == "" GOTO SET_MSVC_VARS
SET SIPXPHONE_MSVC_BASE=C:\Program Files\Microsoft Visual Studio\VC98

:SET_MSVC_VARS
call "%SIPXPHONE_MSVC_BASE%\bin\VCVARS32.bat"

:BUILD_PROJECTS
ECHO ???? Building Project...
call _build_cpp_project sipXtapi %1
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
ECHO .

:DONE
