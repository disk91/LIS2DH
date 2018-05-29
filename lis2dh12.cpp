/** Based on ST MicroElectronics LIS2DH datasheet http://www.st.com/web/en/resource/technical/document/datasheet/DM00042751.pdf
* 30/09/2014 by Conor Forde <me@conorforde.com>
* Updates should be available at https://github.com/Snowda/LIS2DH
*
* Changelog:
*     ... - ongoing development release
*     ... - May 2018 - Update by Disk91 / Paul Pinault to make it working
*           maintained at https://github.com/disk91/LIS2DH
* NOTE: THIS IS ONLY A PARIAL RELEASE. 
* THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE DEVELOPMENT AND IS MISSING MOST FEATURES. 
* PLEASE KEEP THIS IN MIND IF YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.
*/
#include "Arduino.h"
#include "stdint.h"
#include "lis2dh12.h"
#include "Wire.h"

/**
 * Set the LIS2DH chip address based on the configuration selected for
 * SA0 pin.
 */
LIS2DH::LIS2DH( uint8_t sa0 ) {
  switch ( sa0 ) {
    case LOW :
        _address = LIS2DH_LIS2DH_SA0_LOW;    
        break;
    case HIGH :
        _address = LIS2DH_LIS2DH_SA0_HIGH;
        break;    
    default:
        _address = LIS2DH_LIS2DH_SA0_LOW;    
  }
}


bool LIS2DH::init(void) {
    bool ret = true;
    Wire.begin(); 
    if ( this->whoAmI() ) {
      // connexion success
      // set a default configuration with 10Hz - Normal mode (full resolution)
      ret &= this->setDataRate(LIS2DH_ODR_10HZ);
      ret &= this->disableLowPower();
      // enable all the axis
      ret &= this->enableAxisXYZ();

      // set highpass filter (Reset when reading xhlREFERENCE register)
      // activate High Pass filter on Output and Int1
      ret &= this->setHPFilterMode(LIS2DH_HPM_NORMAL_RESET);
      ret &= this->setHPFilterCutOff(LIS2DH_HPCF_ODR_50); // 0.2Hz high pass Fcut
      ret &= this->EnableHPFDS();
      ret &= this->EnableHPIA1();
      ret &= this->EnableHPClick();

      // shutdown interrupt
      ret &= this->writeRegister(LIS2DH_CTRL_REG3,LIS2DH_I1_INTERRUPT_NONE);
      
    } else return false;
        
}



// -----------------------------------------------------
// Data Access
// -----------------------------------------------------


/**
 * Return true if the WHOAMI register returned th expected Value
 * 0x33
 */
bool LIS2DH::whoAmI(void) {
    return (LIS2DH_I_AM_VALUE == readRegister(LIS2DH_WHO_AM_I));
}

/** Read the X axis registers
 * @see LIS2DH_OUT_X_H
 * @see LIS2DH_OUT_X_L
 */
int16_t LIS2DH::getAxisX(void) {
  return readRegisters(LIS2DH_OUT_X_H, LIS2DH_OUT_X_L);
}


/** Read the Y axis registers
 * @see LIS2DH_OUT_Y_H
 * @see LIS2DH_OUT_Y_L
 */
int16_t LIS2DH::getAxisY(void) {
  return readRegisters(LIS2DH_OUT_Y_H, LIS2DH_OUT_Y_L);
}

/** Read the Z axis registers
 * @see LIS2DH_OUT_Z_H
 * @see LIS2DH_OUT_Z_L
 */
int16_t LIS2DH::getAxisZ(void) {
  return readRegisters(LIS2DH_OUT_Z_H, LIS2DH_OUT_Z_L);
}

/** Read the all axis registers
 * @see getAxisZ()
 * @see getAxisY()
 * @see getAxisZ()
 */
void LIS2DH::getMotion(int16_t* ax, int16_t* ay, int16_t* az) {
    *ax = getAxisX();
    *ay = getAxisY();
    *az = getAxisZ();
}




// ======= Temperature

bool LIS2DH::getTempEnabled(void) {
    return (readMaskedRegister(LIS2DH_TEMP_CFG_REG, LIS2DH_TEMP_EN_MASK) != 0);
}

bool LIS2DH::setTempEnabled(bool enable) {
    return writeRegister(LIS2DH_TEMP_CFG_REG, enable ? LIS2DH_TEMP_EN_MASK : 0);
}

