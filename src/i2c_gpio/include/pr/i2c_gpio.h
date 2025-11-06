/**
 * @file i2c_gpio.h
 * @defgroup pr_i2c_gpio I2C GPIO Expander
 * @brief I2C GPIO expander driver with support for multiple chip types
 *
 * The i2c_gpio library provides a unified interface for controlling I2C GPIO
 * expander chips. It uses a vtable-based architecture to support different
 * expander types with varying pin counts and features.
 *
 * Supported GPIO expander types:
 * - TCA9534: 8-bit GPIO expander
 * - PCA9575: 16-bit GPIO expander
 *
 * Key features:
 * - Set/get GPIO pin states
 * - Configure output enable per pin
 * - Pull-up/pull-down configuration (chip-dependent)
 * - Cached register state for efficient updates
 * - Vtable-based extensibility
 *
 * Typical usage pattern:
 * @code
 * #include <pr/i2c_gpio.h>
 * #include <pr/i2c_bus.h>
 *
 * struct pr_i2c_bus i2c;
 * struct pr_i2c_gpio gpio_exp;
 *
 * int main(void) {
 *     pr_i2c_bus_init(&i2c, 4, 5, 400000);
 *
 *     // Initialize TCA9534 at address 0x20
 *     pr_i2c_gpio_init(&gpio_exp, &pr_i2c_gpio_tca9534, &i2c, 0x20);
 *
 *     // Configure pins 0-3 as outputs, 4-7 as inputs
 *     pr_i2c_gpio_oe(&gpio_exp, 0xFF, 0x0F);
 *
 *     // Set pins 0 and 2 high
 *     pr_i2c_gpio_set(&gpio_exp, 0x05, 0x05);
 *
 *     // Read all pins
 *     uint32_t value;
 *     pr_i2c_gpio_get(&gpio_exp, &value);
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_I2C_GPIO_H_
#define PR_I2C_GPIO_H_

#include <stdint.h>
#include <pr/error.h>

struct pr_i2c_bus;
struct pr_i2c_gpio;

/**
 * @brief I2C GPIO virtual function table
 *
 * Function pointers for GPIO expander operations. Different chip types
 * implement these with chip-specific register layouts.
 */
struct pr_i2c_gpio_vtable
{
    pr_error_t (*set)(struct pr_i2c_gpio *i2c_gpio,
                      uint32_t mask, uint32_t value); /**< Set output states */

    pr_error_t (*oe)(struct pr_i2c_gpio *i2c_gpio,
                     uint32_t mask, uint32_t value); /**< Configure output enable */

    pr_error_t (*get)(struct pr_i2c_gpio *i2c_gpio,
                      uint32_t *value); /**< Read pin states */

    pr_error_t (*pull_en)(struct pr_i2c_gpio *i2c_gpio,
                          uint32_t mask, uint32_t enable, uint32_t bias); /**< Configure pull resistors */
};

/**
 * @brief I2C GPIO expander structure
 *
 * Represents an I2C GPIO expander with cached state for efficiency.
 */
struct pr_i2c_gpio
{
    struct pr_i2c_bus *bus;                  /**< I2C bus the expander is on */
    uint8_t saddr;                           /**< 7-bit I2C address */
    const struct pr_i2c_gpio_vtable *vtable; /**< Operations vtable */
    uint32_t set_cache;                      /**< Cached output state */
    uint32_t oe_cache;                       /**< Cached output enable state */
    uint32_t pull_bias_cache;                /**< Cached pull bias (up/down) */
    uint32_t pull_en_cache;                  /**< Cached pull enable state */
};

/**
 * @brief TCA9534 GPIO expander vtable
 *
 * Vtable for 8-bit TCA9534 GPIO expander.
 */
extern const struct pr_i2c_gpio_vtable pr_i2c_gpio_tca9534;

/**
 * @brief PCA9575 GPIO expander vtable
 *
 * Vtable for 16-bit PCA9575 GPIO expander.
 */
