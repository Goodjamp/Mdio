:: The argument #1 must be path to the folder with qmake.exe file (without \)
:: The argument #2 must be path to the folder with mingw32-make.exe file (without \)

set QMakePath=%1
set CompilerPath=%2

cd ..\

git tag >  ToolsAndDocumentation\TagList.txt

cd ToolsAndDocumentation

python updateVersion.py TagList.txt

cd ..\..\

rmdir /S /Q Build

mkdir Build

cd Build

%QMakePath%\bin\qmake.exe ..\Mdio\Mdio.pro -spec win32-g++ "CONFIG+=qtquickcompiler" && %CompilerPath%\bin\mingw32-make.exe qmake_all

%CompilerPath%\bin\mingw32-make.exe -j8

cd ..

rmdir /S /Q MdioDelivery

xcopy Mdio\ToolsAndDocumentation\MdioDelivery MdioDelivery /S /E /Y /I

copy Build\release\Mdio.exe MdioDelivery\Mdio.exe

rmdir /S /Q Build

del /f  MdioDelivery*.zip

Mdio\ToolsAndDocumentation\7-Zip\7zG.exe a -tzip MdioDelivery.zip MdioDelivery
