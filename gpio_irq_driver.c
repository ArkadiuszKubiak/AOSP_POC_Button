/**
 * @file gpio_irq_driver.c
 * @brief GPIO IRQ Driver for handling interrupts on GPIO 23.
 *
 * This driver initializes a GPIO pin (GPIO 23) and sets up an interrupt handler
 * for it. The interrupt handler logs a message when the interrupt is triggered.
 *
 * The driver reads the GPIO and IRQ information from the Device Tree.
 *
 * @details
 * - The driver looks for a Device Tree node named "custom_btn" and its child node "button".
 * - It retrieves the IRQ number associated with the GPIO from the Device Tree.
 * - It requests the IRQ and sets up an interrupt handler for falling edge triggers.
 * - It also requests the GPIO descriptor for GPIO 23.
 *
 * @note
 * - The driver uses the `arch_initcall` macro to register the initialization function
 *   to be called during the boot process.
 * - The driver cleans up by freeing the IRQ and releasing the GPIO descriptor during exit.
 *
 * @author
 * - Original Author: Arkadiusz Kubiak
 *
 * @date
 * - Initial version: 2025-02-21
 *
 * @license
 * - This code is licensed under the GPL-2.0 license.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h> 
#include <linux/irq.h>

#define GPIO_NUM 23
static unsigned int irq_number;
static struct gpio_desc *gpio23;

// Interrupt handler for GPIO 23
static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
    pr_info("GPIO %d interrupt triggered!\n", GPIO_NUM);
    return IRQ_HANDLED;
}

// Initialization function for the GPIO IRQ driver
static int __init gpio_driver_init(void) {
    struct device_node *np;
    int ret;

    // Find the Device Tree node named "custom_btn"
    np = of_find_node_by_name(NULL, "button_interrupt");
    if (!np) {
        pr_err("GPIO IRQ driver: Failed to find Device Tree node 'custom_btn'\n");
        return -ENODEV;
    }
    
    // Get the IRQ number from the Device Tree
    irq_number = of_irq_get(np, 0);
    if (irq_number < 0) {
        pr_err("GPIO IRQ driver: Failed to get IRQ from Device Tree, error %d\n", irq_number);
        return irq_number;
    }
    pr_info("GPIO IRQ driver: Successfully mapped IRQ %d\n", irq_number);
    
    // Request the IRQ and set up the interrupt handler for falling edge triggers
    ret = request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_FALLING, "gpio23_irq", NULL);
    if (ret) {
        pr_err("GPIO IRQ driver: Failed to request IRQ %d, error %d\n", irq_number, ret);
        return ret;
    }
    pr_info("GPIO IRQ driver: Successfully requested IRQ %d\n", irq_number);

    // Request the GPIO descriptor for GPIO 23
    gpio23 = gpiod_get_from_of_node(np, "gpios", 0, GPIOD_IN, "custom_btn");
    if (IS_ERR(gpio23)) {
        pr_err("GPIO IRQ driver: Failed to request GPIO\n");
    }

    pr_info("GPIO IRQ driver initialized\n");
    return 0;
}

// Exit function for the GPIO IRQ driver
static void __exit gpio_driver_exit(void) {
    // Free the IRQ
    free_irq(irq_number, NULL);
    // Release the GPIO descriptor
    gpiod_put(gpio23);
    pr_info("GPIO IRQ driver removed\n");
}

// Register the initialization function to be called during the boot process
device_initcall(gpio_driver_init);
