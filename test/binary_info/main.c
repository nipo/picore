#include <stdio.h>
#include <tusb.h>
#include <pico/unique_id.h>
#include <pr/build_id.h>
#include <pr/binary_info_reader.h>

int main(void)
{
    stdio_init_all();
    
    char id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

    pico_get_unique_board_id_string(id, sizeof(id));

    for (;;) {
        while (!tud_cdc_connected()) {
            sleep_ms(100);
        }
    
        printf("Chip UID: %s\n", id);
        printf("Program name: %s\n", pr_binary_info_program_name());
        printf("Build date: %s\n", pr_binary_info_build_date());
        printf("Board: %s\n", pr_binary_info_board_name());
        printf("Build ID: %s\n", pr_build_id_get());

        while (tud_cdc_connected()) {
            sleep_ms(100);
        }
    }

    return 0;
}
