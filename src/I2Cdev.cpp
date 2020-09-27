// I2Cdev library collection - Main I2C device class
// Abstracts bit and byte I2C R/W functions into a convenient class
// 2013-06-05 by Jeff Rowberg <jeff@rowberg.net>
//
// Changelog:
//      2013-05-06 - add Francesco Ferrara's Fastwire v0.24 implementation with small modifications
//      2013-05-05 - fix issue with writing bit values to words (Sasquatch/Farzanegan)
//      2012-06-09 - fix major issue with reading > 32 bytes at a time with Arduino Wire
//                 - add compiler warnings when using outdated or IDE or limited I2Cdev implementation
//      2011-11-01 - fix write*Bits mask calculation (thanks sasquatch @ Arduino forums)
//      2011-10-03 - added automatic Arduino version detection for ease of use
//      2011-10-02 - added Gene Knight's NBWire TwoWire class implementation with small modifications
//      2011-08-31 - added support for Arduino 1.0 Wire library (methods are different from 0.x)
//      2011-08-03 - added optional timeout parameter to read* methods to easily change from default
//      2011-08-02 - added support for 16-bit registers
//                 - fixed incorrect Doxygen comments on some methods
//                 - added timeout value for read operations (thanks mem @ Arduino forums)
//      2011-07-30 - changed read/write function structures to return success or byte counts
//                 - made all methods static for multi-device memory savings
//      2011-07-28 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2013 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "I2Cdev.h"
#include "STM32FIX.h"


#ifndef BUFFER_LENGTH
// band-aid fix for platforms without Wire-defined BUFFER_LENGTH (removed from some official implementations)
#define BUFFER_LENGTH 32
#endif

/** Default constructor.
 */
I2Cdev::I2Cdev(){
}

/** Read a single bit from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readBit(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout) {
    uint8_t b;
    uint8_t count = readByte(i2c, devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);
    return count;
}

/** Read a single bit from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-15)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readBitW(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout) {
    uint8_t b;
    uint8_t count = readWord(i2c, devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);//TODO: some problem might occur for changing it to int8
    return count;
}

/** Read multiple bits from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t
I2Cdev::readBits(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = readByte(i2c, devAddr, regAddr, &b, timeout)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

/** Read multiple bits from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-15)
 * @param length Number of bits to read (not more than 16)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (1 = success, 0 = failure, -1 = timeout)
 */
int8_t I2Cdev::readBitsW(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data,
                         uint16_t timeout) {
    // 1101011001101001 read byte
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, length=3
    //    010           masked
    //           -> 010 shifted
    uint8_t count;
    uint8_t w; //TODO:changed to int8
    if ((count = readWord(i2c, devAddr, regAddr, &w, timeout)) != 0) {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        w &= mask;
        w >>= (bitStart - length + 1);
        *data = w;
    }
    return count;
}

/** Read single byte from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readByte(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout) {
    return readBytes(i2c, devAddr, regAddr, 1, data, timeout);
}

/** Read single word from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for word value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readWord(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout) {
    return readWords(i2c, devAddr, regAddr, 1, data, timeout);
}

/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Number of bytes read (-1 indicates failure)
 */
int8_t I2Cdev::readBytes(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout) {
#ifdef I2CDEV_SERIAL_DEBUG
    printf("I2C (0x%x) reading %u words from 0x%x...", devAddr, length, regAddr);
#endif
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, uint16_t(devAddr), uint16_t(regAddr), I2C_MEMADD_SIZE_8BIT, data, uint16_t(length), uint32_t(timeout));
#ifdef I2CDEV_SERIAL_DEBUG
    printf(". Done");
#endif

    if(status == HAL_OK){
        return length;
    }
    return -1;
}

/** Read multiple words from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of words to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Number of words read (-1 indicates failure)
 */
int8_t I2Cdev::readWords(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout) {
#ifdef I2CDEV_SERIAL_DEBUG
    printf("I2C (0x%x) reading %u words from 0x%x...", devAddr, length, regAddr);
#endif
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, uint16_t(devAddr), uint16_t(regAddr), I2C_MEMADD_SIZE_16BIT, data, uint16_t(length), uint32_t(timeout));
#ifdef I2CDEV_SERIAL_DEBUG
    printf(". Done");
#endif

    if(status == HAL_OK){
        return length;
    }
    return -1;
}

/** write a single bit in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBit(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    readByte(i2c, devAddr, regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return writeByte(i2c, devAddr, regAddr, b);
}

/** write a single bit in a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-15)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBitW(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data) {
    uint8_t w;//TODO:changed to int8
    readWord(i2c, devAddr, regAddr, &w);
    w = (data != 0) ? (w | (1 << bitNum)) : (w & ~(1 << bitNum));
    return writeWord(i2c, devAddr, regAddr, w);
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBits(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b;
    if (readByte(i2c, devAddr, regAddr, &b) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return writeByte(i2c, devAddr, regAddr, b);
    } else {
        return false;
    }
}

/** Write multiple bits in a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-15)
 * @param length Number of bits to write (not more than 16)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBitsW(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data) {
    //              010 value to write
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, length=3
    // 0001110000000000 mask word
    // 1010111110010110 original value (sample)
    // 1010001110010110 original & ~mask
    // 1010101110010110 masked | value
    uint8_t w; //todo: swiched to int8 potential problems
    if (readWord(i2c, devAddr, regAddr, &w) != 0) {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        w &= ~(mask); // zero all important bits in existing word
        w |= data; // combine data with existing word
        return writeWord(i2c, devAddr, regAddr, w);
    } else {
        return false;
    }
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeByte(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    return writeBytes(i2c, devAddr, regAddr, 1, &data);
}

/** Write single word to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New word value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeWord(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    return writeWords(i2c, devAddr, regAddr, 1, &data);
}

/** Write multiple bytes to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBytes(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {
#ifdef I2CDEV_DEBUG
    printf("I2C (0x%x) writing %u words to 0x%x...$", devAddr, length, regAddr);
#endif
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Write(i2c, uint16_t(devAddr), uint16_t(regAddr), I2C_MEMADD_SIZE_8BIT, data, uint16_t(length), uint32_t(0));
    //note some issue might show up for devAddr, try <<1 if it shows up

#ifdef I2CDEV_DEBUG
    printf(". Done.");
#endif
    return status == HAL_OK;
}

/** Write multiple words to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of words to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeWords(I2C_HandleTypeDef* i2c, uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {
#ifdef I2CDEV_DEBUG
    printf("I2C (0x%x) writing %u words to 0x%x...$", devAddr, length, regAddr);
#endif

    HAL_StatusTypeDef status;
    //Need to figure out wtf is wrong with the PID that let them use 16 bits
    status = HAL_I2C_Mem_Write(i2c, uint16_t(devAddr), uint16_t(regAddr), I2C_MEMADD_SIZE_16BIT, data, uint16_t(length), uint32_t(0));
    //note some issue might show up for devAddr, try <<1 if it shows up

#ifdef I2CDEV_DEBUG
    printf(". Done.");
#endif
    return status == HAL_OK;
}

/** Default timeout value for read operations.
 * Set this to 0 to disable timeout detection.
 */
uint16_t I2Cdev::readTimeout = I2CDEV_DEFAULT_READ_TIMEOUT;


