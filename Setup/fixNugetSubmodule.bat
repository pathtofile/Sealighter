REM This is required to make Nuget work with submodules...
REM We pass in the full path to SolutionDir on the commandline
SET SolutionDir=%1
SET sourceDir=%SolutionDir%packages
SET destDir=%SolutionDir%krabsetw\krabs\packages
SET FAKEFILE=%SolutionDir%..\35MSSharedLib1024.snk

if not exist %sourceDir% mkdir %sourceDir%
if not exist %destDir% mklink /j %destDir% %sourceDir%
ECHO "" > %FAKEFILE%