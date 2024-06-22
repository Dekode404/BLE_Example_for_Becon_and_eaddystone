/* BLE beacon & eddy-stone example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>                       /* This is the standard C lib used for the printf statement in this project */
#include <nvs_flash.h>                   /* This is ESP lib used to initiate the NVS flsh used for the bluetooth application */
#include <esp_log.h>                     /* This is ESP lib used to loig the error */
#include <esp_nimble_hci.h>              /* This is ESP lib used for the HOST and CONTROLLER interface */
#include <nimble/nimble_port.h>          /* This is ESP lib used for initiate the nimbale port for the bluetooth application */
#include <nimble/nimble_port_freertos.h> /* This is ESP lib used for create the task for the nimble bluetooth application */
#include <host/ble_hs.h>                 /* This is ESP lib used for the ble host controller */
#include <services/gap/ble_svc_gap.h>    /* This is ESP lib used for initiate the ble service */

// Uncomment one of the following to enable either EDDYSTONE or BEACON
#define EDDYSTONE_ON
// #define BEACON_ON

#if defined(EDDYSTONE_ON) && defined(BEACON_ON)
#error "Only one of EDDYSTONE_ON or BEACON_ON can be defined at a time."
#elif !defined(EDDYSTONE_ON) && !defined(BEACON_ON)
#error "One of EDDYSTONE_ON or BEACON_ON must be defined."
#endif

#define URL_SITE "parasdefence"

void BLE_app_on_sync(void)
{
    ble_addr_t BLE_address;             /* BLE address variable */
    ble_hs_id_gen_rnd(1, &BLE_address); /* Generate a random BLE non-resolvable private address */
    ble_hs_id_set_rnd(BLE_address.val); /* Set the generated random address */

#ifdef BEACON_ON
    uint8_t UUID_For_Advertise[16];                               /* Variable for 128-bit UUID to advertise */
    memset(UUID_For_Advertise, 0x99, sizeof(UUID_For_Advertise)); /* Set the UUID to 0x99 */
    ble_ibeacon_set_adv_data(UUID_For_Advertise, 2, 10, -50);     /* Set the iBeacon advertising data */

#endif

#ifdef EDDYSTONE_ON
    struct ble_hs_adv_fields fields = (struct ble_hs_adv_fields){0}; /* BLE advertisement feild variable */

    /* Configures the device to advertise Eddystone URL beacons. */
    ble_eddystone_set_adv_data_url(&fields,                        /* The base advertisement fields to transform into an eddystone beacon*/
                                   BLE_EDDYSTONE_URL_SCHEME_HTTPS, /* The prefix of the URL */
                                   URL_SITE,                       /* The middle of the URL */
                                   strlen(URL_SITE),               /* The string length of the url_body argument */
                                   BLE_EDDYSTONE_URL_SUFFIX_COM,   /* The suffix of the URL */
                                   -30);                           /* The Measured Power */
#endif

    struct ble_gap_adv_params adv_params = (struct ble_gap_adv_params){0};                 /* Advertising parameters */
    ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL); /* Start BLE advertising */
}

void Host_task(void *param)
{
    nimble_port_run(); /* Run the nimble port */
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init()); /* Initialize NVS flash */

    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init()); /* Initiate the Nimble BLE Hardware control interference and controller */

    nimble_port_init(); /* Initiate the nimble port for the BLE */

    ble_hs_cfg.sync_cb = BLE_app_on_sync; /* Set the synchronization callback */
    nimble_port_freertos_init(Host_task); /* Initialize NimBLE port with FreeRTOS */
}
