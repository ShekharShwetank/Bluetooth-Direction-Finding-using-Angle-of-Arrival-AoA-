#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// Include the main Bluetooth API headers
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

/**
 * @brief Define the advertising data structure.
 *
 * This data will be placed in the advertising packets. We are advertising:
 * 1. Flags: General Discoverable Mode, BR/EDR Not Supported.
 * 2. Complete Local Name: The name set by CONFIG_BT_DEVICE_NAME.
 */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, (sizeof(CONFIG_BT_DEVICE_NAME) - 1)),
};

/**
 * @brief Callback function executed when Bluetooth has been enabled.
 *
 * This function is registered with bt_enable(). Once the Bluetooth stack
 * is initialized and ready, this callback is invoked. Its job is to
 * start the advertising process.
 *
 * @param err 0 on success, or a negative error code from the stack.
 */
/**
 * @brief Callback function executed when Bluetooth has been enabled.
 */
static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    // Define the advertising parameters.
    // This is the modern replacement for the old BT_LE_ADV_PARAM_INIT macro.
    // We are setting it to be non-connectable and non-scannable advertising.
    struct bt_le_adv_param adv_param = {
        .id = BT_ID_DEFAULT,
        .sid = 0,
        .secondary_max_skip = 0,
        .options = BT_LE_ADV_OPT_USE_NAME, // Options are the same
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
        .peer = NULL,
    };

    // Start advertising with our defined parameters.
    err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising successfully started\n");
}

/**
 * @brief Main application entry point.
 *
 * This function's only job is to initialize the Bluetooth stack.
 * The rest of the application logic is handled by callbacks.
 */
void main(void)
{
    int err;

    printk("--- Starting BLE Advertising Demo ---\n");

    // Initialize the Bluetooth Subsystem.
    // bt_enable() is an asynchronous call. When it completes,
    // it will call the 'bt_ready' function we provided.
    err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
    }

    // The main function exits now, but the application continues to run,
    // driven by Bluetooth events.
    printk("Main function complete. Handing over to BLE stack.\n");
}