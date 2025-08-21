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
        .type = BLE_GATT_SVC_TYPE_PRIMARY,  
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),  
        .characteristics = (struct ble_gatt_chr_def[]){  
            {
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),  
                .access_cb = gatt_svr_chr_access_device_info,  
                .flags = BLE_GATT_CHR_F_READ, 
            },
            {
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),  
                .access_cb = gatt_svr_chr_access_device_info,  
                .flags = BLE_GATT_CHR_F_READ,  
            },
            { 0 }  // 结束标志
        }
    },
    { 0 }  // 结束标志
};


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
            return BLE_ATT_ERR_UNLIKELY;  
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
        }
    }
    return 0;  
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
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;  
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;  

    rc = ble_gap_adv_start(blehr_addr_type, NULL, BLE_HS_FOREVER, &adv_params, blehr_gap_event, NULL);
    if (rc != 0) {  // 如果广告启动失败
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);  // 输出错误信息
        return;  // 退出函数
    }
}

/* 处理 GAP 事件 */
static int blehr_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {  // 根据事件类型进行不同的处理
        case BLE_GAP_EVENT_CONNECT:  // 处理连接事件
        {
            // 连接事件处理：成功或失败
            MODLOG_DFLT(INFO, "connection %s; status=%d\n",
                        event->connect.status == 0 ? "established" : "failed",
                        event->connect.status);

            if (event->connect.status != 0) {  // 如果连接失败
                // 连接失败时，恢复广播，继续等待其他设备的连接
                blehr_advertise();
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
            MODLOG_DFLT(INFO, "disconnect; reason=%d\n", event->disconnect.reason);

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
            MODLOG_DFLT(INFO, "adv complete\n");  
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
            } else {  // 其他订阅事件
                notify_state = event->subscribe.cur_notify;  // 更新当前通知状态
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
    rc = ble_hs_id_copy_addr(blehr_addr_type, addr_val, NULL);

    // 打印设备地址
    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");

    /* 开始 BLE 广播 */
    blehr_advertise();
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
    
    nimble_port_run();
    nimble_port_freertos_deinit();
}



void user_ble_init(void)
{
    int rc;
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
        return;
    }
    ble_hs_cfg.sync_cb = blehr_on_sync;
    ble_hs_cfg.reset_cb = blehr_on_reset;
    rc = gatt_svr_init();
    assert(rc == 0); // 确保 GATT 服务器初始化成功
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0); // 确保设备名称设置成功
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
