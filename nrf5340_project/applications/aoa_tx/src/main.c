#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/direction.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(aoa_tx, LOG_LEVEL_DBG);

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(0x180f)) // Battery Service UUID
};

// Periodic advertising data
static const struct bt_data per_ad[] = {
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, 'A', 'o', 'A', '_', 'T', 'X')
};

// Declare advertising set
static struct bt_le_ext_adv *adv_set;

int main(void)
{
    int err;

    LOG_INF("Starting AoA TX device with CTE...");

    // Initialize Bluetooth
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return -1;
    }
    LOG_INF("Bluetooth initialized");

    // Create extended advertising set
    err = bt_le_ext_adv_create(BT_LE_EXT_ADV_NCONN, NULL, &adv_set);
    if (err) {
        LOG_ERR("Failed to create advertising set (err %d)", err);
        return -1;
    }
    LOG_INF("Advertising set created");

    // Set advertising data
    err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Failed to set advertising data (err %d)", err);
        return -1;
    }
    LOG_INF("Advertising data set");

    // Configure periodic advertising parameters
    struct bt_le_per_adv_param per_adv_param = {
        .interval_min = BT_GAP_PER_ADV_MIN_INTERVAL,
        .interval_max = BT_GAP_PER_ADV_MAX_INTERVAL,
        .options = 0,
    };

    err = bt_le_per_adv_set_param(adv_set, &per_adv_param);
    if (err) {
        LOG_ERR("Failed to set periodic advertising parameters (err %d)", err);
        return -1;
    }
    LOG_INF("Periodic advertising parameters set");

    // Set periodic advertising data
    err = bt_le_per_adv_set_data(adv_set, per_ad, ARRAY_SIZE(per_ad));
    if (err) {
        LOG_ERR("Failed to set periodic advertising data (err %d)", err);
        return -1;
    }
    LOG_INF("Periodic advertising data set");

    // Configure CTE transmission parameters using CORRECT Zephyr constants
    struct bt_df_adv_cte_tx_param cte_params = {
        .cte_len = 20,                    // Use BT_HCI_LE_CTE_LEN_MAX (20 * 8μs = 160μs)
        .cte_type = BT_DF_CTE_TYPE_AOA,   // Use proper enum constant (BIT(0) = 1)
        .cte_count = 1,                   // Number of CTEs per advertising event
        .num_ant_ids = 0,                 // Number of antenna IDs (0 for AoA)
        .ant_ids = NULL,                  // Antenna switching pattern (NULL for AoA)
    };

    // Enable CTE transmission for periodic advertising
    err = bt_df_set_adv_cte_tx_param(adv_set, &cte_params);
    if (err) {
        LOG_ERR("Failed to set CTE TX parameters (err %d)", err);
        return -1;
    }
    LOG_INF("CTE TX parameters set: len=%d, type=%d", cte_params.cte_len, cte_params.cte_type);

    // Enable CTE transmission
    err = bt_df_adv_cte_tx_enable(adv_set);
    if (err) {
        LOG_ERR("Failed to enable CTE TX (err %d)", err);
        return -1;
    }
    LOG_INF("CTE TX enabled");

    // Start periodic advertising
    err = bt_le_per_adv_start(adv_set);
    if (err) {
        LOG_ERR("Failed to start periodic advertising (err %d)", err);
        return -1;
    }
    LOG_INF("Periodic advertising started");

    // Start extended advertising
    err = bt_le_ext_adv_start(adv_set, BT_LE_EXT_ADV_START_DEFAULT);
    if (err) {
        LOG_ERR("Failed to start extended advertising (err %d)", err);
        return -1;
    }

    LOG_INF("AoA TX successfully started - broadcasting CTEs");

    // Keep the application running
    while (1) {
        k_sleep(K_SECONDS(5));
        LOG_INF("AoA TX running - CTE transmission active");
    }

    return 0;
}

// This code is for a Bluetooth AoA TX device that broadcasts CTEs.
// It initializes Bluetooth, creates an extended advertising set,
// sets advertising and periodic advertising data, configures CTE transmission,
// and starts both periodic and extended advertising.
// The CTE transmission parameters are set to broadcast AoA CTEs with a length of 20 (160μs).
// The device continuously runs, logging its status every 5 seconds