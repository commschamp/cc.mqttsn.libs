rem Input
rem BUILD_DIR - Main build directory
rem GENERATOR - CMake generator
rem PLATFORM - CMake generator platform
rem QTDIR - Path to Qt installation
rem EXTERNALS_DIR - (Optional) Directory where externals need to be located
rem COMMS_REPO - (Optional) Repository of the COMMS library
rem COMMS_TAG - (Optional) Tag of the COMMS library
rem CC_MQTTSN_REPO - (Optional) Repository of the cc.mqttsn.generated
rem CC_MQTTSN_TAG - (Optional) Tag of the  cc.mqttsn.generated
rem CC_MQTT311_REPO - (Optional) Repository of the cc.mqtt311.generated
rem CC_MQTT311_TAG - (Optional) Tag of the  cc.mqtt311.generated
rem COMMON_INSTALL_DIR - (Optional) Common directory to perform installations
rem COMMON_BUILD_TYPE - (Optional) CMake build type
rem COMMON_CXX_STANDARD - (Optional) CMake C++ standard

rem -----------------------------------------------------

if [%BUILD_DIR%] == [] echo "BUILD_DIR hasn't been specified" & exit /b 1

if [%GENERATOR%] == [] set GENERATOR="NMake Makefiles"

if NOT [%PLATFORM%] == [] set PLATFORM_PARAM=-A %PLATFORM%

if [%EXTERNALS_DIR%] == [] set EXTERNALS_DIR=%BUILD_DIR%/externals

if [%COMMS_REPO%] == [] set COMMS_REPO="https://github.com/commschamp/comms.git"

if [%COMMS_TAG%] == [] set COMMS_TAG="master"

if [%CC_MQTTSN_REPO%] == [] set CC_MQTTSN_REPO="https://github.com/commschamp/cc.mqttsn.generated.git"

if [%CC_MQTTSN_TAG%] == [] set CC_MQTTSN_TAG="master"

if [%CC_MQTT311_REPO%] == [] set CC_MQTT311_REPO="https://github.com/commschamp/cc.mqtt311.generated.git"

if [%CC_MQTT311_TAG%] == [] set CC_MQTT311_TAG="master"

if [%COMMON_BUILD_TYPE%] == [] set COMMON_BUILD_TYPE=Debug

set COMMS_SRC_DIR=%EXTERNALS_DIR%/comms
set COMMS_BUILD_DIR=%BUILD_DIR%/externals/comms/build
set COMMS_INSTALL_DIR=%COMMS_BUILD_DIR%/install
if NOT [%COMMON_INSTALL_DIR%] == [] set COMMS_INSTALL_DIR=%COMMON_INSTALL_DIR%

set CC_MQTTSN_SRC_DIR=%EXTERNALS_DIR%/cc.mqttsn.generated
set CC_MQTTSN_BUILD_DIR=%BUILD_DIR%/externals/cc.mqttsn.generated/build
set CC_MQTTSN_INSTALL_DIR=%COMMSDSL_BUILD_DIR%/install
if NOT [%COMMON_INSTALL_DIR%] == [] set CC_MQTTSN_INSTALL_DIR=%COMMON_INSTALL_DIR%

set CC_MQTT311_SRC_DIR=%EXTERNALS_DIR%/cc.mqtt311.generated
set CC_MQTT311_BUILD_DIR=%BUILD_DIR%/externals/cc.mqtt311.generated/build
set CC_MQTT311_INSTALL_DIR=%COMMSDSL_BUILD_DIR%/install
if NOT [%COMMON_INSTALL_DIR%] == [] set CC_MQTT311_INSTALL_DIR=%COMMON_INSTALL_DIR%

rem ----------------------------------------------------

mkdir "%EXTERNALS_DIR%"
if exist %COMMS_SRC_DIR%/.git (
    echo "Updating COMMS library..."
    cd "%COMMS_SRC_DIR%"
    git fetch --all
    git checkout .    
    git checkout %COMMS_TAG%
    git pull --all
    if %errorlevel% neq 0 exit /b %errorlevel%    
) else (
    echo "Cloning COMMS library..."
    git clone -b %COMMS_TAG% %COMMS_REPO% %COMMS_SRC_DIR%
    if %errorlevel% neq 0 exit /b %errorlevel%
)