extern const struct pr_i2c_gpio_vtable pr_i2c_gpio_pca9575;

/**
 * @brief Initialize an I2C GPIO expander
 *
 * Sets up a GPIO expander with the specified vtable and bus.
 *
 * @param i2c_gpio Pointer to GPIO expander structure
 * @param vtable Pointer to expander vtable (e.g., &pr_i2c_gpio_tca9534)
 * @param bus Pointer to I2C bus the expander is on
 * @param saddr 7-bit I2C address of the expander
 * @return PR_OK on success, error code on failure
 *
 * @note The I2C bus must be initialized before calling this function
 */
pr_error_t pr_i2c_gpio_init(struct pr_i2c_gpio *i2c_gpio,
                            const struct pr_i2c_gpio_vtable *vtable,
                            struct pr_i2c_bus *bus,
                            uint8_t saddr);

/**
 * @brief Set GPIO output states
 *
 * Sets the output state of specified GPIO pins. Only affects pins
 * configured as outputs.
 *
 * @param i2c_gpio Pointer to GPIO expander structure
 * @param mask Bit mask of pins to modify (1 = modify, 0 = leave unchanged)
 * @param value Desired pin states (1 = high, 0 = low)
 * @return PR_OK on success, error code on failure
 *
 * @note Only pins with corresponding mask bit set are affected
 * @note Value bits are only meaningful where mask bits are set
 */
static inline
pr_error_t pr_i2c_gpio_set(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t mask, uint32_t value)
{
    if (!i2c_gpio->vtable->set)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->set(i2c_gpio, mask, value);
}

/**
 * @brief Configure GPIO output enable
 *
 * Configures pins as inputs or outputs.
 *
 * @param i2c_gpio Pointer to GPIO expander structure
 * @param mask Bit mask of pins to modify (1 = modify, 0 = leave unchanged)
 * @param value Output enable states (1 = output, 0 = input)
 * @return PR_OK on success, error code on failure
 *
 * @note Only pins with corresponding mask bit set are affected
 */
static inline
pr_error_t pr_i2c_gpio_oe(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t mask, uint32_t value)
{
    if (!i2c_gpio->vtable->oe)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->oe(i2c_gpio, mask, value);
}

/**
 * @brief Configure GPIO pull resistors
 *
 * Enables/disables pull-up or pull-down resistors on GPIO pins.
 * Not all expander chips support this feature.
 *
 * @param i2c_gpio Pointer to GPIO expander structure
 * @param mask Bit mask of pins to modify (1 = modify, 0 = leave unchanged)
 * @param enable Pull resistor enable (1 = enable, 0 = disable)
 * @param bias Pull direction (1 = pull-up, 0 = pull-down)
 * @return PR_OK on success, PR_ERR_IMPL if not supported, error code on failure
 *
 * @note Not all chips support pull resistor configuration
 * @note Enable and bias are separate bit masks aligned with mask
 */
static inline
pr_error_t pr_i2c_gpio_pull_en(struct pr_i2c_gpio *i2c_gpio,
                               uint32_t mask, uint32_t enable, uint32_t bias)
{
    if (!i2c_gpio->vtable->pull_en)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->pull_en(i2c_gpio, mask, enable, bias);
}

/**
 * @brief Read GPIO pin states
 *
 * Reads the current state of all GPIO pins (both inputs and outputs).
 *
 * @param i2c_gpio Pointer to GPIO expander structure
 * @param value Pointer to receive pin states (1 = high, 0 = low)
 * @return PR_OK on success, error code on failure
 *
 * @note Output pins read back their driven state
 * @note Input pins read the actual pin level
 */
static inline
pr_error_t pr_i2c_gpio_get(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t *value)
{
    if (!i2c_gpio->vtable->get)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->get(i2c_gpio, value);
}

/** @} */ // end of pr_i2c_gpio group

#endif
