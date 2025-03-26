/**
 * @file gpio_irq_driver.c
 * @brief GPIO IRQ Counter Driver
 *
 * This driver handles GPIO interrupts and counts the number of times an interrupt occurs.
 * It is designed to work with a device tree entry that specifies a GPIO line.
 *
 * @autor Arkadiusz Kubiak
 * @license GPL
 */

 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/gpio.h>
 #include <linux/interrupt.h>
 #include <linux/of.h>
 #include <linux/of_gpio.h>
 #include <linux/platform_device.h>
 #include <linux/atomic.h> // Use atomic operations provided by the kernel

 
 #define DRIVER_NAME "gpio_irq_counter"
 
 // Global variables to store the IRQ number and interrupt count
 static int gpio_irq;
 static atomic_t counter = ATOMIC_INIT(0);
 static struct gpio_desc *gpio_desc;
 
 /**
  * @brief Interrupt handler function.
  *
  * This function is called whenever an interrupt occurs on the specified GPIO line.
  * It increments the interrupt count and logs a message.
  *
  * @param irq The IRQ number.
  * @param dev_id Pointer to device-specific data (unused).
  * @return IRQ_HANDLED indicating the interrupt was handled successfully.
  */
 static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
 {
     // Increment the interrupt count
     atomic_inc(&counter);  // Atomically increments counter
     // Log the interrupt occurrence and count
     pr_info(DRIVER_NAME ": Interrupt occurred! Count: %u\n", atomic_read(&counter));
     return IRQ_HANDLED;
 }
 
 /**
  * @brief Probe function for the GPIO IRQ driver.
  *
  * This function is called when the driver is matched with a device.
  * It retrieves the GPIO from the device tree, requests an IRQ, and sets up the interrupt handler.
  *
  * @param pdev Pointer to the platform device structure.
  * @return 0 on success, or a negative error code on failure.
  */
 static int gpio_irq_probe(struct platform_device *pdev)
 {
     struct device *dev = &pdev->dev;
     int ret;
 
     // Get the GPIO descriptor from the device tree
     gpio_desc = devm_gpiod_get(dev, NULL, GPIOD_IN);
     if (IS_ERR(gpio_desc)) {
         dev_err(dev, "Failed to get GPIO from device tree\n");
         return PTR_ERR(gpio_desc);
     }
 
     // Convert the GPIO descriptor to an IRQ number
     gpio_irq = gpiod_to_irq(gpio_desc);
     if (gpio_irq < 0) {
         dev_err(dev, "Failed to get IRQ number\n");
         return gpio_irq;
     }
 
     // Request the IRQ and set up the interrupt handler
     ret = devm_request_irq(dev, gpio_irq, gpio_irq_handler, IRQF_TRIGGER_FALLING, DRIVER_NAME, NULL);
     if (ret) {
         dev_err(dev, "Failed to request IRQ\n");
         return ret;
     }
 
     // Set the GPIO direction to input
     ret = gpiod_direction_input(gpio_desc);
     if (ret) {
         dev_err(dev, "Failed to set GPIO direction to input\n");
         return ret;
     }
 
     // Log the successful driver loading and IRQ registration
     pr_info(DRIVER_NAME ": Driver loaded, IRQ registered on GPIO\n");
     return 0;
 }
 
 /**
  * @brief Device tree match table.
  *
  * This table is used to match the driver with compatible devices specified in the device tree.
  */
 static const struct of_device_id gpio_irq_dt_ids[] = {
     { .compatible = "custom,gpio-irq-counter" },
     {}
 };
 
 /**
  * @brief Platform driver structure.
  *
  * This structure contains the driver name, device tree match table, and probe function.
  */
 static struct platform_driver gpio_irq_driver = {
     .driver = {
         .name = DRIVER_NAME,
         .of_match_table = gpio_irq_dt_ids,
     },
     .probe = gpio_irq_probe,
 };
 
 /**
  * @brief Driver initialization function.
  *
  * This function is called when the driver is loaded. It registers the platform driver.
  *
  * @return 0 on success, or a negative error code on failure.
  */
 static int __init gpio_irq_driver_init(void)
 {
     return platform_driver_register(&gpio_irq_driver);
 }
 device_initcall(gpio_irq_driver_init);
 
