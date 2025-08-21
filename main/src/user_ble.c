#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOSConfig.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "user_ble.h"
#include "host/ble_uuid.h"
#include "esp_bt.h"
#include "user_hal.h"


// static user_ble BLE_flags;
static const char *manuf_name = "LEBREW"; // 制造商名称
static const char *model_num = "TDS"; // 设备型号
uint16_t hrs_hrm_handle; // 心率测量特征的句柄,通知

// 用于访问心率测量特征
static int
gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

// 用于访问设备信息特征（如型号、制造商）
static int
gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);

// GATT 服务定义
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /* 心率服务 */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,  // 服务类型，主服务
        .uuid = BLE_UUID16_DECLARE(GATT_HRS_UUID),  // 心率服务的 UUID
        .characteristics = (struct ble_gatt_chr_def[]){  // 心率服务的特征定义
            {
                /* 心率测量特征 */
                .uuid = BLE_UUID16_DECLARE(GATT_HRS_MEASUREMENT_UUID),  // 心率测量特征 UUID
                .access_cb = gatt_svr_chr_access_heart_rate,  // 访问回调函数
                .val_handle = &hrs_hrm_handle,  // 特征值句柄
                .flags = BLE_GATT_CHR_F_NOTIFY,  // 该特征支持通知
            },
            {
                /* 体感传感器位置特征 */
                .uuid = BLE_UUID16_DECLARE(GATT_HRS_BODY_SENSOR_LOC_UUID),  // 体感传感器位置特征 UUID
                .access_cb = gatt_svr_chr_access_heart_rate,  // 访问回调函数
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE ,  // 该特征支持读取
            },
            { 0 }  // 结束标志
        }
    },
    {
        /* 设备信息服务 */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,  // 服务类型，主服务
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),  // 设备信息服务的 UUID
        .characteristics = (struct ble_gatt_chr_def[]){  // 设备信息服务的特征定义
            {
                /* 制造商名称特征 */
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),  // 制造商名称特征 UUID
                .access_cb = gatt_svr_chr_access_device_info,  // 访问回调函数
                .flags = BLE_GATT_CHR_F_READ,  // 该特征支持读取
            },
            {
                /* 型号编号特征 */
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),  // 型号编号特征 UUID
                .access_cb = gatt_svr_chr_access_device_info,  // 访问回调函数
                .flags = BLE_GATT_CHR_F_READ,  // 该特征支持读取
            },
            { 0 }  // 结束标志
        }
    },
    { 0 }  // 结束标志
};

/**
 * 处理GATT特征写操作，将从客户端传递来的数据从内存缓冲区（`mbuf`）复制到目标缓冲区。
 * 此函数确保数据的长度在合理范围内，并执行客户端请求的写操作。
 *
 * @param om         存储从客户端接收到的数据的内存缓冲区（`os_mbuf`）。
 * @param min_len    数据包的最小长度要求。如果接收到的数据小于该长度，则会返回错误。
 * @param max_len    数据包的最大长度要求。如果接收到的数据大于该长度，则会返回错误。
 * @param dst        目标缓冲区，数据会被复制到这个位置，通常是GATT服务器中相应特征值的存储位置。
 * @param len        一个指针，存储实际写入数据的长度。
 *
 * @return           0表示成功，非零表示失败。
 *                   返回 `BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN` 表示数据长度无效。
 *                   返回 `BLE_ATT_ERR_UNLIKELY` 表示不可预见的错误。
 */
