REM This is required to make Nuget work with submodules...
REM We pass in the full path to SolutionDir on the commandline
SET SolutionDir=%1
SET SourceDir=%SolutionDir%packages
SET DestDir=%SolutionDir%krabsetw\krabs\packages

ECHO "Linking Solution Nuget Package Dir to Where KrabsETW expects it to be"
ECHO "This is to fix how Nuget (fails to) work with git Submodules"
if not exist %SourceDir% mkdir %SourceDir%
if exist %DestDir% rmdir %DestDir%
mklink /j %DestDir% %SourceDir%
