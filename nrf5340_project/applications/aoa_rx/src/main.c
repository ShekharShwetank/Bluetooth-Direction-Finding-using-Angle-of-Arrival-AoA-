#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/direction.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <string.h>
// #include <math.h> // Uncomment if using math functions like M_PI

LOG_MODULE_REGISTER(aoa_rx, LOG_LEVEL_DBG);

static struct bt_le_per_adv_sync *per_adv_sync;

// --- Angle estimation function (commented out due to M_PI error) ---
// static float estimate_angle(float delta_phase) {
//     float angle;
//     // angle = delta_phase * (180.0f / M_PI); // Convert to degrees
//     // return angle;
//     return 0.0f;
// }

// --- Encryption function (commented out due to struct errors) ---
// static int encrypt_angle(float angle, uint8_t *out_buf, size_t out_buf_len) {
//     // Example AES-GCM encryption using Zephyr's crypto API
//     // struct cipher_aead_pkt aead_pkt = {
//     //     .nonce = (uint8_t *)aes_iv,
//     //     .nonce_len = sizeof(aes_iv),
//     //     .pkt = pkt,
//     //     .tag_len = 16,
//     // };
//     // int rc = cipher_aead_op(&ctx, &aead_pkt, CRYPTO_CIPHER_OP_ENCRYPT);
//     // return rc;
//     return -ENOTSUP;
// }

// --- CTE IQ report callback (commented out due to undefined types) ---
// static void cte_report_cb(struct bt_le_per_adv_sync *sync,
//                           const struct bt_df_per_adv_sync_iq_samples_report *report)
// {
//     // Example: process IQ samples
//     // for (int i = 0; i < report->sample_count; ++i) {
//     //     buf.i[i] = report->sample[i].i;
//     //     buf.q[i] = report->sample[i].q;
//     // }
//     // float angle = estimate_angle(...);
//     // LOG_INF("Estimated AoA: %.2f degrees", angle);
// }

static void sync_cb(struct bt_le_per_adv_sync *sync,
                    struct bt_le_per_adv_sync_synced_info *info)
{
    printk("Synced to periodic advertiser\n");

    struct bt_df_per_adv_sync_cte_rx_param cte_rx_param = {
        .cte_types = BT_DF_CTE_TYPE_AOA,
        .slot_durations = BT_DF_ANTENNA_SWITCHING_SLOT_1US,
        .max_cte_count = 0,
        .num_ant_ids = 0,
        .ant_ids = NULL,
    };
    int err = bt_df_per_adv_sync_cte_rx_enable(sync, &cte_rx_param);
    printk("CTE RX enable: %d\n", err);
}

static void term_cb(struct bt_le_per_adv_sync *sync,
                    const struct bt_le_per_adv_sync_term_info *info)
{
    printk("Sync terminated (reason: %d)\n", info->reason);
}

static void recv_cb(struct bt_le_per_adv_sync *sync,
                    const struct bt_le_per_adv_sync_recv_info *info,
                    struct net_buf_simple *buf)
{
    printk("Received periodic advertising data (len=%u)\n", buf->len);
    // If your SDK delivers CTE IQ samples here, parse them in this callback.
}

static struct bt_le_per_adv_sync_cb per_adv_sync_cbs = {
    .synced = sync_cb,
    .term = term_cb,
    .recv = recv_cb,
    // .cte_report = cte_report_cb, // Commented out: not available in this SDK
};

static bool ad_parse_cb(struct bt_data *data, void *user_data)
{
    char *dev_name = user_data;
    if (data->type == BT_DATA_NAME_COMPLETE) {
        size_t len = data->data_len < 31 ? data->data_len : 31;
        memcpy(dev_name, data->data, len);
        dev_name[len] = '\0';
        return false; // Stop parsing
    }
    return true; // Continue parsing
}

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad)
{
    char dev_name[32] = {0};
    bt_data_parse(ad, ad_parse_cb, dev_name);
    if (strcmp(dev_name, "AoA_TX_Sim") == 0) {
        printk("Found AoA_TX_Sim, creating sync...\n");
        struct bt_le_per_adv_sync_param sync_create_param = {
            .addr = *addr,
            .sid = 0,
            .skip = 0,
            .timeout = 400,
            .options = 0,
        };
        int err = bt_le_per_adv_sync_create(&sync_create_param, &per_adv_sync);
        printk("Sync create: %d\n", err);
        bt_le_scan_stop();
    }
}

// Use 'int main(void)' for Zephyr simulation builds
int main(void)
{
    int err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    bt_le_per_adv_sync_cb_register(&per_adv_sync_cbs);
    err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, scan_cb);
    if (err) {
        printk("Scan start failed (err %d)\n", err);
        return -1;
    }
    printk("Scanning for AoA_TX_Sim...\n");
    while (1) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