static int gatt_svr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
               void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;
    // 获取缓冲区中的数据长度
    om_len = OS_MBUF_PKTLEN(om);
    printf("om_len: %d\n",om_len);
    // 检查数据长度是否符合要求
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;  // 如果数据长度不符合要求，返回错误
    }
    else
    {
        // 将数据从mbuf缓冲区复制到目标缓冲区
        rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
        if (rc != 0) {
            return BLE_ATT_ERR_UNLIKELY;  // 如果复制过程出现问题，返回错误
        }
        bool chang_ok = true;
        for (size_t i = 0; i < om_len; i++) {
            if(i<=15)
            {
                if(command_parameter[i] != ((uint8_t *)dst)[i])
                {
                    chang_ok = false;
                    printf("false\n");
                }
            }
            printf("%02x\n", ((uint8_t *)dst)[i]);
        }
        if(chang_ok && om_len >= 17)
        {
            printf("BLE_TESR-----------\n");
            // setup_gui_running();
            // switch (((uint8_t *)dst)[16])
            // {
            // case 1://含水率测量
            //     if(device_ui_info.state.measure_start_MD == 0)
            //     {
            //        gui_task_msg_send(BLE_MD_measure,NULL,0,NULL,0);
            //     //    BLE_flags.BLE_MD_measure = true;
            //     }
            //     break;
            // case 2://水活度测量
            //     if(device_ui_info.state.measure_start_AW == 0)
            //     {
            //        gui_task_msg_send(BLE_AW_measure,NULL,0,NULL,0);
            //     //    BLE_flags.BLE_MD_measure = true;
            //     }
            //     break;   
            // case 3://色度测量
            //     if(device_ui_info.state.measure_start_SCA == 0)
            //     {
            //        gui_task_msg_send(BLE_RD_measure,NULL,0,NULL,0);
            //     //    BLE_flags.BLE_MD_measure = true;
            //     }
            //     break;
            // case 4://开启蜂鸣器
            //     device_ui_info.state.buzzer = 1;
            //     sys_data.buzzer = device_ui_info.state.buzzer;
            //     user_save_sys_evn(&sys_data);
            //     lv_event_send(prj_layer_cont,LV_EVENT_USER_CHANGED,NULL);
            //     break;
            // case 5://关闭蜂鸣器
            //     device_ui_info.state.buzzer = 0;
            //     sys_data.buzzer = device_ui_info.state.buzzer;
            //     user_save_sys_evn(&sys_data);
            //     lv_event_send(prj_layer_cont,LV_EVENT_USER_CHANGED,NULL);        
            //     break;
            // case 6://设置中文
            //     gui_task_msg_send(BLE_LANGUAGE_CN,NULL,0,NULL,0);
            //     break;
            // case 7://设置英文
            //     gui_task_msg_send(BLE_LANGUAGE_EN,NULL,0,NULL,0);
            //     break;
            // case 8://调节亮度
            //     device_ui_info.value.brightness = ((uint8_t *)dst)[17];
            //     sys_data.brightness = device_ui_info.value.brightness;
            //     user_save_sys_evn(&sys_data);
            //     user_set_led_bright(((uint8_t *)dst)[17]);
            //     break;  
            // case 9://息屏时间
            //     device_ui_info.value.auto_sleep_duration = ((uint8_t *)dst)[17];
            //     sys_data.auto_sleep_min = device_ui_info.value.auto_sleep_duration;
            //     user_save_sys_evn(&sys_data);
            //     break; 
            // case 10://含水率校准
            //     printf("device_ui_info.state.calibration_start_MD = %d\n",device_ui_info.state.calibration_start_MD);
            //     if(device_ui_info.state.calibration_start_MD == 0)
            //     {
            //        gui_task_msg_send(BLE_M_zeroing,NULL,0,NULL,0);
            //     }
            //     break;        
            // case 11://密度校准
            //     printf("device_ui_info.state.calibration_start_MD = %d\n",device_ui_info.state.calibration_start_MD);
            //     if(device_ui_info.state.calibration_start_MD == 0)
            //     {
            //        gui_task_msg_send(BLE_D_zeroing,NULL,0,NULL,0);
            //     }
            //     break;   
            // case 12://水活校准
            //     if(device_ui_info.state.calibration_start_AW == 0)
            //     {
            //        gui_task_msg_send(BLE_AW_zeroing,NULL,0,NULL,0);
            //     }
            //     break;   
            // case 13://色度校准
            //     if(device_ui_info.state.calibration_start_SCA == 0)
            //     {
            //        gui_task_msg_send(BLE_RD_zeroing,NULL,0,NULL,0);
            //     }
            //     break;  
            // case 14://开启wifi
            //     sys_data.work_mode = APP_MODE_FACTORY;
            //     user_save_sys_evn(&sys_data);
            //     // esp_restart();
            //     gui_task_msg_send(USER_TEST_IN_EVT,NULL,0,NULL,0);
            //         // 1. 停止主机任务
            //     // tcp_client_init();
            //     printf("--------=----------++++++++++++++\n");
            //     // int stop = nimble_port_stop();
            //     // if(stop!=1)
            //     // {
            //     //     printf("--------=----------++++++++++++++\n");
            //     //     esp_err_t ret = nimble_port_deinit();
            //     //     gui_task_msg_send(USER_TEST_IN_EVT,NULL,0,NULL,0);
            //     // }
            //     break; 
            // case 15://更改WiFi
            //     printf("Change WiFi\n");
            //     char *hex_str = ((char *)dst) + 17;
            //     printf("hex_str: %s\n", hex_str); // 检查HEX部分

            //     char *command = strtok(hex_str,";");
            //     while(command != NULL)
            //     {
            //         if (strncmp(command, "WIFI:", 5) == 0) {
            //             sscanf(command+5,"%s",sys_data.WIFI_name);
            //             printf("WIFI: %s\n",sys_data.WIFI_name);
            //             user_save_sys_evn(&sys_data);
            //         }
            //         else if(strncmp(command, "SD:", 3) == 0){
            //             sscanf(command+3,"%s",sys_data.WIFI_sd);
            //             printf("SD: %s\n",sys_data.WIFI_name);
            //             user_save_sys_evn(&sys_data);
            //         }
            //         command = strtok(NULL, ";");
            //     }
            //     break;    
            // default:
            //     break;
            // }
        }
    }
    return 0;  // 数据复制成功，返回成功
}

