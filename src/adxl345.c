/*


Запит

if(adxl.triggered(adxl.getInterruptSource(), ADXL345_ACTIVITY)){
}

WA:0x53, 0x30
RA:0x53, << 0x83
WA:0x53


*/
#include "adxl345.h"
#include "i2c.h"

#define ADXL345_I2C_ADDRESS 0x53


/* ------- Register names ------- */
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_RESERVED1 0x01
#define ADXL345_REG_THRESH_TAP 0x1d
#define ADXL345_REG_OFSX 0x1e
#define ADXL345_REG_OFSY 0x1f
#define ADXL345_REG_OFSZ 0x20
#define ADXL345_REG_DUR 0x21
#define ADXL345_REG_LATENT 0x22
#define ADXL345_REG_WINDOW 0x23
#define ADXL345_REG_THRESH_ACT 0x24
#define ADXL345_REG_THRESH_INACT 0x25
#define ADXL345_REG_TIME_INACT 0x26
#define ADXL345_REG_ACT_INACT_CTL 0x27
#define ADXL345_REG_THRESH_FF 0x28
#define ADXL345_REG_TIME_FF 0x29
#define ADXL345_REG_TAP_AXES 0x2a
#define ADXL345_REG_ACT_TAP_STATUS 0x2b
#define ADXL345_REG_BW_RATE 0x2c
#define ADXL345_REG_POWER_CTL 0x2d
#define ADXL345_REG_INT_ENABLE 0x2e
#define ADXL345_REG_INT_MAP 0x2f
#define ADXL345_REG_INT_SOURCE 0x30
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0 0x32
#define ADXL345_REG_DATAX1 0x33
#define ADXL345_REG_DATAY0 0x34
#define ADXL345_REG_DATAY1 0x35
#define ADXL345_REG_DATAZ0 0x36
#define ADXL345_REG_DATAZ1 0x37
#define ADXL345_REG_FIFO_CTL 0x38
#define ADXL345_REG_FIFO_STATUS 0x39


#define ADXL345_REG_INT_SOURCE  0x30        // Source of interrupts
#define ADXL345_REG_INT_SOURCE_DATA_READY   (1<<7)
#define ADXL345_REG_INT_SOURCE_SINGLE_TAP   (1<<6)
#define ADXL345_REG_INT_SOURCE_DOUBLE_TAP   (1<<5)
#define ADXL345_REG_INT_SOURCE_Activity     (1<<4)



/* 
 Interrupt PINs
 INT1: 0
 INT2: 1
 */
#define ADXL345_INT1_PIN 0x00
#define ADXL345_INT2_PIN 0x01


/*Interrupt bit position*/
#define ADXL345_INT_DATA_READY_BIT 0x07
#define ADXL345_INT_SINGLE_TAP_BIT 0x06
#define ADXL345_INT_DOUBLE_TAP_BIT 0x05
#define ADXL345_INT_ACTIVITY_BIT   0x04
#define ADXL345_INT_INACTIVITY_BIT 0x03
#define ADXL345_INT_FREE_FALL_BIT  0x02
#define ADXL345_INT_WATERMARK_BIT  0x01
#define ADXL345_INT_OVERRUNY_BIT   0x00


static void writeTo(uint8_t reg, uint8_t value)
{
    I2C_start(ADXL345_I2C_ADDRESS);
    I2C_write(reg);
    I2C_write(value);
    I2C_stop();
}

void setRegisterBit(uint8_t regAdress, int bitPos, bool state) {
    uint8_t _b = I2C_read(ADXL345_I2C_ADDRESS, regAdress);
    // readFrom(regAdress, 1, &_b);
    
    if (state) {
        _b |= (1 << bitPos);  // forces nth bit of _b to be 1.  all other bits left alone.
    } 
    else {
        _b &= ~(1 << bitPos); // forces nth bit of _b to be 0.  all other bits left alone.
    }
    writeTo(regAdress, _b);  
}


static void powerOn() {
    //Turning on the ADXL345
    writeTo(ADXL345_REG_POWER_CTL, 0);      
    writeTo(ADXL345_REG_POWER_CTL, 16);
    writeTo(ADXL345_REG_POWER_CTL, 8); 
}

static void setLowPower(bool state) {  
    setRegisterBit(ADXL345_REG_BW_RATE, 4, state); 
}

static void setActivityThreshold(uint8_t activityThreshold) {
    writeTo(ADXL345_REG_THRESH_ACT, activityThreshold);
}

static void setInactivityThreshold(uint8_t inactivityThreshold) {
    writeTo(ADXL345_REG_THRESH_INACT, inactivityThreshold);
}

