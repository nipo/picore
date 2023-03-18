add_library(pr_i2c_bus i2c_bus.c)
target_link_libraries(pr_i2c_bus PUBLIC
        pico_stdlib
        pr_error
        hardware_i2c
        hardware_gpio
        )
target_include_directories (pr_i2c_bus PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
