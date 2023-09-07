cd ..\

git tag >  ToolsAndDocumentation\TagList.txt

cd ToolsAndDocumentation
python updateVersion.py TagList.txt

cd ..\..\

rmdir /S /Q Build

mkdir Build

cd Build

C:\Qt\5.15.0\mingw81_32\bin\qmake.exe ..\Mdio\Mdio.pro -spec win32-g++ "CONFIG+=qtquickcompiler" && C:/Qt/Tools/mingw810_32/bin/mingw32-make.exe qmake_all

C:\Qt\Tools\mingw810_32\bin\mingw32-make.exe -j8

cd ..

rmdir /S /Q MdioDelivery

xcopy Mdio\ToolsAndDocumentation\MdioDelivery MdioDelivery /S /E /Y /I

copy Build\release\Mdio.exe MdioDelivery\Mdio.exe

rmdir /S /Q Build

del /f  MdioDelivery*.zip

Mdio\ToolsAndDocumentation\7-Zip\7zG.exe a -tzip MdioDelivery.zip MdioDelivery