// 处理心率测量特征的访问
static int gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("GATT_HRS_BODY_SENSOR_LOC_UUID\n");
    // static uint8_t body_sens_loc = 0x01;  // 模拟的体感传感器位置（胸部）

    uint16_t uuid;
    int rc;
    uuid = ble_uuid_u16(ctxt->chr->uuid);  // 获取特征的 UUID
        switch (ctxt->op)
        {
        case BLE_GATT_ACCESS_OP_READ_CHR:// 读取特征值的操作
            if (uuid == GATT_HRS_BODY_SENSOR_LOC_UUID) {  
                char HWSW_version[16];
                sprintf(HWSW_version, "HW%sSW%s", DEVICE_HW, DEVICE_SW);
                rc = os_mbuf_append(ctxt->om, &HWSW_version, strlen(HWSW_version));
                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;  // 返回访问结果
            }
        break;
        case BLE_GATT_ACCESS_OP_WRITE_CHR:// 写入特征值的操作
            if (uuid == GATT_HRS_BODY_SENSOR_LOC_UUID) {
                if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
                    MODLOG_DFLT(INFO, "Characteristic write; conn_handle=%d attr_handle=%d", conn_handle, attr_handle);
                } else {
                    MODLOG_DFLT(INFO, "Characteristic write by NimBLE stack; attr_handle=%d", attr_handle);
                }
                uint8_t  gatt_svr_chr_val[128] = {0};
                rc = gatt_svr_write(ctxt->om, sizeof(uint8_t), sizeof(gatt_svr_chr_val), &gatt_svr_chr_val, NULL);
                ble_gatts_chr_updated(attr_handle);
                return rc;  // 返回操作结果
            }
        break;
        
        default:
            break;
        }

    assert(0);  // 如果是未知的特征，触发断言
    return BLE_ATT_ERR_UNLIKELY;
}

// 处理设备信息特征的访问
static int
gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);  // 获取特征的 UUID

    if (uuid == GATT_MODEL_NUMBER_UUID) {  // 如果访问的是型号编号特征
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));  // 将型号编号添加到输出缓冲区
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;  // 返回访问结果
    }

    if (uuid == GATT_MANUFACTURER_NAME_UUID) {  // 如果访问的是制造商名称特征
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));  // 将制造商名称添加到输出缓冲区
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;  // 返回访问结果
    }

    assert(0);  // 如果是未知的特征，触发断言
    return BLE_ATT_ERR_UNLIKELY;
}

