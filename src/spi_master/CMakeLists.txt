add_library(pr_spi_master spi_master.c)
target_link_libraries(pr_spi_master PUBLIC
        pico_stdlib
        pr_error
        hardware_spi
        hardware_gpio
        )
target_include_directories (pr_spi_master PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
