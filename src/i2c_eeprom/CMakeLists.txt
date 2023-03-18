add_library(pr_i2c_eeprom eeprom.c)
target_link_libraries(pr_i2c_eeprom PUBLIC
                      pico_stdlib
                      pr_error
                      pr_i2c_bus
        )
target_include_directories (pr_i2c_eeprom PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
