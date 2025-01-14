#ifndef TOUCH_DRIVER_GT911_H
#define TOUCH_DRIVER_GT911_H

#include "mbed.h"
#include "lvgl.h"
#include "SoftI2C.h"

class TouchDriverGT911 {
public:
    TouchDriverGT911(PinName sda, PinName scl, PinName int_pin, PinName rst_pin);
    void init();
    bool read(lv_indev_t *drv, lv_indev_data_t *data);
    void invertX(bool invert) { _invert_x = invert; }
    void invertY(bool invert) { _invert_y = invert; }
    void swapXY(bool swap) { _swap_xy = swap; }
    

private:
    SoftI2C _i2c;
    DigitalInOut _int_pin;
    DigitalOut _rst_pin;
    InterruptIn _int;
    void _irq_handler();
    EventFlags _event;
    
    bool _inited;
    char _product_id[4];
    uint16_t _firmware_version;
    uint16_t _max_x_coord;
    uint16_t _max_y_coord;
    int16_t _last_x;
    int16_t _last_y;
    bool _invert_x;
    bool _invert_y;
    bool _swap_xy;

    uint8_t _dev_addr;
    int32_t gt911_i2c_write8(uint16_t register_addr, uint8_t data);
    int32_t gt911_i2c_read(uint16_t register_addr, uint8_t *data_buf, uint8_t len);
};

#endif // TOUCH_DRIVER_GT911_H