echo "Building COMMS library..."
mkdir "%COMMS_BUILD_DIR%"
cd %COMMS_BUILD_DIR%
cmake -G %GENERATOR% %PLATFORM_PARAM% -S %COMMS_SRC_DIR% -B %COMMS_BUILD_DIR% ^
    -DCMAKE_INSTALL_PREFIX=%COMMS_INSTALL_DIR% -DCMAKE_BUILD_TYPE=%COMMON_BUILD_TYPE% ^
    -DCMAKE_CXX_STANDARD=%COMMON_CXX_STANDARD%
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build %COMMS_BUILD_DIR% --config %COMMON_BUILD_TYPE% --target install
if %errorlevel% neq 0 exit /b %errorlevel%

rem ----------------------------------------------------

if exist %CC_MQTTSN_SRC_DIR%/.git (
    echo "Updating cc.mqttsn.generated library..."
    cd "%CC_MQTTSN_SRC_DIR%"
    git fetch --all
    git checkout .    
    git checkout %CC_MQTTSN_TAG%
    git pull --all
    if %errorlevel% neq 0 exit /b %errorlevel%    
) else (
    echo "Cloning cc.mqttsn.generated library..."
    git clone -b %CC_MQTTSN_TAG% %CC_MQTTSN_REPO% %CC_MQTTSN_SRC_DIR%
    if %errorlevel% neq 0 exit /b %errorlevel%
)

echo "Building cc.mqttsn.generated library..."
mkdir "%CC_MQTTSN_BUILD_DIR%"
cd %CC_MQTTSN_BUILD_DIR%
cmake -G %GENERATOR% %PLATFORM_PARAM% -S %CC_MQTTSN_SRC_DIR% -B %CC_MQTTSN_BUILD_DIR% ^
    -DCMAKE_INSTALL_PREFIX=%CC_MQTTSN_INSTALL_DIR% -DCMAKE_BUILD_TYPE=%COMMON_BUILD_TYPE% ^
    -DCMAKE_CXX_STANDARD=%COMMON_CXX_STANDARD% -DOPT_REQUIRE_COMMS_LIB=OFF
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build %CC_MQTTSN_BUILD_DIR% --config %COMMON_BUILD_TYPE% --target install
if %errorlevel% neq 0 exit /b %errorlevel%

rem ----------------------------------------------------

if exist %CC_MQTT311_SRC_DIR%/.git (
    echo "Updating cc.mqtt311.generated library..."
    cd "%CC_MQTT311_SRC_DIR%"
    git fetch --all
    git checkout .    
    git checkout %CC_MQTT311_TAG%
    git pull --all
    if %errorlevel% neq 0 exit /b %errorlevel%    
) else (
    echo "Cloning cc.mqtt311.generated library..."
    git clone -b %CC_MQTT311_TAG% %CC_MQTT311_REPO% %CC_MQTT311_SRC_DIR%
    if %errorlevel% neq 0 exit /b %errorlevel%
)

echo "Building cc.mqtt311.generated library..."
mkdir "%CC_MQTT311_BUILD_DIR%"
cd %CC_MQTT311_BUILD_DIR%
cmake -G %GENERATOR% %PLATFORM_PARAM% -S %CC_MQTT311_SRC_DIR% -B %CC_MQTT311_BUILD_DIR% ^
    -DCMAKE_INSTALL_PREFIX=%CC_MQTT311_INSTALL_DIR% -DCMAKE_BUILD_TYPE=%COMMON_BUILD_TYPE% ^
    -DCMAKE_CXX_STANDARD=%COMMON_CXX_STANDARD% -DOPT_REQUIRE_COMMS_LIB=OFF
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build %CC_MQTT311_BUILD_DIR% --config %COMMON_BUILD_TYPE% --target install
if %errorlevel% neq 0 exit /b %errorlevel%

