
# UBP UI
A **Qt6-based desktop application** using Widgets and SerialPort modules. It supports building and running both via **Qt Creator** and from the **console** with CMake.

---

# Set Up

## 1. Install Qt on Linux
```bash
wget https://download.qt.io/official_releases/online_installers/qt-online-installer-linux-x64-online.run
chmod +x qt-online-installer-linux-x64-online.run
./qt-online-installer-linux-x64-online.run
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
sudo apt install libxcb-cursor0 libxcb-cursor-dev
```

- During installation, select **Qt 6.x** and **Qt Creator**.
- Ensure the **Desktop GCC kit** is installed.

```bash
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
```

## 2. Install Build Tools
```bash
sudo apt-get install build-essential cmake
```

- `build-essential` provides gcc, g++, make.
- `cmake` is required for building from the console.

---

# Build

## 1. Using Qt Creator
1. Open the project folder in **Qt Creator**
    ```bash
    ~/Qt/Tools/QtCreator/bin/qtcreator <project-dir>/universal-protocol-bridge/desktop
    ```
    To run QT on **Wayland** export additional env variable:
    ```bash
    export QT_QPA_PLATFORM=wayland 
    ```
2. Qt Creator may suggest to install Qt6SerialPort or other missing packages. Accept it.
3. Open `CMakeLists.txt`.
4. Select your **Qt Kit** (Desktop Qt 6.x GCC).
5. Click **Configure Project**. Qt Creator may automatically offer to configure the selected project immediately after its launching, so no additional configuration step is required here.
6. Press **Run** to build and launch the application.

---

## 2. Using Console (CMake)

### Step 1: Create Build Directory
```bash
cd <project-dir>/universal-protocol-bridge/desktop
mkdir -p ./build/Desktop_Qt_6_10_1-Release
cd ./build/Desktop_Qt_6_10_1-Release
```

### Step 2: Configure Project
Replace `/home/username/Qt/6.10.1/gcc_64` with your Qt installation path:

```bash
cmake -DCMAKE_PREFIX_PATH=/home/username/Qt/6.10.1/gcc_64 -DCMAKE_BUILD_TYPE=Release ../..
cd ../..
```

### Step 3: Build
```bash
cmake --build ./build/Desktop_Qt_6_10_1-Release --parallel $(nproc)
```

- The executable will be created at:
`./build/Desktop_Qt_6_10_1-Release/desktop`

### Step 4: Run
```bash
sudo ./build/Desktop_Qt_6_10_1-Release/desktop
```

### Optional: Clean Build
```bash
rm -rf ./build/Desktop_Qt_6_10_1-Release
```
