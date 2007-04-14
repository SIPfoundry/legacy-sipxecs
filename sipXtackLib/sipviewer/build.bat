@REM Needs ANT_HOME and JAVA_HOME to be set

@IF "%1"=="clean" GOTO CLEAN
@IF NOT "%1"=="install" GOTO BUILD
@echo "Generating batch file .."
@mkdir lib
@mkdir lib\sipviewer
@echo java -cp .\jdom.jar;.\sipviewer.jar com.pingtel.sipviewer.SIPViewer %%1 %%2 %%3 %%4 %%5 %%6 %%7 %%8 %%9 >lib\sipviewer\sipviewer.bat

:BUILD  
@%ANT_HOME%/bin/ant %1 -Djdom.jar=..\lib\jdom-b10.jar -Dlib.dir=lib
@GOTO END

:CLEAN
@del /q dist\*
@del /q lib\sipviewer\*
@rmdir dist\api
@rmdir dist
@rmdir lib\sipviewer
@rmdir lib
@echo "Clean finished"

:END

