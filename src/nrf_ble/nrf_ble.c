#include "nrf_ble.h"

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "ble_dis.h"
#include "ble_dfu.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"

/* FreeRTOS related */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sys.h"
#include "app.h"
#include "ble_cmd.h"
#include "pinetime_board.h"

#define SYS_TASK_DELAY        1 
TaskHandle_t  sys_task_handle;
TaskHandle_t  app_task_handle;

// ---------------------------------------------------------------------------------------------------------

#define APP_BLE_CONN_CFG_TAG            1  
#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2

#define DEVICE_NAME                     "PineTime-cOS"                  /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "Pine64"
#define MODEL_NUM                       "PINETIME-COS"                  /**< Model number. Will be passed to Device Information Service. */
#define MANUFACTURER_ID                 0x1111111111                    /**< Manufacturer ID, part of System ID. Will be passed to Device Information Service. */
#define ORG_UNIQUE_ID                   0x111111                        /**< Organizational Unique ID, part of System ID. Will be passed to Device Information Service. */


#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN      /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                               /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                500                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(400, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(401, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */


#define SEC_PARAM_BOND                      1                           /**< Perform bonding. */
#define SEC_PARAM_MITM                      1                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      0                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS                  0                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_DISPLAY_ONLY        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                          /**< Maximum encryption key size. */
#define PASSKEY_LENGTH                      6                           /**< Length of pass-key received by the stack for display. */

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                       /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                 /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                     /**< Advertising module instance. */

static uint16_t   m_conn_handle = BLE_CONN_HANDLE_INVALID;              /**< Handle of the current connection. */
//static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[] =                                       /**< Universally unique service identifier. */
{    
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE},
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};

uint8_t inputEnd = 1, inputSize = 0;
uint16_t msgSize = 0;
uint8_t msgType = 0;

static void advertising_start(void* p_context);

static void gap_params_init(void) {

    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
        (const uint8_t*)DEVICE_NAME,
        strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_WATCH);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval   = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval   = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency       = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout    = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void nrf_qwr_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

static void nus_data_handler(ble_nus_evt_t* p_evt) {

    uint8_t msgStart = 0;

    switch (p_evt->type) {
        case BLE_NUS_EVT_RX_DATA:
        {
            // rx[0] = 0x01 - init
            // rx[1] = 0xxx - message type
            // rx[2] = 0x00 - message size
            // rx[3] = 0x00 - message size - header
            // rx[4] = 0xxx - message
            // rx[..] = 0xxx - message

            // New Message ?
            if ( inputEnd == 1 && p_evt->params.rx_data.p_data[0] == COMMAND_BASE ) {
                inputEnd = 0;
                msgType = p_evt->params.rx_data.p_data[1];
                msgSize = p_evt->params.rx_data.p_data[2];            
                msgStart = 4;
            }

            for (uint32_t i = msgStart; i < p_evt->params.rx_data.length; i++) {
                inputBuffer[inputSize++] = p_evt->params.rx_data.p_data[i];
            }

            if ( inputSize >= msgSize && inputEnd == 0 ) {
                inputEnd = 1;
                inputSize = 0;
                msgSize = 0;
                // call function to evalute received msg
                ble_command(msgType);
                msgType = 0;
            }
            break;
        }

        case BLE_NUS_EVT_COMM_STARTED:
            ble_connection();
            break;

        default:
            break;
    }

}

void send_data_ble(uint8_t * data, uint16_t size) {
    uint32_t       err_code;

    uint16_t length = size;

    do {       
        err_code = ble_nus_data_send(&m_nus, data, (uint16_t*)&length, m_conn_handle);
        /*if ((err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_NOT_FOUND))
        {
            APP_ERROR_CHECK(err_code);
        }*/
    } while (err_code == NRF_ERROR_RESOURCES);

    //APP_ERROR_CHECK(err_code);

}

static void services_init(void) {

    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    ble_dis_init_t     dis_init;
    ble_dis_sys_id_t   sys_id;
    nrf_ble_qwr_init_t qwr_init = { 0 };

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.model_num_str, MODEL_NUM);

    sys_id.manufacturer_id            = MANUFACTURER_ID;
    sys_id.organizationally_unique_id = ORG_UNIQUE_ID;
    dis_init.p_sys_id                 = &sys_id;

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}

static void on_conn_params_evt(ble_conn_params_evt_t* p_evt) {
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

static void conn_params_init(void) {

    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
    //uint32_t err_code;

    switch (ble_adv_evt) 
    {
        case BLE_ADV_EVT_FAST:

            break;
        case BLE_ADV_EVT_IDLE:
            advertising_start(NULL);
            break;
        
        default:
            break;
    }
}

static void ble_evt_handler(ble_evt_t const* p_ble_evt, void* p_context) {
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            //NRF_LOG_INFO("Connected");
            pinetimecos.bluetoothState = StatusON;
            app_push_message(UpdateBleConnection);
            app_push_message(WakeUp);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            //NRF_LOG_INFO("Disconnected");
            pinetimecos.bluetoothState = StatusOFF;
            app_push_message(UpdateBleConnection);
            app_push_message(WakeUp);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            //advertising_start(NULL);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported.
            //err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            //APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            //err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            //APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PASSKEY_DISPLAY:
        {
            memcpy(pinetimecos.passkey, p_ble_evt->evt.gap_evt.params.passkey_display.passkey, PASSKEY_LENGTH);
            pinetimecos.passkey[PASSKEY_LENGTH] = 0;

            app_push_message(ShowPasskey);
            app_push_message(WakeUp);

            //NRF_LOG_INFO("Passkey: %s", nrf_log_push(passkey));
        } break;
        
        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            //NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST");
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            /*NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
                          p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                          p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                          p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                          *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                          *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));*/
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void ble_stack_init(void) {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    pinetimecos.app_ram_base = ram_start;

    err_code = sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    APP_ERROR_CHECK(err_code);

    err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


void gatt_init(void) {
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);

    //err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GAP_DATA_LENGTH);
    //APP_ERROR_CHECK(err_code);    
}

static void advertising_init(void) {
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void delete_bonds(void)
{
    ret_code_t err_code;

    //NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

static void advertising_start(void* p_context) {
    UNUSED_PARAMETER(p_context);

    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}


static void pm_evt_handler(pm_evt_t const * p_evt)
{

    pm_handler_on_pm_evt(p_evt);
    pm_handler_disconnect_on_sec_failure(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(NULL);
            break;

        default:
            break;
    }
}

static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


void nrf_ble_init(void) {

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    
    peer_manager_init();

    nrf_sdh_freertos_init(advertising_start, NULL);
}

