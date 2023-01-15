/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * SPDX-FileCopyrightText: The Mbed TLS Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: 2015-2021 Espressif Systems (Shanghai) CO LTD
 */
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "esp_crt_bundle.h"

#include "https.h"
#include "button_cb.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "192.168.114.158"
#define WEB_PORT "8000"
#define WEB_URL "/api/data/"
static const char *TAG = "example";

static const char *REQUEST = "POST " WEB_URL " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "Cache-Control: no-cache\r\n"
                             "Content-Type: application/x-www-form-urlencoded\r\n"
                             "Content-Length: %d\r\n"
                             "\r\n"
                             "%s";
bool set_vars_flag = false;
int ret, flags, len;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_x509_crt cacert;
mbedtls_ssl_config conf;
mbedtls_net_context server_fd;

void https_post_task(const char *pvParameters)
{
    char buf[256];
    char req_buf[256];
    snprintf(req_buf, 255, REQUEST, strlen(pvParameters), (char *)pvParameters);
    ESP_LOGW(TAG, "%s", req_buf);

    if (!set_vars_flag)
    {

        mbedtls_ssl_init(&ssl);
        mbedtls_x509_crt_init(&cacert);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        ESP_LOGI(TAG, "Seeding the random number generator");

        mbedtls_ssl_config_init(&conf);

        mbedtls_entropy_init(&entropy);
        if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                         NULL, 0)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
            abort();
        }

        ESP_LOGI(TAG, "Attaching the certificate bundle...");

        ret = esp_crt_bundle_attach(&conf);

        if (ret < 0)
        {
            ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
            abort();
        }

        ESP_LOGI(TAG, "Setting hostname for TLS session...");

        /* Hostname set here should match CN in server certificate */
        if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
            abort();
        }

        ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

        if ((ret = mbedtls_ssl_config_defaults(&conf,
                                               MBEDTLS_SSL_IS_CLIENT,
                                               MBEDTLS_SSL_TRANSPORT_STREAM,
                                               MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
            goto exit;
        }

        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
        mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

        if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
            goto exit;
        }
        set_vars_flag = true;
    }

    while (1)
    {
        mbedtls_net_init(&server_fd);

        ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                       WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }

        ESP_LOGI(TAG, "Connected.");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }

        ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            /* In real life, we probably want to close connection if ret != 0 */
            ESP_LOGW(TAG, "Failed to verify peer certificate!");
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
            ESP_LOGW(TAG, "verification info: %s", buf);
        }
        else
        {
            ESP_LOGI(TAG, "Certificate verified.");
        }

        ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

        ESP_LOGI(TAG, "Writing HTTP request...");

        size_t written_bytes = 0;
        ESP_LOGW(TAG, "length is %d %s", strlen(req_buf), req_buf);

        do
        {
            ret = mbedtls_ssl_write(&ssl,
                                    (const unsigned char *)req_buf + written_bytes,
                                    strlen(req_buf) - written_bytes);
            if (ret >= 0)
            {
                ESP_LOGI(TAG, "%d bytes written", ret);
                written_bytes += ret;
            }
            else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
                goto exit;
            }
        } while (written_bytes < strlen(buf));

        ESP_LOGI(TAG, "Reading HTTP response...");
        buf[0] = '\0';
        do
        {
            len = sizeof(buf) - 1;
            bzero(buf, sizeof(buf));
            ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

            if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                continue;

            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            {
                ret = 0;
                break;
            }

            if (ret < 0)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
                break;
            }

            if (ret == 0)
            {
                ESP_LOGI(TAG, "connection closed");
                break;
            }

            len = ret;
            ESP_LOGD(TAG, "%d bytes read", len);
            /* Print response directly to stdout as it is read */
            for (int i = 0; i < len; i++)
            {
                putchar(buf[i]);
            }
        } while (1);

        mbedtls_ssl_close_notify(&ssl);

    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if (ret != 0)
        {
            mbedtls_strerror(ret, buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
        }

        putchar('\n'); // JSON output doesn't have a newline at end

        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);
        printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

        // for (int countdown = 10; countdown >= 0; countdown--)
        // {
        //     ESP_LOGI(TAG, "%d...", countdown);
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
        // ESP_LOGI(TAG, "Starting again!");
        break;
    }
}

void send_data(const char *data)
{
    ESP_LOGI(TAG, "%s", data);
    https_post_task(data);
}

char out[14];

/**
 * @brief sends queue data to the server
 *
 */
void https_send_task()
{
    static char data_to_send[512];
    while (1)
    {
        ESP_LOGI(TAG, "Checking queue");

        if (uxQueueMessagesWaiting(xQueue) >= MAX_BUFFER_TO_STORE)
        {
            ESP_LOGI(TAG, "BUFFER EXCEEDED! SENDING RECORDS! buffer size: %d", uxQueueMessagesWaiting(xQueue));
            // xQueueReceive(xQueue, out, (TickType_t)0);
            data_to_send[0] = '\0';
            while (1)
            {
                int to_send = uxQueueMessagesWaiting(xQueue) > MAX_RECORD_TO_SEND_AT_ONCE ? MAX_RECORD_TO_SEND_AT_ONCE : uxQueueMessagesWaiting(xQueue);
                if (to_send == 0)
                {
                    break;
                }
                else
                {
                    ESP_LOGI(TAG, "length of witing queue is : %d", to_send);
                    for (int i = 0; i < to_send; i++)
                    {
                        xQueueReceive(xQueue, out, (TickType_t)0);
                        if (i == 0)
                        {
                            strcat(data_to_send, "id=D-A-00-11-22-33-44&");
                        }
                        else
                        {
                            strcat(data_to_send, "&");
                        }
                        strcat(data_to_send, out);
                    }
                }
                ESP_LOGW(TAG, "Records compiled and ready to send");
                send_data(data_to_send);
                data_to_send[0] = '\0';
            }
        }
        ESP_LOGW(TAG, "will wait for 10 second ");

        vTaskDelay(10000 / portTICK_PERIOD_MS); // wait for 10 seconds before checking queue
    }
}