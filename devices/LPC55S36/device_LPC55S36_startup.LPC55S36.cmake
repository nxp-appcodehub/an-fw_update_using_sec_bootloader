# Add set(CONFIG_USE_device_LPC55S36_startup true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_device_LPC55S36_system)

if(CONFIG_TOOLCHAIN STREQUAL armgcc)
  target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/gcc/startup_LPC55S36.S
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL mcux)
  target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/startup_lpc55s36.c
      ${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/startup_lpc55s36.cpp
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL iar)
  add_config_file(${CMAKE_CURRENT_LIST_DIR}/iar/startup_LPC55S36.s "" device_LPC55S36_startup.LPC55S36)
endif()

if(CONFIG_TOOLCHAIN STREQUAL mdk)
  add_config_file(${CMAKE_CURRENT_LIST_DIR}/arm/startup_LPC55S36.S "" device_LPC55S36_startup.LPC55S36)
endif()

else()

message(SEND_ERROR "device_LPC55S36_startup.LPC55S36 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