// GATT 服务注册回调函数
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];  // 用于存储 UUID 的缓冲区
    printf("gatt_svr_register_cb\n");
    switch (ctxt->op) {  // 根据操作类型来处理不同的情况
    case BLE_GATT_REGISTER_OP_SVC://服务的注册操作
        // 注册服务操作
        // 打印服务的 UUID 和句柄
        // MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
        //             ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),  // 获取并打印服务的 UUID
        //             ctxt->svc.handle);  // 打印服务的句柄
        printf("registered service %s with handle=%d\n",ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
            ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR://特征的注册操作
        // 注册特征操作
        // 打印特征的 UUID、定义句柄和特征值句柄
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),  // 获取并打印特征的 UUID
                    ctxt->chr.def_handle,  // 打印特征定义句柄
                    ctxt->chr.val_handle);  // 打印特征值句柄
        printf("registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
            ctxt->chr.def_handle,
            ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        // 注册描述符操作
        // 打印描述符的 UUID 和句柄
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),  // 获取并打印描述符的 UUID
                    ctxt->dsc.handle);  // 打印描述符的句柄
        break;

    default:
        // 如果遇到未知的操作类型，触发断言
        assert(0);  // 触发断言，表示遇到未知操作
        break;
    }
}


// 初始化 GATT 服务
int
gatt_svr_init(void)
{
    int rc;

    // 初始化 GAP 和 GATT 服务
    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);  // 计算服务配置
    if (rc != 0) {
        return rc;  // 返回错误代码
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);  // 注册GATT添加服务，触发gatt_svr_register_cb回调
    if (rc != 0) {
        return rc;  // 返回错误代码
    }

    return 0;  // 返回成功
}



static const char *tag = "USER_BLE";

// 定义一个定时器句柄用于周期性地发送心率数据
// static TimerHandle_t blehr_tx_timer;

// BLE通知状态，表示客户端是否订阅通知
bool notify_state;

// 连接句柄，用于识别与客户端的连接
uint16_t conn_handle;

// 设备名称
static const char *device_name = "LEBREW TDS";

// 声明处理 GAP 事件的回调函数
static int blehr_gap_event(struct ble_gap_event *event, void *arg);

// BLE 地址类型
static uint8_t blehr_addr_type;

// 用于模拟心跳的变量
// static uint8_t heartrate = 90;

/**
 * Utility function to log an array of bytes.
 * 用于打印字节数组的辅助函数
 */
