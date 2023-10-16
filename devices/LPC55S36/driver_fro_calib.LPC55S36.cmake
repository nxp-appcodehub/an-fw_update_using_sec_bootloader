# Add set(CONFIG_USE_driver_fro_calib true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_DEVICE_ID STREQUAL LPC55S36) AND CONFIG_USE_driver_common)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/drivers
)

if(CONFIG_TOOLCHAIN STREQUAL mdk)
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/arm/keil_lib_fro_calib.lib
      -Wl,--end-group
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL iar)
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/iar/iar_lib_fro_calib.a
      -Wl,--end-group
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL mcux)
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/libfro_calib_hardabi.a
      ${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/libfro_calib_softabi.a
      -Wl,--end-group
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL armgcc)
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/gcc/libfro_calib_hardabi.a
      ${CMAKE_CURRENT_LIST_DIR}/gcc/libfro_calib_softabi.a
      -Wl,--end-group
  )
endif()

else()

message(SEND_ERROR "driver_fro_calib.LPC55S36 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