// Sets the TIME_INACT register, which contains an unsigned time value representing the
// amount of time that acceleration must be less thant the value in the THRESH_INACT
// register for inactivity to be declared. The scale factor is 1sec/LSB. The value must
// be between 0 and 255.
static void setTimeInactivity(uint8_t timeInactivity) {
    writeTo(ADXL345_REG_TIME_INACT, timeInactivity);  
}

static void setActivityX(bool state) {  
    setRegisterBit(ADXL345_REG_ACT_INACT_CTL, 6, state); 
}
static void setActivityY(bool state) {  
    setRegisterBit(ADXL345_REG_ACT_INACT_CTL, 5, state); 
}
static void setActivityZ(bool state) {  
    setRegisterBit(ADXL345_REG_ACT_INACT_CTL, 4, state); 
}
static void setInactivityX(bool state) {  
    setRegisterBit(ADXL345_REG_ACT_INACT_CTL, 2, state); 
}
static void setInactivityY(bool state) {  
    setRegisterBit(ADXL345_REG_ACT_INACT_CTL, 1, state); 
}
static void setInactivityZ(bool state) {  
    setRegisterBit(ADXL345_REG_ACT_INACT_CTL, 0, state); 
}

// Set the mapping of an interrupt to pin1 or pin2
// eg: setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT,ADXL345_INT2_PIN);
static void setInterruptMapping(uint8_t interruptBit, bool interruptPin) {
    setRegisterBit(ADXL345_REG_INT_MAP, interruptBit, interruptPin);
}

static void setInterrupt(uint8_t interruptBit, bool state) {
    setRegisterBit(ADXL345_REG_INT_ENABLE, interruptBit, state);
}

void adxl345_init(uint8_t sensitivity)
{
    powerOn();

    setLowPower(true);

    //set activity/ inactivity thresholds (0-255)
    // adxl.setActivityThreshold(75); //62.5mg per increment
    // adxl.setInactivityThreshold(75); //62.5mg per increment
    // Активность будет всё что больше "80"
    // adxl.setActivityThreshold(80);
    // Отсутствие активности - когда меньше "18"
    // adxl.setInactivityThreshold(18);

    setActivityThreshold( sensitivity );
    setInactivityThreshold( sensitivity-1);
    // Если нет активности - через 3 сек засыпаем
    setTimeInactivity(3); // how many seconds of no activity is inactive?

    //look of activity only on Z /// movement on this axes - 1 == on; 0 == off
    setActivityX(0);
    setActivityY(0);
    setActivityZ(1);

    //look of inactivity movement on this axes - 1 == on; 0 == off
    setInactivityX(0);
    setInactivityY(0);
    setInactivityZ(1);

    //look of tap movement on this axes - 1 == on; 0 == off
    // adxl.setTapDetectionOnX(0);
    // adxl.setTapDetectionOnY(0);
    // adxl.setTapDetectionOnZ(1);

    //set values for what is a tap, and what is a double tap (0-255)
    // adxl.setTapThreshold(50); //62.5mg per increment
    // adxl.setTapDuration(15); //625us per increment
    // adxl.setDoubleTapLatency(80); //1.25ms per increment
    // adxl.setDoubleTapWindow(200); //1.25ms per increment

    //set values for what is considered freefall (0-255)
    // adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
    // adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment

    //setting all interrupts to take place on int pin 1
    //I had issues with int pin 2, was unable to reset it
    // adxl.setInterruptMapping(ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN);
    // adxl.setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN);
    // adxl.setInterruptMapping(ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN);
    setInterruptMapping(ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN);

#if 0
    // Но сейчас нам это не нужно - ещё не спим
    adxl.setInterrupt(ADXL345_INT_ACTIVITY_BIT, false);

    // При остутствии активности датчик подаст плюс на свой первый вывод
    adxl.setInterruptMapping(ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN);

    // Как раз это сейчас актуально
    adxl.setInterrupt(ADXL345_INT_INACTIVITY_BIT, true);
#endif
    //register interrupt actions - 1 == on; 0 == off
    // adxl.setInterrupt(ADXL345_INT_SINGLE_TAP_BIT, 1);
    // adxl.setInterrupt(ADXL345_INT_DOUBLE_TAP_BIT, 1);
    // adxl.setInterrupt(ADXL345_INT_FREE_FALL_BIT,  1);
    setInterrupt(ADXL345_INT_ACTIVITY_BIT,   1);
    // adxl.setInterrupt(ADXL345_INT_INACTIVITY_BIT, 1);
}

static uint8_t getInterruptSource() {
    return I2C_read(ADXL345_I2C_ADDRESS, ADXL345_REG_INT_SOURCE);
}

bool adxl345_is_active()
{
    return getInterruptSource() & (1<< ADXL345_INT_DATA_READY_BIT);
}
