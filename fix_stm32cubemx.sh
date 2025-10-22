cp -a Utilities.bak Utilities
for i in main fmc gpio ltdc sai usart
do
    mv Src/${i}.c{,.template}
    mv Inc/${i}.h{,.template}
done
cp Src/main.c{.bak,}
cp Inc/main.h{.bak,}
rm Middlewares/Third_Party/ARM_CMSIS/Source/*/*.c
rm Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_camera.*
rm Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_qspi.c*

