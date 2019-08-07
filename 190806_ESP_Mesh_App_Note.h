//------------------------------------------------------------------------------------
// ESP32 MESH Development Framework App note
//------------------------------------------------------------------------------------
// Download Toolchain
https://dl.espressif.com/dl/esp32_win32_msys2_environment_and_toolchain-20180110.zip
// Document at: https://github.com/espressif/esp-mdf
// Unzip the zip file to C:\ (or some other location, but this guide assumes C:\) 
// and it will create an msys32 directory with a pre-prepared environment.
/*
Open a MSYS2 MINGW32 terminal window by running C:\msys32\mingw32.exe. 
The environment in this window is a bash shell. 
Create a directory named esp that is a default location to develop ESP32 applications. 
To do so, run the following shell command:
*/
mkdir -p ~/esp
cd ~/esp
// Get MDF
// Run file mingw32.exe in C:\msys32, then run command:  
git clone --recursive https://github.com/espressif/esp-mdf.git
// Export environment PATH 
export IDF_PATH=~/esp/esp-idf
export MDF_PATH=~/esp/esp-mdf
// Open file export_idf_path.sh in C:\msys32\etc\profile.d then at 2 line bellow into this file.
export IDF_PATH="C:/msys32/home/Administrator/esp/esp-idf"
export MDF_PATH="C:/msys32/home/Administrator/esp/esp-mdf"
// Check PATH 
printenv IDF_PATH
printenv MDF_PATH
// Start project
cp -r $MDF_PATH/examples/get-started/ .
cd  get-started/
// Build and flash
make menuconfig
make erase_flash flash
// Monitor and debug
make monitor
// Update ESP-MDF:
cd ~/esp/esp-mdf
git pull
git submodule update --init --recursive
// How to navigate to project folder
cd D:\\13_Mekong\\11_C.P_VUNG_TAU\\ESP32_Mesh_C.P_Vung_Tau\\190724_CP_SENSOR_NODE_v11
make menuconfig
make erase_flash flash
