cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME dxft8)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# BSP include paths
set(BSP_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/STM32746G-Discovery
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Components/wm8994
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Components/ft5336
)

# Add BSP includes to stm32cubemx interface
target_include_directories(stm32cubemx INTERFACE
    ${BSP_Include_Dirs}
    ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Fonts
)

# STM32746G-Discovery BSP sources
set(STM32746G-Discovery_BSP_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_audio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_lcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_sdram.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_ts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Components/wm8994/wm8994.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/BSP/Components/ft5336/ft5336.c
)

# Create STM32746G-Discovery BSP static library
add_library(STM32746G-Discovery_BSP OBJECT)
target_sources(STM32746G-Discovery_BSP PRIVATE ${STM32746G-Discovery_BSP_Src})
target_include_directories(STM32746G-Discovery_BSP PUBLIC ${BSP_Include_Dirs})
target_link_libraries(STM32746G-Discovery_BSP PUBLIC stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Performance-critical DSP files that must be optimized
set(DSP_CRITICAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Process_DSP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/SDR_Audio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/FIR_Coefficients.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/decode.c
)

# Set optimization flags for DSP-critical files
set_source_files_properties(${DSP_CRITICAL_FILES}
    PROPERTIES
    COMPILE_OPTIONS "-Os;-ffunction-sections;-fdata-sections"
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/ADIF.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Codec_Gains.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/DS3231.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Display.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/FIR_Coefficients.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Ini.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Process_DSP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/SDR_Audio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/SiLabs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/autoseq_engine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/button.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/decode_ft8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/gen_ft8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/log_file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/options.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/qso_display.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/traffic_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/constants.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/ldpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/pack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/text.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library/unpack.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/FT8_library
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    STM32746G-Discovery_BSP

    # Add user defined libraries
)

# Generate bin file
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating .bin file"
)
