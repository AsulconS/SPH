@ECHO off

SET CXX=cl
SET CXX_FLAGS=/EHsc
SET INCLUDE_PATH=/I ".\include" /I ".\include\HSGIL\external"

SET HSGIL_LIBS=hsgil-core.lib hsgil-math.lib hsgil-window.lib hsgil-graphics.lib

%CXX% %CXX_FLAGS% %INCLUDE_PATH% /D HSGIL_STATIC_BUILD /c old.cpp
%CXX% %CXX_FLAGS% %INCLUDE_PATH% /D HSGIL_STATIC_BUILD /c main.cpp
%CXX% %CXX_FLAGS% %INCLUDE_PATH% /D HSGIL_STATIC_BUILD /c fluid.cpp

%CXX% %CXX_FLAGS% old.obj hsgil-core.lib hsgil-math.lib hsgil-window.lib hsgil-graphics.lib /link /out:old.exe
%CXX% %CXX_FLAGS% main.obj hsgil-core.lib hsgil-math.lib hsgil-window.lib hsgil-graphics.lib /link /out:main.exe
%CXX% %CXX_FLAGS% fluid.obj hsgil-core.lib hsgil-math.lib hsgil-window.lib hsgil-graphics.lib /link /out:fluid.exe

clean.bat
