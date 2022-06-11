cmake_minimum_required(VERSION 3.6)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_SYSTEM_NAME Generic)

##########
# tool chain
##########
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_EXE_LINKER   arm-none-eabi-g++)
set(OBJCOPY            arm-none-eabi-objcopy)
set(SIZE               arm-none-eabi-size)

##########
# architecture
##########
set(CPU       cortex-m4)
set(CHIP      STM32F405)
set(LDSCRIPT  STM32F405RGTx_FLASH.ld)
set(FPU       -mfpu=fpv4-sp-d16)
set(FLOAT_ABI -mfloat-abi=hard)

##########
# project name
##########
project(f405_LegoDriver)

set(ELF_FILE ${PROJECT_NAME}.elf)
set(HEX_FILE ${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_NAME}.bin)
set(MAP_FILE ${PROJECT_NAME}.map)

##########
# compiler options
##########
add_compile_options(-O2)
add_compile_options(-Wall)
add_compile_options(-ffunction-sections)
add_compile_options(-fdata-sections)
add_compile_options(-fstack-usage)

set(CMAKE_C_FLAGS "-mcpu=${CPU} -std=gnu11 ${FPU} ${FLOAT_ABI} -mthumb")
set(CMAKE_CXX_FLAGS "-mcpu=${CPU} -std=gnu++14 -fno-use-cxa-atexit ${FPU} ${FLOAT_ABI} -mthumb")
set(CMAKE_AS_FLAGS "-mcpu=${CPU} -x assembler-with-cpp ${FPU} ${FLOAT_ABI} -mthumb")
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=${CPU} -T${CMAKE_CURRENT_SOURCE_DIR}/${LDSCRIPT} --specs=nosys.specs -Wl,-Map=${MAP_FILE} -Wl,--gc-sections -static ${FPU} ${FLOAT_ABI} -mthumb -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group")

enable_language(ASM)

##########
# defines
##########
add_definitions(-D${CHIP}xx)
add_definitions(-DUSE_FULL_LL_DRIVER)
add_definitions(-DUSE_HAL_DRIVER)

##########
# directory name
##########
set(ROOT   .)
set(CORE   ${ROOT}/Core)
set(USER   ${ROOT}/User)
set(DRIVER ${ROOT}/Drivers/STM32F4xx_HAL_Driver)
set(CMSIS  ${ROOT}/Drivers/CMSIS)
set(RTOS   ${ROOT}/Middlewares/Third_Party/FreeRTOS/Source)
set(USB    ${ROOT}/USB_DEVICE)
set(USBD   ${ROOT}/Middlewares/ST/STM32_USB_Device_Library)

##########
# header include path
##########
include_directories(
	${CORE}/Inc
	${USER}
	${DRIVER}/Inc
	${CMSIS}/Device/ST/STM32F4xx/Include
	${CMSIS}/Include
	${RTOS}/include
	${RTOS}/CMSIS_RTOS
	${RTOS}/portable/GCC/ARM_CM4F
	${USB}/App
	${USB}/Target
	${USBD}/Class/CDC/Inc
	${USBD}/Core/Inc
)

##########
# source files
##########
file(GLOB SRCS
	${USER}/*.cpp
	${USER}/**/*.cpp
	${CORE}/Src/main.c
	${CORE}/Src/freertos.c
	${CORE}/Src/stm32f4xx_it.c
	${CORE}/Src/stm32f4xx_hal_msp.c
	${CORE}/Src/stm32f4xx_hal_timebase_tim.c
	${CORE}/Src/system_stm32f4xx.c
	${ROOT}/startup_stm32f405xx.s
	${DRIVER}/Src/stm32f4xx_ll_gpio.c
	${DRIVER}/Src/stm32f4xx_hal_pcd.c
	${DRIVER}/Src/stm32f4xx_hal_pcd_ex.c
	${DRIVER}/Src/stm32f4xx_ll_usb.c
	${DRIVER}/Src/stm32f4xx_ll_rcc.c
	${DRIVER}/Src/stm32f4xx_ll_utils.c
	${DRIVER}/Src/stm32f4xx_ll_exti.c
	${DRIVER}/Src/stm32f4xx_hal_rcc.c
	${DRIVER}/Src/stm32f4xx_hal_rcc_ex.c
	${DRIVER}/Src/stm32f4xx_hal_flash.c
	${DRIVER}/Src/stm32f4xx_hal_flash_ex.c
	${DRIVER}/Src/stm32f4xx_hal_flash_ramfunc.c
	${DRIVER}/Src/stm32f4xx_hal_gpio.c
	${DRIVER}/Src/stm32f4xx_hal_dma_ex.c
	${DRIVER}/Src/stm32f4xx_hal_dma.c
	${DRIVER}/Src/stm32f4xx_hal_pwr.c
	${DRIVER}/Src/stm32f4xx_hal_pwr_ex.c
	${DRIVER}/Src/stm32f4xx_hal_cortex.c
	${DRIVER}/Src/stm32f4xx_hal.c
	${DRIVER}/Src/stm32f4xx_hal_exti.c
	${DRIVER}/Src/stm32f4xx_ll_dma.c
	${DRIVER}/Src/stm32f4xx_ll_i2c.c
	${DRIVER}/Src/stm32f4xx_ll_spi.c
	${DRIVER}/Src/stm32f4xx_hal_tim.c
	${DRIVER}/Src/stm32f4xx_hal_tim_ex.c
	${DRIVER}/Src/stm32f4xx_ll_tim.c
	${RTOS}/croutine.c
	${RTOS}/event_groups.c
	${RTOS}/list.c
	${RTOS}/queue.c
	${RTOS}/stream_buffer.c
	${RTOS}/tasks.c
	${RTOS}/timers.c
	${RTOS}/CMSIS_RTOS/cmsis_os.c
	${RTOS}/portable/MemMang/heap_1.c
	${RTOS}/portable/GCC/ARM_CM4F/port.c
	${USB}/App/usb_device.c
	${USB}/App/usbd_desc.c
	${USB}/App/usbd_cdc_if.c
	${USB}/Target/usbd_conf.c
	${USBD}/Core/Src/usbd_core.c
	${USBD}/Core/Src/usbd_ctlreq.c
	${USBD}/Core/Src/usbd_ioreq.c
	${USBD}/Class/CDC/Src/usbd_cdc.c
)

##########
# products
##########
add_executable(${ELF_FILE} ${SRCS})

add_custom_command(TARGET ${ELF_FILE} POST_BUILD
	COMMAND ${OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
	COMMAND ${OBJCOPY} -Oihex   ${ELF_FILE} ${HEX_FILE}
	COMMAND ${SIZE} --format=berkeley ${ELF_FILE} ${HEX_FILE}
	COMMENT "Invoking: Cross ARM GNU Print Size"
)