uint16_t LIS2DH::getTemperature(void) {
    if(tempDataAvailable()){
        return readRegisters(LIS2DH_OUT_TEMP_H, LIS2DH_OUT_TEMP_L);
    } else {
        //if new data isn't available
        return 0;
    }
}

bool LIS2DH::tempHasOverrun(void) {
    uint8_t overrun = readMaskedRegister(LIS2DH_STATUS_REG_AUX, LIS2DH_TOR_MASK);
    return (overrun != 0);
}

bool LIS2DH::tempDataAvailable(void) {
    uint8_t data = readMaskedRegister(LIS2DH_STATUS_REG_AUX, LIS2DH_TDA_MASK);
    return (data != 0);
}


// ======== Data Rate 

uint8_t LIS2DH::getDataRate(void) {
    return readMaskedRegister(LIS2DH_CTRL_REG1, LIS2DH_ODR_MASK);
}

bool LIS2DH::setDataRate(uint8_t data_rate) {
    if ( data_rate > LIS2DH_ODR_MAXVALUE ) return false;
    data_rate <<= LIS2DH_ODR_SHIFT;
    return writeMaskedRegisterI(LIS2DH_CTRL_REG1, LIS2DH_ODR_MASK, data_rate);
}

// ========= Power Management

bool LIS2DH::enableLowPower(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK, true);
}


bool LIS2DH::disableLowPower(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK, false);
}


bool LIS2DH::isLowPowerEnabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK) != 0);
}

// ========== Axis management

bool LIS2DH::enableAxisX(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK, true);
}

bool LIS2DH::disableAxisX(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK, false);
}

bool LIS2DH::isXAxisEnabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK) != 0);
}

bool LIS2DH::enableAxisY(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK, true);
}

bool LIS2DH::disableAxisY(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK, false);
}

bool LIS2DH::isYAxisEnabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK) != 0);
}

bool LIS2DH::enableAxisZ(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK, true);
}

bool LIS2DH::disableAxisZ(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK, false);
}

bool LIS2DH::isZAxisEnabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK) != 0);
}

bool LIS2DH::enableAxisXYZ(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_XYZ_EN_MASK, true);
}

bool LIS2DH::disableAxisXYZ(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG1, LIS2DH_XYZ_EN_MASK, false);
}

// ====== HIGH Pass filter mode
// see http://www.st.com/content/ccc/resource/technical/document/application_note/60/52/bd/69/28/f4/48/2b/DM00165265.pdf/files/DM00165265.pdf/jcr:content/translations/en.DM00165265.pdf

/*
 * Enable/Disable High Pass Filter standard output
 */
bool LIS2DH::EnableHPFDS(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK, true);
}

bool LIS2DH::disableHPFDS(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK, false);
}

bool LIS2DH::isHPFDSEnabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK) != 0);
}

/**
 * High Pass filter allow to remove the continous component to only keep the
 * dynamic acceleration => basically it remove the gravity... as any stable 
 * acceleration
 */
bool LIS2DH::getHPFilterMode(uint8_t mode) {
    return readMaskedRegister(LIS2DH_CTRL_REG2, LIS2DH_HPM_MASK);
}

bool LIS2DH::setHPFilterMode(uint8_t mode) {
    if(mode > LIS2DH_HPM_MAXVALUE) {
        return false;
    }
    uint8_t filter_mode = mode << LIS2DH_HPM_SHIFT;
    return writeMaskedRegisterI(LIS2DH_CTRL_REG2, LIS2DH_HPM_MASK, filter_mode);
}

/*
 * Cut-Off Frequency
 * the reference is the ODR frequency (acquisition frequency), the Cut Off
 * frequency is this frequency divide par a given number from 9 to 400
 */
bool LIS2DH::getHPFilterCutOff(uint8_t mode) {
    return readMaskedRegister(LIS2DH_CTRL_REG2, LIS2DH_HPM_MASK);
}

bool LIS2DH::setHPFilterCutOff(uint8_t mode) {
    if(mode > LIS2DH_HPCF_MAXVALUE) {
        return false;
    }
    uint8_t fcut = mode << LIS2DH_HPCF_SHIFT;
    return writeMaskedRegisterI(LIS2DH_CTRL_REG2, LIS2DH_HPCF_MASK, fcut);
}

/*
 * Enable/Disable High Pass Filter on Click
 */
bool LIS2DH::EnableHPClick(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_HPCLICK_MASK, true);
}

