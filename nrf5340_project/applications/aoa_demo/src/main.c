#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
// Include the Direction Finding API header
#include <zephyr/bluetooth/direction.h>

// Advertising data remains the same
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, (sizeof(CONFIG_BT_DEVICE_NAME) - 1)),
};

// Create a global pointer for our advertising set
static struct bt_le_ext_adv *adv_set;

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

    // Parameters for starting the extended advertising set.
    struct bt_le_ext_adv_start_param ext_adv_start_param = {
        .timeout = 0, // No timeout
        .num_events = 0, // Advertise indefinitely
    };

    // Create a non-connectable, non-scannable extended advertising set.
    // We pass the option directly instead of using a complex struct.
    err = bt_le_ext_adv_create(BT_LE_EXT_ADV_NCONN, NULL, &adv_set);
    if (err) {
        printk("Failed to create advertising set (err %d)\n", err);
        return;
    }
    printk("Advertising set created.\n");

    // --- Configure CTE Transmission ---
    struct bt_df_adv_cte_tx_param cte_params = {
        .cte_len = 16, // 16 * 125us = 20us CTE
        .cte_type = BT_DF_CTE_TYPE_AOA,
        .cte_count = 1,
        .num_ant_ids = 0,
        .ant_ids = NULL,
    };

    err = bt_df_set_adv_cte_tx_param(adv_set, &cte_params);
    if (err) {
        printk("Failed to set CTE parameters (err %d)\n", err);
        return;
    }
    printk("CTE parameters set.\n");

    // Set the advertising data for our set.
    err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Failed to set advertising data (err %d)\n", err);
        return;
    }
    printk("Advertising data set.\n");

    // Start the extended advertising set.
    err = bt_le_ext_adv_start(adv_set, &ext_adv_start_param);
    if (err) {
        printk("Failed to start extended advertising (err %d)\n", err);
        return;
    }

    printk("Extended advertising with CTE successfully started.\n");
}

/**
 * @brief Main application entry point.
 */
void main(void)
{
    printk("--- Starting BLE Extended Advertising with CTE Demo ---\n");

    // Enable the Bluetooth Subsystem
    int err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
    }

    printk("Main function complete. Handing over to BLE stack.\n");
}