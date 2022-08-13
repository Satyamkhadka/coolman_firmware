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
#define WEB_PATH "/api/data/"

// #define WEB_SERVER "example.com"
// #define WEB_PORT "80"
// #define WEB_PATH "/"

static const char *TAG = "example";

static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Cache-Control: no-cache\r\n"
                             "Content-Type: application/x-www-form-urlencoded\r\n"
                             "\r\n"
                             "%s";
// static const char *REQUEST = "POST " WEB_PATH " HTTP/1.0\r\n"
//                              "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
//                              "User-Agent: esp-idf/1.0 esp32\r\n"
//                              "Cache-Control: no-cache\r\n"
//                              "Content-Type: application/x-www-form-urlencoded\r\n"
//                              "\r\n"
//                              "id=D-A-00-11-22-33-44&66=8&76=5&81=6";
// static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
//                              "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
//                              "User-Agent: esp-idf/1.0 esp32\r\n"
//                              "\r\n";
void https_post_task(const char *pvParameters)

{

    char buf[512];
    // strcpy(buf, REQUEST);
    snprintf(buf, 511, REQUEST, (char *)pvParameters);
    ESP_LOGW(TAG, "%s", buf);
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while (1)
    {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0)
        {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
        {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, buf, strlen(buf)) < 0)
        {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                       sizeof(receiving_timeout)) < 0)
        {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do
        {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            for (int i = 0; i < r; i++)
            {
                putchar(recv_buf[i]);
            }
        } while (r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        break;
    }
}
void send_data(const char *data)
{
    ESP_LOGI(TAG, "%s", data);
    https_post_task(data);
}
char out[12];

/**
 * @brief sends queue data to the server
 *
 */
void https_send_task()
{
    static char data_to_send[1024];
    while (1)
    {
        if (uxQueueMessagesWaiting(xQueue) > MAX_BUFFER_TO_STORE)
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
                    for (int i = 0; i < MAX_RECORD_TO_SEND_AT_ONCE; i++)
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
        vTaskDelay(10000 / portTICK_PERIOD_MS); // wait for 10 seconds before checking queue
    }
}
