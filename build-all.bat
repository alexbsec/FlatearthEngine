@echo on

set binDir=.\bin

if not exist %binDir% (
    echo Making bin directory...
    mkdir %binDir%
) else (
    echo Bin directory exists!
)

pushd engine
call .\run.bat
popd

pushd testsuite
call .\run.bat
popd

pause