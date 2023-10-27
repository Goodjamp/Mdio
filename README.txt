The *МВВ-4-2-конфігуратор* software is use to configure *МВВ-4-2* module

To make a building for the deployment of the new version of SW the DeployDelivery.bat could be used.
Required toolchain:
- Qt (qmake.exe and mingw32-make.exe (mingw81_32))
- git
- python version 3, regexp module

The  DeployDelivery.bat has a next signature:
DeployDelivery.bat arg1 arg2
arg1 - path to the folder with *qmake.exe* (without \) . Typically the qmake.exe is placed on the: C:\Qt\Qt_version\mingw81_32\bin
arg1 - path to the folder with *mingw32-make.exe* (without \). Typically the *mingw32-make.exe* on the: C:\Qt\Tools\mingw810_32\bin

The last git tag is used to create the SW version. The last tag is incremented in the building process and added as a new tag.

To make a build take the next steps:
#update tags list from the repositories
git fetch --tags
#run builder
ToolsAndDocumentation\DeployDelivery.bat C:\Qt\Qt_version\mingw81_32\bin C:\Qt\Tools\mingw810_32\bin
#push the new tag to repositories
git push --tags