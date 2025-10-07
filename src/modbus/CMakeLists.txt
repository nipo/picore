add_library(pr_modbus
            modbus.c
            modbus_crc16.c
            )
                
# target_link_libraries(pr_modbus PRIVATE)
target_link_libraries(pr_modbus PUBLIC
                      pr_task
                      pr_fifo
                      pr_endian
                      hardware_timer
                      )
target_include_directories(pr_modbus PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(pr_modbus PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)
