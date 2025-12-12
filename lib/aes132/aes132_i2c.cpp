// ----------------------------------------------------------------------------
//         ATMEL Crypto-Devices Software Support  -  Colorado Springs, CO -
// ----------------------------------------------------------------------------
// DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
// DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

/** \file
 *  \brief 	This file contains implementations of I2C functions.
 *  \author Atmel Crypto Products
 *  \date 	June 16, 2011
 */

#include <stdint.h>                    //!< C type definitions
#include <string.h>
#include <Wire.h> // Required for Arduino Wire library functions
#include <Arduino.h> // For Serial output

#include "aes132_i2c.h"                //!< I2C library definitions
#include "i2c_phys.h"                  //!< I2C physical layer (from i2c_phys library)

/** \brief These enumerations are flags for I2C read or write addressing. */
enum aes132_i2c_read_write_flag {
	I2C_WRITE = (uint8_t) 0x00,	//!< write command id
	I2C_READ  = (uint8_t) 0x01   //!< read command id
};

/** \brief This function initializes and enables the I2C hardware peripheral. */
void aes132p_enable_interface(void)
{
	i2c_enable_phys();
}


/** \brief This function disables the I2C hardware peripheral. */
void aes132p_disable_interface(void)
{
	i2c_disable_phys();
}


/** \brief This function selects a I2C AES132 device.
 *
 * @param[in] device_id I2C address
 * @return always success
 */
uint8_t aes132p_select_device(uint8_t device_id)
{
	return i2c_select_device_phys(device_id);
}


/** \brief This function writes bytes to the device.
 * \param[in] count number of bytes to write
 * \param[in] word_address word address to write to
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t aes132p_write_memory_physical(uint8_t count, uint16_t word_address, uint8_t *data)
{
	Wire.beginTransmission(i2c_address_current >> 1);
	Wire.write((uint8_t) (word_address >> 8)); // MSB
	Wire.write((uint8_t) (word_address & 0xFF)); // LSB

	for (uint8_t i = 0; i < count; i++) {
		Wire.write(data[i]);
	}

	uint8_t status = Wire.endTransmission();

	if (status == 0) {
		return AES132_FUNCTION_RETCODE_SUCCESS;
	} else {
		return AES132_FUNCTION_RETCODE_COMM_FAIL;
	}
}


/** \brief This function reads bytes from the device.
 * \param[in] size number of bytes to read
 * \param[in] word_address word address to read from
 * \param[out] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t aes132p_read_memory_physical(uint8_t size, uint16_t word_address, uint8_t *data)
{
	// #region agent log
	Serial.print("[DEBUG] read_memory_physical: size=");
	Serial.print(size);
	Serial.print(", word_addr=0x");
	Serial.print(word_address, HEX);
	Serial.print(", i2c_addr=0x");
	Serial.println(i2c_address_current, HEX);
	Serial.flush(); // Ensure log is sent immediately
	// #endregion
	
	Wire.beginTransmission(i2c_address_current >> 1);
	Wire.write((uint8_t) (word_address >> 8)); // MSB
	Wire.write((uint8_t) (word_address & 0xFF)); // LSB
	uint8_t status = Wire.endTransmission(false); // Send repeated start

	if (status != 0) {
		return AES132_FUNCTION_RETCODE_COMM_FAIL;
	}

	// Clear any leftover data in Wire buffer before requesting
	// ESP32 Wire library may have leftover data from previous failed operations
	while (Wire.available()) {
		Wire.read(); // Discard any leftover bytes
	}

	// Use size_t to properly handle return value from Wire.requestFrom
	// ESP32 Wire.requestFrom returns size_t, and can return 0 on failure
	// Use 3-parameter version with stop=true to ensure proper I2C stop condition
	size_t bytes_received_size = Wire.requestFrom((uint8_t)(i2c_address_current >> 1), (size_t)size, true);
	
	// Check for failure: bytes_received_size should equal size
	// ESP32 Wire.requestFrom returns 0 on failure, or the actual number of bytes received
	if (bytes_received_size == 0) {
		return AES132_FUNCTION_RETCODE_COMM_FAIL;
	}

	// Verify Wire.available() matches expected bytes
	// This is critical - Wire.available() must match bytes_received_size
	int available = Wire.available();
	if (available != (int)bytes_received_size || available != (int)size) {
		// Clear buffer on mismatch
		while (Wire.available()) {
			Wire.read();
		}
		return AES132_FUNCTION_RETCODE_COMM_FAIL;
	}

	// Read all available bytes - we've already verified Wire.available() == size
	for (uint8_t i = 0; i < size; i++) {
		if (!Wire.available()) {
			// Clear remaining buffer
			while (Wire.available()) {
				Wire.read();
			}
			return AES132_FUNCTION_RETCODE_COMM_FAIL;
		}
		data[i] = Wire.read();
	}
	
	// Verify all bytes were read
	if (Wire.available() != 0) {
		// Clear remaining buffer
		while (Wire.available()) {
			Wire.read();
		}
		return AES132_FUNCTION_RETCODE_COMM_FAIL;
	}
	
	return AES132_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function resynchronizes communication.
 * \return status of the operation
 */
uint8_t aes132p_resync_physical(void)
{
	uint8_t nine_clocks = 0xFF;
	uint8_t n_retries = 2;
	uint8_t aes132_lib_return;

	do {
		aes132_lib_return = i2c_send_start();
		if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS) {
			// If a device is holding SDA or SCL, disabling and
			// re-enabling the I2C peripheral might help.
			i2c_disable_phys();
			i2c_enable_phys();
		}
		if (--n_retries == 0)
			return aes132_lib_return;

		// Retry creating a Start condition if it failed.
	} while(aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS);

	// Do not evaluate the return code which most likely indicates error,
	// since nine_clocks is unlikely to be acknowledged.
	(void) i2c_send_bytes(1, &nine_clocks);

	return i2c_send_stop();
}
