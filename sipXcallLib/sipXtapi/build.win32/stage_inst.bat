@ECHO OFF

REM  
REM  Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
REM  Contributors retain copyright to elements licensed under a Contributor Agreement.
REM  Licensed to the User under the LGPL license.
REM  
REM

SET SOURCE_BASE=".."

:PREP_STAGING
  rmdir /S /Q staging 2>NUL
  
  mkdir staging
  mkdir staging\bin
  mkdir staging\lib
  mkdir staging\include
  mkdir staging\doc

:POPULATE_STAGING
  copy %SOURCE_BASE%\Release\sipXtapi.dll .\staging\bin\
  copy %SOURCE_BASE%\Debug\sipXtapid.dll .\staging\bin\
  copy %SOURCE_BASE%\Release\sipXtapi.lib .\staging\lib\
  copy %SOURCE_BASE%\Debug\sipXtapid.lib .\staging\lib\

  copy %SOURCE_BASE%\..\examples\PlaceCall\Release\PlaceCall.exe .\staging\bin\
  copy %SOURCE_BASE%\..\examples\ReceiveCall\Release\ReceiveCall.exe .\staging\bin\

  copy %SOURCE_BASE%\Release\sipXtapiTest.exe .\staging\bin

  copy %SOURCE_BASE%\..\doc\sipXtapi\html\* .\staging\doc\
  
  copy %SOURCE_BASE%\..\include\tapi\sipXtapi.h .\staging\include\
  copy %SOURCE_BASE%\..\include\tapi\sipXtapiEvents.h .\staging\include\
  GOTO EXIT


:ERROR_JRE
  ECHO Unable to find JREin %SIPXPHONE_JRE_BASE%, please set SIPXPHONE_JRE_BASE
  GOTO EXIT

:EXIT
