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
#include "provisioning.h"
/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "httpbin.org"
#define WEB_PORT "443"
#define WEB_URL "/post"
static const char *TAG = "https";

static const char *REQUEST = "POST " WEB_URL " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "Cache-Control: no-cache\r\n"
                             "Content-Type: application/x-www-form-urlencoded\r\n"
                             "Content-Length: %d\r\n"
                             "\r\n"
                             "%s";

void https_post_task(char *pvParameters)
{

    char buf[512];
    char req_buf[512];
    snprintf(req_buf, 255, REQUEST, strlen(pvParameters), pvParameters);
    int ret, flags, len;
    ESP_LOGW(TAG, "%s", req_buf);
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

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

    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.
       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif

#ifdef CONFIG_MBEDTLS_SSL_PROTO_TLS1_3
    mbedtls_ssl_conf_min_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_4);
    mbedtls_ssl_conf_max_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_4);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
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
        } while (written_bytes < strlen(req_buf));

        ESP_LOGI(TAG, "Reading HTTP response...");

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
        ESP_LOGI(TAG, "%s", buf);

        printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

        break;
    }
}

void test_task(char *pvParameters)
{
    ESP_LOGI(TAG, "%s", pvParameters);
    vTaskDelay(12000 / portTICK_PERIOD_MS); // wait for 10 sec before sending again
    vTaskDelete(NULL);
}

esp_err_t send_data(const char *data)
{
    TaskHandle_t xHandle = NULL;
    while (1)
    {
        char req_res_data[512];
        strcpy(req_res_data, data);
        bool success = 0;
        // if (https_post_task(req_res_data) == ESP_OK)
        // {
        //     sscanf(req_res_data, "{\"success\": \"%i\", \"status\": \"successfully saved!\"}", success);
        //     if (success)
        //     {
        //         return ESP_OK;
        //     }
        //     else
        //     {
        //         continue;
        //     }
        // }
        // else
        // {
        //     continue;
        // }
        if (is_wifi_connected)
        {
            ESP_LOGW(TAG, "%s", req_res_data);

            https_post_task(req_res_data);
            ESP_LOGW(TAG, "created task for sending");
        }
        else
        {
            ESP_LOGW(TAG, "wifi not connected-- waiting for 10 sec to resend");
            vTaskDelay(10000 / portTICK_PERIOD_MS); // wait for 10 sec before sending again
        }

        ESP_LOGI(TAG, "%s", req_res_data);
    }
}

char out[14];

/**
 * @brief sends queue data to the server
 *
 */
void https_send_task()
{
    static char data_to_send[512];
    SemaphoreHandle_t xHttpsRequestSemaphore = xSemaphoreCreateBinary(); // semaphore for self syncronization
    xSemaphoreGive(xHttpsRequestSemaphore);

    while (1)
    {
        ESP_LOGI(TAG, "Checking queue");
        // checking if https sending action is available
        xSemaphoreTake(xHttpsRequestSemaphore, portMAX_DELAY);

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
        xSemaphoreGive(xHttpsRequestSemaphore);
    }
}