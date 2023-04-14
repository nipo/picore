#include <pr/endian.h>

extern inline
uint16_t pr_be16_na_read(const void *ptr);

extern inline
uint16_t pr_le16_na_read(const void *ptr);

extern inline
uint32_t pr_be32_na_read(const void *ptr);

extern inline
uint32_t pr_le32_na_read(const void *ptr);

extern inline
void pr_be16_na_write(void *ptr, uint16_t value);

extern inline
void pr_le16_na_write(void *ptr, uint16_t value);

extern inline
void pr_be32_na_write(void *ptr, uint32_t value);

extern inline
void pr_le32_na_write(void *ptr, uint32_t value);
