#include "touch_driver_GT911.h"
#include "mbed_trace.h"

#define TRACE_GROUP "LVGL"

#define GT911_I2C_SLAVE_ADDR    0xBA

/* Register Map of GT911 */
#define GT911_PRODUCT_ID1               0x8140
#define GT911_PRODUCT_ID2               0x8141
#define GT911_PRODUCT_ID3               0x8142
#define GT911_PRODUCT_ID4               0x8143
#define GT911_FIRMWARE_VER_L            0x8144
#define GT911_FIRMWARE_VER_H            0x8145
#define GT911_X_COORD_RES_L             0x8146
#define GT911_X_COORD_RES_H             0x8147
#define GT911_Y_COORD_RES_L             0x8148
#define GT911_Y_COORD_RES_H             0x8149
#define GT911_VENDOR_ID                 0x814A

#define GT911_STATUS_REG                0x814E
#define GT911_STATUS_REG_BUF            0x80
#define GT911_STATUS_REG_LARGE          0x40
#define GT911_STATUS_REG_PROX_VALID     0x20
#define GT911_STATUS_REG_HAVEKEY        0x10
#define GT911_STATUS_REG_PT_MASK        0x0F

#define GT911_TRACK_ID1                 0x814F
#define GT911_PT1_X_COORD_L             0x8150
#define GT911_PT1_X_COORD_H             0x8151
#define GT911_PT1_Y_COORD_L             0x8152
#define GT911_PT1_Y_COORD_H             0x8153
#define GT911_PT1_X_SIZE_L              0x8154
#define GT911_PT1_X_SIZE_H              0x8155

TouchDriverGT911::TouchDriverGT911(PinName sda, PinName scl, PinName int_pin, PinName rst_pin) :
    _i2c(sda, scl),
    _int_pin(int_pin, PIN_OUTPUT, PullUp, 0),
    _rst_pin(rst_pin, 1),
    _last_x(0),
    _last_y(0),
    _invert_x(false),
    _invert_y(false),
    _swap_xy(false)
{
    _dev_addr = GT911_I2C_SLAVE_ADDR;
    init();
}

void TouchDriverGT911::init()
{
    // init GT911
    if (!_inited) {
        uint8_t data_buf=0;
        int32_t ret;

        _int_pin.output();

        _int_pin = 0;                          // set int=1 for alternate I2C address 0x28
        _rst_pin = 1;
        ThisThread::sleep_for(10ms);
        _rst_pin = 0;
        ThisThread::sleep_for(20ms);
        _rst_pin = 1;
        ThisThread::sleep_for(20ms);
        _int_pin = 0;
        ThisThread::sleep_for(100ms);

        _int_pin.mode(PullDown);
        _int_pin.input();
        ThisThread::sleep_for(150ms);

        tr_info("Checking for GT911 Touch Controller");
        if ((ret = gt911_i2c_read(GT911_PRODUCT_ID1, &data_buf, 1))) {
            tr_error("Error reading from device");
            return;
        }

        // Reading GTA911 controller infos
        gt911_i2c_read(GT911_PRODUCT_ID1, (uint8_t *)_product_id, sizeof(_product_id));
        tr_info("\tProduct ID: %s", _product_id);

        gt911_i2c_read(GT911_FIRMWARE_VER_L, (uint8_t *)&_firmware_version, sizeof(_firmware_version));
        tr_info("\tFirmware Version: %x", _firmware_version);

        gt911_i2c_read(GT911_VENDOR_ID, &data_buf, 1);
        tr_info("\tVendor ID: 0x%02x", data_buf);

        gt911_i2c_read(GT911_X_COORD_RES_L, (uint8_t *)&_max_x_coord, sizeof(_max_x_coord));
        tr_info("\tX Resolution: %d", _max_x_coord);

        gt911_i2c_read(GT911_Y_COORD_RES_L, (uint8_t *)&_max_y_coord, sizeof(_max_y_coord));
        tr_info("\tY Resolution: %d", _max_y_coord);

        _inited = true;
    }
}

bool TouchDriverGT911::read(lv_indev_t *drv, lv_indev_data_t *data)
{
    uint8_t touchdata[2+8];        // 2 bytes for status, 8 bytes for 1st touch point

    gt911_i2c_read(GT911_STATUS_REG, touchdata, 1); //sizeof(touchdata));

    uint8_t touch_pnt_cnt = touchdata[0] & 0x0F;
    bool touch_valid = touchdata[0] & 0x80;

    if (touch_valid || (touch_pnt_cnt < 6)) {
        gt911_i2c_write8(GT911_STATUS_REG, 0x00);       // reset status
    }

    if (touch_pnt_cnt != 1) {    // ignore no touch & multi touch
        data->point.x = _last_x;
        data->point.y = _last_y;
        data->state = LV_INDEV_STATE_REL;
        
        return false;
    }

//    gt911_i2c_read(gt911_status.i2c_dev_addr, GT911_TRACK_ID1, &data_buf, 1);
//    ESP_LOGI(TAG, "\ttrack_id: %d", data_buf);

    gt911_i2c_read(GT911_PT1_X_COORD_L, (uint8_t*)&_last_x, 2);
    gt911_i2c_read(GT911_PT1_Y_COORD_L, (uint8_t*)&_last_y, 2);

if (_invert_x){
    _last_x = _max_x_coord - _last_x;
}
if (_invert_y){
    _last_y = _max_y_coord - _last_y;
}
if (_swap_xy){
    int16_t swap_buf = _last_x;
    _last_x = _last_y;
    _last_y = swap_buf;
}

    data->point.x = _last_x;
    data->point.y = _last_y;
    data->state = LV_INDEV_STATE_PR;
    tr_info("X=%lu Y=%lu", data->point.x, data->point.y);
    tr_info("X=%lu Y=%lu", data->point.x, data->point.y);

    return false;
}

int32_t TouchDriverGT911::gt911_i2c_read(uint16_t register_addr, uint8_t *data_buf, uint8_t len) {
    uint32_t ret = 0;

    register_addr = __REV16(register_addr);

    ret = _i2c.write(_dev_addr, (char *)&register_addr, 2, false);
    if (ret != 0) {
        tr_error("Error writing to device");
        return -1;
    }
    
    ret = _i2c.read(_dev_addr, (char *)data_buf, len, false);
    if (ret != 0) {
        tr_error("Error reading from device");
        return -1;
    }

    return ret;
}

int32_t TouchDriverGT911::gt911_i2c_write8(uint16_t register_addr, uint8_t data) {
    uint32_t ret;
    uint8_t data_buf[3];

    data_buf[0] = (register_addr >> 8) & 0xFF;  
    data_buf[1] = register_addr & 0xFF;
    data_buf[2] = data; 

    ret = _i2c.write(_dev_addr, (char *)data_buf, 3, true);
    if (ret != 0) {
        tr_error("Error writing to device");
        return -1;
    }

    return ret;
}

