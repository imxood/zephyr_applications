# zephyr RTOS 开发

### 初始化

    cd <...>/zephyrproject

    git clone https://github.com/imxood/zephyr_develop.git

### 编译 stm32f103c8

Blue pill 开发板

    cd zephyr_develop/apps/f103c8_app

    west build -b stm32f103c8 -- -DBOARD_ROOT=../.. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    west build
    west flash

### 编译 stm32f767ig

正点原子的开发板

    cd zephyr_develop/apps/f767ig_app

    west build -b stm32f767ig -- -DBOARD_ROOT=../.. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    west build
    west flash
