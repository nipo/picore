add_library(pr_i2c_gpio i2c_gpio.c)
target_link_libraries(pr_i2c_gpio PUBLIC
                      pico_stdlib
                      pr_error
                      pr_i2c_bus
        )
target_include_directories (pr_i2c_gpio PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
