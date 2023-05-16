// vim: set ft=arduino:
/*
 * tasker_function_ids.h
 * Copyright (C) 2022 Kryštof Havránek <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TASKER_FUNCTION_IDS_H
#define TASKER_FUNCTION_IDS_H

// System tasks 0x01, 0x02, 0x03//
#define TSID_OS_SLEEP_SCHEDULE 0x0501
#define TSID_OS_COMM_MODBUS 0x0601
#define TSID_CONFIG_SERVER 0x1101
#define TSID_CONFIG_SERIAL 0x1201

// Telemetry tasks //
#define TSID_TELEMETRY_SEND_DATA_CRITICAL_TIMEOUT 0xAA01
#define TSID_TELEMETRY_SEND_DATA 0xAB01
#define TSID_TELEMETRY_I2C_SCAN 0xAC01
#define TSID_TELEMETRY_CHECK_SENSOR_TASK 0xAD01


// Sensor readings  - 0x41, 0x43, 0x45//
#define TSID_SENSOR_GENERIC 0x0141

#define TSID_SENSOR_ADS 0x0241
#define TSID_SENSOR_HTU21 0x0741
#define TSID_SENSOR_INA 0x0441
#define TSID_SENSOR_MPU6050 0x0541
#define TSID_SENSOR_RUUVI 0x0641
#define TSID_SENSOR_MCP23017 0x0741
#define TSID_SENSOR_DS248X 0x0841
#define TSID_SENSOR_AS5600 0x0941

#define TSID_SENSOR_SPECIAL 0x0143
#define TSID_SENSOR_AS5600_POS  0x0243
#define TSID_SENSOR_AS5600_PRI  0x0343
#define TSID_SENSOR_DS248X_VAL  0x0543

// Sensor readings special 0x55
#define TSID_WIFI_SNIFFER_LOG 0x0155
#define TSID_RUUVI_SCAN 0x0455


#endif /* !TASKER_FUNCTION_IDS_H */
