@ECHO OFF

REM  
REM  Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
REM  Contributors retain copyright to elements licensed under a Contributor Agreement.
REM  Licensed to the User under the LGPL license.
REM  
REM

SET PROJECT_NAME=%1

ECHO .
ECHO PROJECT_NAME: %PROJECT_NAME%
ECHO .

pushd .

:BUILD
ECHO ??? Building...
cd ..\..

msdev %PROJECT_NAME%.dsp /MAKE %1 ALL
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
popd .
EXIT /B 1

:DONE
popd.
EXIT /B 0
	