void print_bytes(const uint8_t *bytes, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        MODLOG_DFLT(INFO, "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

/**
 * Utility function to print BLE address.
 * 打印设备的BLE地址
 */
void print_addr(const void *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

/*
 * 启动广播，使用以下参数：
 *     o 通用可发现模式
 *     o 无定向连接模式
 */
static void blehr_advertise(void)
{
    struct ble_gap_adv_params adv_params;  // 广播参数结构体，用于设置广播的相关参数
    struct ble_hs_adv_fields fields;  // 广播字段结构体，用于设置广播数据内容
    int rc;
    uint8_t custom_data[4] = {0x01, 0x02, 0x03, 0x04};
    // 设置广告数据，包括标志、发射功率和设备名称
    memset(&fields, 0, sizeof(fields));  // 清零字段结构体，确保没有脏数据

    // 设置广告标志：标志字段指定广告的类型及其特性
    // BLE_HS_ADV_F_DISC_GEN: 设置为通用可发现性广告模式（设备可以被发现）
    // BLE_HS_ADV_F_BREDR_UNSUP: 指定设备不支持 BR/EDR（传统蓝牙）
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    // 设置发射功率级别字段：指定广告数据中是否包含发射功率级别
    fields.tx_pwr_lvl_is_present = 1;  // 设置为1表示包含发射功率级别
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;  // 自动计算并设置发射功率级别

    // 设置设备名称字段：
    fields.name = (uint8_t *)device_name;  // 设置设备名称，类型转换为 uint8_t * 指向字符串
    fields.name_len = strlen(device_name);  // 设置设备名称长度
    fields.mfg_data = (uint8_t *)custom_data;
    fields.mfg_data_len = sizeof(custom_data);
    fields.name_is_complete = 1;  // 表示设备名称已经完整

    // 设置广播字段
    rc = ble_gap_adv_set_fields(&fields);  // 调用 API 设置广播字段
    if (rc != 0) {  // 如果设置广播字段失败
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);  // 输出错误信息
        return;  // 退出函数
    }

    // 启动广告
    memset(&adv_params, 0, sizeof(adv_params));  // 清零广告参数结构体

    // 设置广告参数：
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;  // 设置为无定向连接模式（客户端可以随时连接）
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;  // 设置为通用发现模式（设备对任何设备可见）

    // 启动广告
    //指向 BLE GAP 事件回调函数 的指针。当广告过程完成时，或者有连接建立时，回调函数会被调用来处理相应的事件
    rc = ble_gap_adv_start(blehr_addr_type, NULL, BLE_HS_FOREVER, &adv_params, blehr_gap_event, NULL);
    if (rc != 0) {  // 如果广告启动失败
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);  // 输出错误信息
        return;  // 退出函数
    }
}


// /* 停止心率数据传输定时器 */
// static void blehr_tx_hrate_stop(void)
// {
//     xTimerStop( blehr_tx_timer, 1000 / portTICK_PERIOD_MS );
// }

// /* 重置心率测量 */
// static void blehr_tx_hrate_reset(void)
// {
//     int rc;

//     // 重置定时器
//     if (xTimerReset(blehr_tx_timer, 1000 / portTICK_PERIOD_MS ) == pdPASS) {
//         rc = 0;
//     } else {
//         rc = 1;
//     }

//     assert(rc == 0);
// }

// /* 模拟心跳并将数据通知给客户端 */
// static void blehr_tx_hrate(TimerHandle_t ev)
// {
//     static uint8_t hrm[2];
//     int rc;
//     struct os_mbuf *om;

//     if (!notify_state) {
//         // 如果客户端没有订阅通知，停止定时器并重置心率
//         blehr_tx_hrate_stop();
//         heartrate = 90;
//         return;
//     }

//     // 心率数据
//     hrm[0] = 0x06; // 传感器的接触
//     hrm[1] = heartrate; // 模拟的心率数据

//     // 模拟心跳
//     heartrate++;
//     if (heartrate == 160) {
//         heartrate = 90;
//     }

//     // 创建数据包
//     om = ble_hs_mbuf_from_flat(hrm, sizeof(hrm));

//     // 向客户端发送通知
    // rc = ble_gatts_notify_custom(conn_handle, hrs_hrm_handle, om);

//     assert(rc == 0);

//     // 重置心率测量定时器
//     blehr_tx_hrate_reset();
// }

/* 处理 GAP 事件 */
static int blehr_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {  // 根据事件类型进行不同的处理
        case BLE_GAP_EVENT_CONNECT:  // 处理连接事件
        {
            // 连接事件处理：成功或失败
            MODLOG_DFLT(INFO, "connection %s; status=%d\n",
                        event->connect.status == 0 ? "established" : "failed",  // 如果状态为0，表示连接成功，否则连接失败
                        event->connect.status);

            if (event->connect.status != 0) {  // 如果连接失败
                // 连接失败时，恢复广播，继续等待其他设备的连接
                blehr_advertise();
                printf("连接失败，重新广播\n");
            }
            printf("连接成功\n");
            conn_handle = event->connect.conn_handle;  // 保存连接句柄
            UI_icon.ble_icon = true;
            ui_msg_t msg = {
                .type = UI_MSG_UPDATA_WIFI_BLE_ICON,
            };
            xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT:  // 处理断开连接事件
        {
            printf("BLE连接失败--------------------------------------------------\n");
            MODLOG_DFLT(INFO, "disconnect; reason=%d\n", event->disconnect.reason);  // 打印断开连接的原因

            // 断开连接时，恢复广播，等待其他设备的连接
            blehr_advertise();
            UI_icon.ble_icon = false;
            ui_msg_t msg = {
                .type = UI_MSG_UPDATA_WIFI_BLE_ICON,
            };
            xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
            break;
        }
        case BLE_GAP_EVENT_ADV_COMPLETE:  // 处理广告完成事件
        {
            MODLOG_DFLT(INFO, "adv complete\n");  // 打印广告完成的日志

            // 广告完成后重新开始广告，继续广播设备信息
            blehr_advertise();
            break;
        }
        case BLE_GAP_EVENT_SUBSCRIBE:  // 处理订阅事件
        {
            MODLOG_DFLT(INFO, "subscribe event; cur_notify=%d\n value handle; "
                        "val_handle=%d\n",
                        event->subscribe.cur_notify, hrs_hrm_handle);  // 打印当前通知状态和值句柄

            // 处理心率服务（HRM）的订阅事件
            if (event->subscribe.attr_handle == hrs_hrm_handle && event->subscribe.cur_notify) {  // 如果是心率测量的订阅事件
                notify_state = event->subscribe.cur_notify;  // 更新当前通知状态
                // blehr_tx_hrate_reset();  // 重置心率数据传输定时器
            } else {  // 其他订阅事件
                notify_state = event->subscribe.cur_notify;  // 更新当前通知状态
                // blehr_tx_hrate_stop();  // 停止心率数据传输定时器
            }
            ESP_LOGI("BLE_GAP_SUBSCRIBE_EVENT", "conn_handle from subscribe=%d", conn_handle);  // 打印连接句柄
            break;
        }
        case BLE_GAP_EVENT_MTU:  // 处理MTU（最大传输单元）更新事件
        {
            MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d mtu=%d\n",
                        event->mtu.conn_handle,  // 打印连接句柄
                        event->mtu.value);  // 打印更新后的MTU值
            break;
        }
    }

    return 0;  // 事件处理完成后返回
}


