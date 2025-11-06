add_library(pr_gpio_irq gpio_irq.c)

target_link_libraries(pr_gpio_irq PRIVATE
        pico_stdlib
        hardware_gpio
        PUBLIC
        pr_task
        )

target_include_directories (pr_gpio_irq PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
