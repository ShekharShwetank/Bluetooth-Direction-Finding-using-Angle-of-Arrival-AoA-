#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/direction.h>

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, "AoATX", 5),
};

// Global pointer for our advertising set
static struct bt_le_ext_adv *adv_set;

// Callback for when Bluetooth is ready
static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }
    printk("Bluetooth initialized\n");

    struct bt_le_ext_adv_start_param ext_adv_start_param = {
        .timeout = 0,
        .num_events = 0,
    };

    // Use the correct parameter for a non-connectable advertising set
    err = bt_le_ext_adv_create(BT_LE_EXT_ADV_NCONN, NULL, &adv_set);
    if (err) {
        printk("Failed to create advertising set (err %d)\n", err);
        return;
    }
    printk("Advertising set created.\n");

    // Configure CTE Transmission
    struct bt_df_adv_cte_tx_param cte_params = {
        .cte_len = 16,
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

    // Set the advertising data
    err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Failed to set advertising data (err %d)\n", err);
        return;
    }
    printk("Advertising data set.\n");

    // Start extended advertising
    err = bt_le_ext_adv_start(adv_set, &ext_adv_start_param);
    if (err) {
        printk("Failed to start extended advertising (err %d)\n", err);
        return;
    }
    printk("Extended advertising with CTE successfully started.\n");
}

// Main application entry point
void main(void)
{
    printk("--- Starting AoA CTE Transmitter ---\n");
    int err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
    }
    printk("Main function complete.\n");
}