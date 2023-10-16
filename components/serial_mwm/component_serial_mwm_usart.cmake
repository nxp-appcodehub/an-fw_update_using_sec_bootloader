# Add set(CONFIG_USE_component_serial_mwm_usart true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_driver_flexcomm_usart_freertos)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/serial_mwm_usart.c
)

else()

message(SEND_ERROR "component_serial_mwm_usart dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