// 处理 NimBLE 主机同步事件
static void blehr_on_sync(void)
{
    int rc;

    // 推断自动生成的 BLE 地址类型（如公共地址、随机地址等）
    rc = ble_hs_id_infer_auto(0, &blehr_addr_type);
    assert(rc == 0);  // 确保推断地址类型成功

    uint8_t addr_val[6] = {0};  // 用于存储设备地址的数组
    // 获取推断出的 BLE 地址，并将其存储在 addr_val 中
    rc = ble_hs_id_copy_addr(blehr_addr_type, addr_val, NULL);

    // 打印设备地址
    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);  // 调用 print_addr 函数打印地址
    MODLOG_DFLT(INFO, "\n");

    /* 开始 BLE 广播 */
    blehr_advertise();
    // int err = ble_att_set_preferred_mtu(CONFIG_BT_NIMBLE_ATT_PREFERRED_MTU);
    // if(err){
    //     MODLOG_ERROR(INFO,"MTU defeated");
    // }
    // else{
    //     MODLOG_ERROR(INFO,"MTU seccessful");
    // }
}

// 处理 BLE 设备重置事件
static void blehr_on_reset(int reason)
{
    // 打印重置原因
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

// BLE 主机任务，用于运行 NimBLE 主机栈
void blehr_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    
    /* 该函数将一直运行，直到调用 nimble_port_stop() 停止 NimBLE 主机任务 */
    nimble_port_run();

    // 停止 NimBLE 主机栈并清理资源
    nimble_port_freertos_deinit();
}



void user_ble_init(void)
{
    int rc;

    // /* 初始化 NVS — 用于存储 PHY 校准数据 */
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     // 如果 NVS 没有足够空间或版本不匹配，则擦除并重新初始化 NVS
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret); // 确保 NVS 初始化成功

    /* 初始化 NimBLE 协议栈 */
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        // 如果 NimBLE 初始化失败，打印错误信息并退出
        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
        return;
    }

    /* 配置 NimBLE 主机设置 */
    // 设置同步回调函数，用于处理 BLE 主机栈同步时的操作
    ble_hs_cfg.sync_cb = blehr_on_sync;
    // 设置重置回调函数，用于处理 BLE 主机栈重置时的操作
    ble_hs_cfg.reset_cb = blehr_on_reset;

    /* 创建一个定时器用于周期性发送心率数据 */
    // 每秒调用一次 `blehr_tx_hrate` 函数，传输心率数据
    // blehr_tx_timer = xTimerCreate("blehr_tx_timer", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, blehr_tx_hrate);

    // 初始化 GATT 服务器，注册心率测量服务
    rc = gatt_svr_init();
    assert(rc == 0); // 确保 GATT 服务器初始化成功

    /* 设置设备的默认名称 */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0); // 确保设备名称设置成功

    /* 启动任务，开始处理 BLE 主机事件 */
    nimble_port_freertos_init(blehr_host_task);
}

//发送数据到通知
int user_send_notify(char* data, size_t data_size)
{
    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(data, data_size);
    int err = ble_gatts_notify_custom(conn_handle, hrs_hrm_handle, om);
    return err;
}
