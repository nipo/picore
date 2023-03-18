#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pico/binary_info.h>
#include <pr/binary_info_reader.h>
#include <pr/build_id.h>
#include "build_uuid.h"

#define BINARY_INFO_TAG_PR BINARY_INFO_MAKE_TAG('P', 'u')
#define BINARY_INFO_ID_BUILD_UUID 0xb22c71f9
//#define BINARY_INFO_TAG_PR BINARY_INFO_TAG_RASPBERRY_PI
//#define BINARY_INFO_ID_BUILD_UUID BINARY_INFO_ID_RP_PROGRAM_URL

bi_decl(bi_string(BINARY_INFO_TAG_PR, BINARY_INFO_ID_BUILD_UUID, BUILD_UUID))

const char *pr_build_id_get(void)
{
    const struct binary_info_descriptor *desc = pr_bi_desc_find();
    if (!desc)
        return NULL;

    return pr_bi_find_string(desc, NULL, BINARY_INFO_TAG_PR, BINARY_INFO_ID_BUILD_UUID);
}