bool LIS2DH::disableHPClick(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_HPCLICK_MASK, false);
}

bool LIS2DH::isHPClickEnabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG2, LIS2DH_HPCLICK_MASK) != 0);
}

/*
 * Enable/Disable High Pass Filter on Interrupt 2
 */
bool LIS2DH::EnableHPIA2(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_HPIA2_MASK, true);
}

bool LIS2DH::disableHPIA2(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_HPIA2_MASK, false);
}

bool LIS2DH::isHPIA1Enabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG2, LIS2DH_HPIA2_MASK) != 0);
}

/*
 * Enable/Disable High Pass Filter on Interrupt 1
 */
bool LIS2DH::EnableHPIA1(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_HPIA1_MASK, true);
}

bool LIS2DH::disableHPIA1(void) {
    return writeMaskedRegister8(LIS2DH_CTRL_REG2, LIS2DH_HPIA1_MASK, false);
}

bool LIS2DH::isHPIA1Enabled(void) {
    return (readMaskedRegister(LIS2DH_CTRL_REG2, LIS2DH_HPIA1_MASK) != 0);
}

// ========== INTERRUPT MANAGEMENT

/**
 * Enable / Disable an interrupt source on INT1
 */
bool LIS2DH::enableInterruptInt1(uint8_t _int) {
  return this->writeMaskedRegister8(LIS2DH_CTRL_REG3,_int,true);
}

bool LIS2DH::disableInterruptInt1(uint8_t _int) {
  return this->writeMaskedRegister8(LIS2DH_CTRL_REG3,_int,false);
}

// -----------------------------------------------------
// Write to LIS2DH
// -----------------------------------------------------

/**
 * Write a 8b register on the chip
 */
bool LIS2DH::writeRegister(const uint8_t register_addr, const uint8_t value) {
    Wire.beginTransmission(_address); //open communication with 
    Wire.write(register_addr);  
    Wire.write(value); 
    return Wire.endTransmission(); 
}

/**
 * Write a 16b register
 */
bool LIS2DH::writeRegisters(const uint8_t msb_register, const uint8_t msb_value, const uint8_t lsb_register, const uint8_t lsb_value) { 
    //send write call to sensor address
    //send register address to sensor
    //send value to register
    bool msb_bool, lsb_bool;
    msb_bool = writeRegister(msb_register, msb_value);
    lsb_bool = writeRegister(lsb_register, lsb_value);
    return msb_bool & lsb_bool; 
}

/**
 * Change one bit of the given register
 * when value is true, the bit is forced to 1
 * when value is false, the bit is forced to 0
 */
bool LIS2DH::writeMaskedRegister8(const uint8_t register_addr, const uint8_t mask, const bool value) {
    uint8_t data = readRegister(register_addr);
    uint8_t combo;
    if(value) {
        combo = (mask | data);
    } else {
        combo = ((~mask) & data);
    }
    return writeRegister(register_addr, combo);
}

/**
 * Change a register content. The mask is applied to the value. 
 * The conten of the register is read then clear before
 * the value is applied.
 * Value is not shift
 */
bool LIS2DH::writeMaskedRegisterI(const int register_addr, const int mask, const int value) {
    uint8_t data = readRegister(register_addr);
    uint8_t masked_value = (( data & ~mask) | (mask & value)); 
    return writeRegister(register_addr, masked_value);
}

// -----------------------------------------------------
// Read to LIS2DH
// -----------------------------------------------------

/**
 * Read 8bits of data from the chip, return the value
 */
uint8_t LIS2DH::readRegister(const uint8_t register_addr) {
    Wire.beginTransmission(_address); //open communication with 
    Wire.write(register_addr);  
    Wire.endTransmission(); 
    Wire.requestFrom(_address, (uint8_t)1);
    uint8_t v = Wire.read(); 
    return v;
}

/**
 * Read 16 bits of data by reading two different registers
 */
uint16_t LIS2DH::readRegisters(const uint8_t msb_register, const uint8_t lsb_register) {
    uint8_t msb = readRegister(msb_register);
    uint8_t lsb = readRegister(lsb_register);
    return (((int16_t)msb) << 8) | lsb;
}

/**
 * Read and mask a register from the chip
 * The returned value is not shift
 */
uint8_t LIS2DH::readMaskedRegister(const uint8_t register_addr, const uint8_t mask) {
    uint8_t data = readRegister(register_addr);
    return (data & mask);
}
