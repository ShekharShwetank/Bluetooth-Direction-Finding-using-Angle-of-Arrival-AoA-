#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// The crypto code is disabled for the QEMU target as it's not supported.
// To re-enable when building for real hardware, change '#if 0' to '#if 1'
// and add the correct Kconfig flags to prj.conf.
#if 0
#include <mbedtls/gcm.h>
#include <mbedtls/error.h>
#endif

/**
 * @brief Simulates generating an Angle of Arrival (AoA) value.
 */
static int simulate_aoa(void)
{
    return rand() % 360;
}

#if 0 // Disabling the encryption function for the QEMU build
static int encrypt_aoa_data(const uint8_t *plaintext, size_t plaintext_len,
                            uint8_t *ciphertext, size_t ciphertext_buf_size,
                            size_t *ciphertext_len)
{
    int ret = 0;
    mbedtls_gcm_context ctx;
    unsigned char tag[16];
    const uint8_t key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    const uint8_t nonce[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b
    };

    mbedtls_gcm_init(&ctx);
    ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
    if (ret != 0) {
        printk("mbedtls_gcm_setkey failed with error %d\n", ret);
        goto cleanup;
    }
    if (ciphertext_buf_size < plaintext_len + sizeof(tag)) {
        printk("Ciphertext buffer is too small.\n");
        ret = -1;
        goto cleanup;
    }
    ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, plaintext_len,
                                    nonce, sizeof(nonce), NULL, 0,
                                    plaintext, ciphertext, sizeof(tag), tag);
    if (ret != 0) {
        printk("mbedtls_gcm_crypt_and_tag failed with error %d\n", ret);
        goto cleanup;
    }
    memcpy(ciphertext + plaintext_len, tag, sizeof(tag));
    *ciphertext_len = plaintext_len + sizeof(tag);
cleanup:
    mbedtls_gcm_free(&ctx);
    return ret;
}
#endif

void main(void)
{
    srand(k_uptime_get_32());
    int angle = 0;

    printk("--- MCUboot & Simulation Demo ---\n");
    printk("Target: QEMU Cortex-M3\n");
    printk("NOTE: Encryption is disabled for this QEMU build.\n\n");

    while (1) {
        angle = simulate_aoa();

        // The encryption call is now disabled. We just print the simulated value.
        printk("Simulated AoA: %d degrees\n", angle);

        k_sleep(K_SECONDS(2));
    }
}