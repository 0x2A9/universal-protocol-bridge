# Adapter
Firmware for an **STM32-based USB** adapter. The adapter bridges **USB (CDC)** communication with on-board peripherals such as ```UART```, ```I²C```, and ```SPI```devices

# Set Up
- Download a special ```Arm GNU Toolchain 15.3.Rel1 (Build arm-15.149)) 15.3.1 20260627``` for the **ARM** architecture
    ```bash
    sudo apt install gcc-arm-none-eabi gdb-arm-none-eabi
    ```
- In case ```apt``` contains an old version, use an official [web page](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Search for the ```x86_64 Linux hosted cross toolchains``` and ```AArch32 bare-metal target (arm-none-eabi)```
- Download ```.deb``` ```1.8.0``` release of the `stlink` [repository](https://github.com/stlink-org/stlink/releases) and install:
    ```bash
    sudo dpkg -i stlink_1.8.0-1_amd64.deb
    ```
- Check everything is installed correctly by running:
    ```bash
    arm-none-eabi-gcc --version
    arm-none-eabi-g++ --version
    st-info --version
    st-flash --version
    ```

# Build
## Basic
- To build the firmware (```.elf```, ```.bin```, and ```.hex``` files), run:
	```bash
	make
	```

- To build the firmware and flash it to the MCU, run:
	```bash
	make flash
	```
This will build the project using the **default** settings:
- ```MODE=DEBUG```
- ```OPT=0```
- First auto-discovered device under ```devices/*/*```

The output files will be placed in the ```build/``` directory.

## Selective
### 1. Mode
- ```DEBUG``` – debug symbols enabled, no optimizations
- ```RELEASE``` – no debug symbols, optimized build

    ```bash
    make MODE=RELEASE
    make flash MODE=DEBUG
    ```

### 2. Optimization Level
- ```0```, ```1```, ```2```, ```3``` – standard **GCC** optimization levels
- ```s``` – optimize for size (```-Os```)
    
    ```bash
    make OPT=3
    make flash OPT=s
    ```

### 3. Target Device
If multiple devices are present under the ```devices/``` directory, it is possible to select which one to build for:

```bash
make DEVICE=stm32f303vct6
make flash DEVICE=stm32f303vct6
```

## Clean
```bash
make clean
```