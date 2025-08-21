/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_BLEHR_SENSOR_
#define H_BLEHR_SENSOR_

// #include "nimble/ble.h"
// #include "modlog/modlog.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_HW "0.0.1"
#define DEVICE_SW "0.0.4"

/* Heart-rate configuration */
#define GATT_HRS_UUID                           0x180D
#define GATT_HRS_MEASUREMENT_UUID               0x2A37
#define GATT_HRS_BODY_SENSOR_LOC_UUID           0x2A38
#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

extern uint16_t hrs_hrm_handle;
extern bool notify_state;
extern uint16_t conn_handle;

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);
void user_ble_init(void);
int user_send_notify(char* data, size_t data_size);

static uint8_t command_parameter[16] =
{0xa6,0xc3,0xca,0x4b,0x9c,0x1c,0x46,0xb8,0x58,0xe9,0x4b,0xd1,0xd2,0xa5,0x56,0xa6}; 

// typedef struct users_ble
// {
//     bool BLE_MD_measure;
//     bool BLE_AW_measure;
//     bool BLE_RD_measure;
//     bool BLE_buzzer;
// }user_ble;

// user_ble *user_ble_info(void);

#ifdef __cplusplus
}
#endif

#endif
