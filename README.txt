The *МВВ-4-2-конфігуратор* software is use to configure *МВВ-4-2* module

The special algorithm of deployment was designed.
Toolchain:
- Qt (qmake.exe and mingw32-make.exe (mingw81_32))
- git
- python versin 3, regexp module

Typicaly the qmake.exe placed by the next path: C:\Qt\Qt_version\mingw81_32\bin
Typicaly the mingw32-make.exe placed by the next path: C:\Qt\Qt_version\mingw81_32\bin OR C:\Qt\Tools\mingw810_32\bin

The SW version is based on the last tag. The last tag is incremented on the process of deployment building and added as a new tag.
To make a deployment build make a next steps:

#update tags list from the repositorium
git fetch --tags

#run builder
ToolsAndDocumentation\DeployDelivery.bat C:\Path\to\the\folder\with\*qmake.exe* file (without \)  C:\Path\to\the\folder\with\*mingw32-make.exe* file (without \)

#push new tag to repositorium
git push --tags