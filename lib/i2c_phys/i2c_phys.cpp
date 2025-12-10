/*
 * i2c_phys.cpp - ESP32 Arduino Port
 *
 * Ported from Atmel SAM D21 ASF to ESP32 Arduino Wire library
 * This file is C++ to interface with Arduino Wire library, but provides C interface
 */

#include <Arduino.h>
#include <Wire.h>
#include "i2c_phys.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C read/write flags (matching aes132_i2c.c enum values)
#define I2C_WRITE_VALUE 0x00
#define I2C_READ_VALUE  0x01

// I2C clock frequency (400kHz)
#define I2C_CLOCK_FREQ 400000

// I2C default address (0xA0)
#define I2C_DEFAULT_ADDRESS   ((uint8_t) 0xA0)

// I2C address currently in use
static uint8_t i2c_address_current = I2C_DEFAULT_ADDRESS;

// I2C pins (SDA, SCL)
static int sda_pin = 21;
static int scl_pin = 22;

/** \brief This function selects a I2C AES132 device.
 *
 * @param[in] device_id I2C address
 * @return always success
 */
uint8_t i2c_select_device_phys(uint8_t device_id)
{
	i2c_address_current = device_id & ~1;
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function initializes and enables the I2C peripheral.
 */
void i2c_enable_phys(void)
{
	Wire.begin(sda_pin, scl_pin);
	Wire.setClock(I2C_CLOCK_FREQ);
}

/** \brief This function disables the I2C peripheral.
 */
void i2c_disable_phys(void)
{
	// Wire.end() is not typically needed for ESP32
	// Empty implementation is intentional - no action needed
}

/** \brief This function creates a Start condition.
 * \return status of the operation
 */
uint8_t i2c_send_start(void)
{
	// In Arduino Wire library, beginTransmission() creates the start condition
	// This function is called before sending data, so we just return success
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function creates a Stop condition.
 * \return status of the operation
 */
uint8_t i2c_send_stop(void)
{
	// In Arduino Wire library, endTransmission() creates the stop condition
	// This function is called after sending data, so we just return success
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

// Flag to track if we need repeated start (for read operations)
static bool need_repeated_start = false;

/** \brief This function sends bytes to an I2C device.
 * \param[in] count number of bytes to send
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
	Wire.beginTransmission(i2c_address_current >> 1);
	
	for (uint8_t i = 0; i < count; i++) {
		Wire.write(data[i]);
	}
	
	// ESP32 Wire library: endTransmission(sendStop) 
	// sendStop = false for repeated start, true for normal stop
	uint8_t status = Wire.endTransmission(!need_repeated_start);
	
	if (status == 0) {
		return I2C_FUNCTION_RETCODE_SUCCESS;
	} else {
		need_repeated_start = false; // Reset on error
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
}

/** \brief This function receives one byte from an I2C device.
 *
 * \param[out] data pointer to received byte
 * \return status of the operation
 */
uint8_t i2c_receive_byte(uint8_t *data)
{
	return i2c_receive_bytes(1, data);
}

/** \brief This function receives bytes from an I2C device.
 *
 * \param[in] count number of bytes to receive
 * \param[out] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data)
{
	// Wire.requestFrom automatically handles repeated start if previous transmission
	// ended without stop
	uint8_t bytes_received = Wire.requestFrom((uint8_t)(i2c_address_current >> 1), (uint8_t)count, (uint8_t)true);
	
	need_repeated_start = false; // Reset flag after read
	
	if (bytes_received != count) {
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	
	for (uint8_t i = 0; i < count; i++) {
		if (Wire.available()) {
			data[i] = Wire.read();
		} else {
			return I2C_FUNCTION_RETCODE_COMM_FAIL;
		}
	}
	
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function creates a Start condition and sends the I2C address.
 * \param[in] read 0x01 for reading, 0x00 for writing
 * \return status of the operation
 */
uint8_t i2c_send_slave_address(uint8_t read)
{
	// Set flag for repeated start if this is a read operation
	// (write address was already sent, now we need read address)
	if (read == I2C_READ_VALUE) {
		need_repeated_start = true;
	} else {
		need_repeated_start = false;
	}
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

/** \brief Set I2C pins (call before i2c_enable_phys)
 * \param[in] sda SDA pin number
 * \param[in] scl SCL pin number
 */
void i2c_set_pins(int sda, int scl)
{
	sda_pin = sda;
	scl_pin = scl;
}

#ifdef __cplusplus
}
#endif

