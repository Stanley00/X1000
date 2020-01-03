/*
 * drivers/misc/sky1311t.c
 *
 * Skyrelay sky1311t contactless reader IC driver.
 * This driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/sysctl.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/random.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#include <linux/spi/spi.h>
#include <linux/spi/sky1311t.h>


/**
 * IOCTL commands
 */
#define SKY1311T_IOC_MAGIC              't'
#define SKY1311T_IOC_S_TIMER_DELAY_HZ    _IOW(SKY1311T_IOC_MAGIC, 0, int)
#define SKY1311T_IOC_R_CARD              _IOR(SKY1311T_IOC_MAGIC, 1, int)
#define SKY1311T_IOC_R_CARD_BLOCK        _IOR(SKY1311T_IOC_MAGIC, 2, int)
#define SKY1311T_IOC_START_C_CARD_IN     _IOW(SKY1311T_IOC_MAGIC, 3, int)
#define SKY1311T_IOC_STOP_C_CARD_IN      _IOW(SKY1311T_IOC_MAGIC, 4, int)

/**
 *******************************************************************************
 ** 简述：sky1311 命令定义
 ******************************************************************************/
#define     CMD_IDLE                (0x00)
#define     CMD_CLR_FF              (0x03)
#define     CMD_TX                  (0x04)
#define     CMD_RX                  (0x08)
#define     CMD_TX_RX               (0x0C)
#define     CMD_SW_RST              (0x0F)
#define     CMD_M1_AUTO             (0x16)
/**
 *******************************************************************************
 ** 简述：sky1311 寄存器地址定义
 ******************************************************************************/
#define     ADDR_FIFO_LEN           (0x00)
#define     ADDR_FIFO_STA           (0x01)
#define     ADDR_FIFO_CTRL          (0x02)
#define     ADDR_FIFO               (0x03)
#define     ADDR_TX_CTRL            (0x04)
#define     ADDR_TX_PUL_WID         (0x05)
#define     ADDR_TX_BYTE_NUM        (0x06)
#define     ADDR_TX_BIT_NUM         (0x07)
#define     ADDR_TX_FWAIT           (0x08)
#define     ADDR_TIME_OUT0          (0x09)
#define     ADDR_TIME_OUT1          (0x0A)
#define     ADDR_TIME_OUT2          (0x0B)
#define     ADDR_FGUD_RX            (0x0C)
#define     ADDR_RX_CTRL            (0x0D)
#define     ADDR_RX_PUL_DETA        (0x0E)
#define     ADDR_CRC_CTRL           (0x0F)
#define     ADDR_RX_START_BIT_NUM   (0x10)
#define     ADDR_CRC_IN             (0x11)
#define     ADDR_IRQ_EN             (0x12)
#define     ADDR_IRQ_STA            (0x13)
#define     ADDR_ERR_STA            (0x14)
#define     ADDR_RX_NUM_L           (0x15)
#define     ADDR_RX_NUM_H           (0x16)
#define     ADDR_TIMER_CNT0         (0x17)
#define     ADDR_TIMER_CNT1         (0x18)
#define     ADDR_FSM_STATE          (0x19)
#define     ADDR_PA_OFFSET          (0x1A)
#define     ADDR_MOD_SRC            (0x1B)
#define     ADDR_MFOUT_SEL          (0x1C)
#define     ADDR_ANA_CFG0           (0x1D)
#define     ADDR_ANA_CFG1           (0x1E)
#define     ADDR_ANA_CFG2           (0x1F)
#define     ADDR_RATE_CTRL          (0x20)
#define     ADDR_RATE_THRES         (0x21)
#define     ADDR_RATE_FRAME_END     (0x22)
#define     ADDR_RATE_SUB_THRES     (0x23)
#define     ADDR_RATE_RX_BYTE       (0x24)
#define     ADDR_RATE_RX_BIT        (0x25)
#define     ADDR_M1_SUC_STATE       (0x26)
#define     ADDR_M1_SUC64_0         (0x27)
#define     ADDR_M1_SUC64_1         (0x28)
#define     ADDR_M1_SUC64_2         (0x29)
#define     ADDR_M1_SUC64_3         (0x2A)
#define     ADDR_M1_SUC96_0         (0x2B)
#define     ADDR_M1_SUC96_1         (0x2C)
#define     ADDR_M1_SUC96_2         (0x2D)
#define     ADDR_M1_SUC96_3         (0x2E)
#define     ADDR_M1_CTRL            (0x2F)
#define     ADDR_M1_KEY             (0x30)
#define     ADDR_M1_ID              (0x31)
#define     ADDR_RX_PRE_PROC        (0x32)
#define     ADDR_TX_B_CTRL          (0x33)
#define     ADDR_TX_B_EGT_NUM       (0x34)
#define     ADDR_TX_B_BYTE_NUM      (0x35)
#define     ADDR_RX_B_CTRL          (0x36)
#define     ADDR_RX_B_BYTE_NUM      (0x37)
#define     ADDR_RX_B_PRE           (0x38)
#define     ADDR_RX_MANC_SEC_PULSE  (0x39)
#define     ADDR_ANA_CFG3           (0x3A)
#define     ADDR_ANA_CFG4           (0x3B)
#define     ADDR_ANA_CFG5           (0x3C)
#define     ADDR_ANA_CFG6           (0x3D)
#define     ADDR_ANA_CFG7           (0x3E)
#define     ADDR_ANA_CFG8           (0x3F)
/**
 *******************************************************************************
 ** 简述：sky1311 寄存器位定义
 ******************************************************************************/
#define     TX_106                  (0x00)
#define     TX_212                  (0x40)
#define     TX_424                  (0x80)
#define     TX_POLE_HIGH            (1<<3)
#define     TX_CRC_EN               (1<<2)
#define     TX_PARITY_EVEN          (0x01)
#define     TX_PARITY_ODD           (0x03)
#define     TX_POLE                 (1<<3)
#define     RX_PARITY_EN            (1<<0)
#define     RX_CRC_EN               (1<<1)
#define     RX_PARITY_ODD           (1<<2)
#define     RX_MIFARE_ON            (1<<3)
#define     RX_CAL_CTRL_0           (1<<6)
#define     RX_CAL_CTRL_1           (1<<7)
#define     IRQ_M1_AUTO_EN          (1<<7)
#define     IRQ_TOUT_EN             (1<<6)
#define     IRQ_TX_EN               (1<<5)
#define     IRQ_RX_EN               (1<<4)
#define     IRQ_HIGH_EN             (1<<3)
#define     IRQ_LOW_EN              (1<<2)
#define     IRQ_OSC_EN              (1<<1)
#define     IRQ_ERR_EN              (1<<0)
#define     IRQ_M1_AUTO             (1<<7)
#define     IRQ_TOUT                (1<<6)
#define     IRQ_TX                  (1<<5)
#define     IRQ_RX                  (1<<4)
#define     IRQ_HIGH                (1<<3)
#define     IRQ_LOW                 (1<<2)
#define     IRQ_OSC                 (1<<1)
#define     IRQ_ERR                 (1<<0)
#define     MFOUT_RX_PHASE          (0x00)
#define     MFOUT_BIT_TX            (0x01)
#define     MFOUT_RX_BIT            (0x02)
#define     MFOUT_ANALOG_RX         (0x03)
#define     MFOUT_DO                (0x04)
#define     COLL_EN                 (1<<1)
#define     RX_FORBID               (1<<2)
#define     COLL_FLAG               (1<<3)
#define     RX_RATE_WID_0           (0x00)
#define     RX_RATE_WID_2           (0x40)
#define     RX_RATE_WID_4           (0x80)
#define     RX_RATE_212             (0x10)
#define     RX_RATE_424             (0x20)
#define     RX_RATE_PAR_ODD         (0x04)
#define     RX_RATE_PAR_EVEN        (0x00)
#define     RX_RATE_CRC_EN          (0x02)
#define     RX_RATE_PAR_EN          (0x01)
#define     TX_B_EOF_L0             (1<<7)
#define     TX_B_SOF_L1             (1<<6)
#define     TX_B_SOF_L0             (1<<5)
#define     TX_B_EGT_S              (1<<4)
#define     TX_B_CRC_EN             (1<<3)
#define     TX_B_EOF_EN             (1<<2)
#define     TX_B_SOF_EN             (1<<1)
#define     TX_B_POLE               (1<<0)
#define     RX_B_CRC_EN             (1<<0)
#define     TX_EN                   (1<<7)
#define     RX_EN                   (1<<6)

#define     PA_3P3V                 (3<<2)
#define     PA_3P0V                 (2<<2)
#define     PA_2P5V                 (1<<2)
#define     PA_2P0V                 (0<<2)
#define     WKU_EN                  (1<<7)
#define     WKU_100MS               (0<<4)
#define     WKU_200MS               (1<<4)
#define     WKU_300MS               (2<<4)
#define     WKU_400MS               (3<<4)
#define     WKU_500MS               (4<<4)
#define     WKU_600MS               (5<<4)
#define     WKU_800MS               (6<<4)
#define     WKU_1S                  (7<<4)
#define     WKU_DETA                (0<<2)
#define     WKU_ABS                 (1<<2)
#define     WKU_AND                 (2<<2)
#define     WKU_OR                  (3<<2)
#define     RSSI_2AVG               (0   )
#define     RSSI_4AVG               (1)
#define     RSSI_8AVG               (2)
#define     RSSI_16AVG              (3)
#define     ADC_SAMPLE_5US          (0<<7)
#define     ADC_SAMPLE_10US         (1<<7)
#define     TX_SETTLE_0US           (0<<5)
#define     TX_SETTLE_5US           (1<<5)
#define     TX_SETTLE_10US          (2<<5)
#define     TX_SETTLE_15US          (3<<5)
/**
 *******************************************************************************
 ** 简述：sky1311 部分命令字定义
 ******************************************************************************/
#define     TYPE_A_SEL              (0x00)
#define     TYPE_B_SEL              (0x40)
#define     RATE_SEL                (0x80)
#define     CRC_A                   (0x01)
#define     CRC_B                   (0x04)
#define     TYPE_A                  (0x01)
#define     TYPE_B                  (0x02)
#define     RATE_ON                 (0x01)
#define     RATE_OFF                (0x00)
#define     ANA2_A                  (0x80)
#define     ANA2_B                  (0x98)
#define     COLL_NO                 (0x00)
#define     COLL_YES                (0x01)

#define     PARITY_CRC_ERROR        (0xC0)
#define     PARITY_ERROR            (0x80)
#define     CRC_ERROR               (0x40)
#define     NO_ANS                  (-1)
#define     M1_ERROR                (-2)
/**
 *******************************************************************************
 ** 简述：全局预定义
 ******************************************************************************/
#define     SEL1                    (0x93)
#define     SEL2                    (0x95)
#define     SEL3                    (0x97)

#define     REQA                    (0x26)
#define     WUPA                    (0x52)
#define     SELECT                  (0x90)
#define     HALTA                   (0x50)
#define     PATS                    (0xE0)
#define     PPS                     (0xD0)

#define     APF_CODE                (0x05) // REQB命令帧前缀字节APf
#define     APN_CODE                (0x05) // REQB命令帧前缀字节APn
#define     APC_CODE                (0x1D) // ATTRIB命令帧前缀字节APC
#define     HALTB_CODE              (0x50) // 挂起命令

#define     REQIDEL                 (0x00) // IDLE
#define     REQALL                  (0x08) // ALL

#define     M1_AUTH                 (0x60)
#define     M1_AUTH_KEYA            (0x60)
#define     M1_AUTH_KEYB            (0x61)
#define     M1_READ                 (0x30)
#define     M1_WRITE                (0xA0)
#define     M1_INCREMENT            (0xC1)
#define     M1_DECREMENT            (0xC0)
#define     M1_RESTORE              (0xC2)
#define     M1_TRANSFER             (0xB0)
#define     M1_ACK                  (0xA0)


#define     MINADVAL                (0x30)
#define     MAXADVAL                (0xFF)
#define     RSSI_DELTA              (4)
#define     RSSI_ABS                (3)

#define     TIMEOUT_CNT_MAX         (20)   // Can be modified according to actual situation


typedef enum card_type {
    TYPEA_CARD,
    TYPEB_CARD,
    MIFARE1_CARD,
    UNKNOWN_TYPE,
} card_type_t;

typedef enum adc_state {
    NOCHANGE,
    LESSEN,
    LARGEN,
} adc_state_t;

typedef enum sta_result
{
    OK                       = 0,   ///< 无错，操作成功
    ERROR                    = 1,   ///< 非特定错误
    TIMEOUT                  = 3,   ///< 超时错误
    RXPARITY                 = 4,   ///< 接收奇偶校验错
    RXCHECKCRC               = 5,   ///< 接收CRC_ERRORC校验错
    FIFOFULL                 = 6,   ///< FIFO满
    FIFOEMPTY                = 7,   ///< FIFO空
    COLLISION                = 8,   ///< 防冲突错误
    FRAMING                  = 9,   ///< 数据帧错误
    UIDFORMAT                = 10,  ///< UID格式错误
    M1ERROR                  = 11,  ///< 操作M1卡错误

    ERRORADDRESSALIGNMENT    = 12,  ///< Address alignment does not match
    ERRORACCESSRIGHTS        = 13,  ///< Wrong mode (e.g. user/system) mode is set
    ERRORINVALIDPARAMETER    = 14,  ///< Provided parameter is not valid
    ERROROPERATIONINPROGRESS = 15,  ///< A conflicting or requested operation is still in progress
    ERRORINVALIDMODE         = 16,  ///< Operation not allowed in current mode
    ERRORUNINITIALIZED       = 17,  ///< Module (or part of it) was not initialized properly
    ERRORBUFFERFULL          = 18,  ///< Circular buffer can not be written because the buffer is full
    ERRORTIMEOUT             = 19,  ///< Time Out error occurred (e.g. I2C arbitration lost, Flash time-out, etc.)
    ERRORNOTREADY            = 20,  ///< A requested final state is not reached
    OPERATIONINPROGRESS      = 21,  ///< Indicator for operation in progress

    UNKNOWERROR              = 0x7F,
    NORESPONSE               = 0xFF
}sta_result_t;

typedef struct card_info {
    enum card_type type;
    bool only_r_uid;
    unsigned char uid[10];
    unsigned char keytype;
    unsigned char m1key[6];
    unsigned char block_id;
    unsigned char block_data[16];
} card_info_t;

struct sky1311t_drv_data {
    bool has_card_in;
    bool card_remove;

    unsigned short hz;
    unsigned short rcval;
    unsigned char max_adcval;
    unsigned char timeout_cnt;

    struct card_info card;

    atomic_t opened;
    wait_queue_head_t wait;
    struct spi_device *spi;
    struct spi_message msg;
    struct spi_transfer xfer;
    struct device *dev;
    struct mutex lock;
    struct timer_list timer;
    struct work_struct work;
    struct miscdevice miscdev;
    struct sky1311t_platform_data *pdata;
};

static inline void sky1311t_set_cs(struct sky1311t_platform_data *pdata,
        unsigned char sta)
{
#ifdef CONFIG_SKY1311T_DRV_HOLD_CS
    if (gpio_is_valid(pdata->spi_cs_pin)) {
        gpio_direction_output(pdata->spi_cs_pin, sta);
    }
#endif
}

static inline unsigned char sky1311t_irq_done(struct sky1311t_platform_data *pdata)
{
    return gpio_get_value(pdata->ic_int_pin);
}

/**
 * Smartlink sky1311t write and read data
 * @tx_buf/rx_buf data to write/read
 * @len: data length
 * @retval: =0: success
 *          <0: failed
 */
static int sky1311t_spi_transfer(struct sky1311t_drv_data *sky1311t,
        unsigned char *tx_buf, unsigned char *rx_buf, unsigned int len)
{
    struct spi_device *spi = sky1311t->spi;
    int errno = 0;

    sky1311t->xfer.tx_buf        = tx_buf;
    sky1311t->xfer.rx_buf        = rx_buf;
    sky1311t->xfer.len           = len;
    sky1311t->xfer.bits_per_word = spi->bits_per_word;
    sky1311t->xfer.speed_hz      = spi->max_speed_hz;
    sky1311t->xfer.delay_usecs   = 0;

    spi_message_init(&sky1311t->msg);
    spi_message_add_tail(&sky1311t->xfer, &sky1311t->msg);

    errno = spi_sync(sky1311t->spi, &sky1311t->msg);
    if (errno < 0) {
        dev_err(sky1311t->dev, "spi transfer failed, errno %d\n", errno);
    }

    return errno;
}

static int sky1311t_send_command(struct sky1311t_drv_data *sky1311t,
        unsigned char cmd)
{
    unsigned int errno;
    unsigned char tx_buf[1];

    tx_buf[0] = (cmd & 0x1F) | 0x80;

    sky1311t_set_cs(sky1311t->pdata, 0);
    errno = sky1311t_spi_transfer(sky1311t, tx_buf, NULL, 1);
    sky1311t_set_cs(sky1311t->pdata, 1);

    return errno;
}

static int sky1311t_read_register(struct sky1311t_drv_data *sky1311t,
        unsigned char reg_addr, unsigned char *reg_val)
{
    int errno = 0;
    unsigned char tx_buf[2];
    unsigned char rx_buf[2];

    tx_buf[0] = (reg_addr & 0x3F) | 0x40; /* bit[7,6]=01, as addr/read mode */
    tx_buf[1] = 0xFF;

    sky1311t_set_cs(sky1311t->pdata, 0);
    errno = sky1311t_spi_transfer(sky1311t, tx_buf, rx_buf, 2);
    if (errno < 0) {
        sky1311t_set_cs(sky1311t->pdata, 1);
        return errno;
    }
    if (reg_val != NULL) {
        *reg_val = rx_buf[1];
    }
    sky1311t_set_cs(sky1311t->pdata, 1);

    return 0;
}

static int sky1311t_write_register(struct sky1311t_drv_data *sky1311t,
        unsigned char reg_addr, unsigned char reg_val)
{
    unsigned int errno;
    unsigned char tx_buf[2];

    tx_buf[0] = reg_addr & 0x3F;
    tx_buf[1] = reg_val;

    sky1311t_set_cs(sky1311t->pdata, 0);
    errno = sky1311t_spi_transfer(sky1311t, tx_buf, NULL, 2);
    sky1311t_set_cs(sky1311t->pdata, 1);

    return errno;
}

static int sky1311t_modify_register(struct sky1311t_drv_data *sky1311t,
        unsigned char reg_addr, unsigned char mask, bool is_set)
{
    int errno;
    unsigned char reg_val;

    errno = sky1311t_read_register(sky1311t, reg_addr, &reg_val);
    if (errno < 0) {
        dev_err(sky1311t->dev, "modify register failed\n");
        return errno;
    }

    if (is_set)
        reg_val |= mask;
    else
        reg_val &= (~mask);

    return sky1311t_write_register(sky1311t, reg_addr, reg_val);
}

static int sky1311t_read_fifo(struct sky1311t_drv_data *sky1311t,
        unsigned char *data, unsigned int len)
{
    unsigned int errno;

#ifdef CONFIG_SKY1311T_DRV_HOLD_CS
    unsigned char tx_buf[1];

    tx_buf[0] = (ADDR_FIFO & 0x3F) | 0x40; /* bit[7,6]=01, as addr/read mode */

    sky1311t_set_cs(sky1311t->pdata, 0);
    errno = sky1311t_spi_transfer(sky1311t, tx_buf, NULL, 1);
    if (errno < 0) {
        dev_err(sky1311t->dev, "%s do init connect failed\n", __FUNCTION__);
        sky1311t_set_cs(sky1311t->pdata, 1);
        return errno;
    }

    errno =sky1311t_spi_transfer(sky1311t, NULL, data, len);
    sky1311t_set_cs(sky1311t->pdata, 1);
#else
    while(len--) {
        errno = sky1311t_read_register(sky1311t, ADDR_FIFO, data++);
        if (errno < 0) {
            dev_err(sky1311t->dev, "read fifo failed, errno %d\n", errno);
            return errno;
        }
    }
#endif

    return 0;
}

static int sky1311t_write_fifo(struct sky1311t_drv_data *sky1311t,
        unsigned char *data, unsigned int len)
{
    unsigned int errno;

#ifdef CONFIG_SKY1311T_DRV_HOLD_CS
    unsigned char tx_buf[1];

    tx_buf[0] = (ADDR_FIFO & 0x3F);

    sky1311t_set_cs(sky1311t->pdata, 0);
    errno = sky1311t_spi_transfer(sky1311t, tx_buf, NULL, 1);
    if (errno < 0) {
        dev_err(sky1311t->dev, "%s do init connect failed\n", __FUNCTION__);
        sky1311t_set_cs(sky1311t->pdata, 1);
        return errno;
    }

    errno = sky1311t_spi_transfer(sky1311t, data, NULL, len);
    sky1311t_set_cs(sky1311t->pdata, 1);
#else
    while(len--) {
        errno = sky1311t_write_register(sky1311t, ADDR_FIFO, *(data++));
        if (errno < 0) {
            dev_err(sky1311t->dev, "write fifo failed, errno %d\n", errno);
            return errno;
        }
    }
#endif

    return 0;
}

static inline void sky1311t_pd_enable(struct sky1311t_drv_data *sky1311t)
{
    gpio_direction_output(sky1311t->pdata->ic_pd2_pin, 1);
}

static inline void sky1311t_pd_disable(struct sky1311t_drv_data *sky1311t)
{
    gpio_direction_output(sky1311t->pdata->ic_pd2_pin, 0);
}

static inline void sky1311t_clear_all_irqsta(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_IRQ_STA, 0xFF);
}

static inline void sky1311t_clear_irqsta(struct sky1311t_drv_data *sky1311t,
        unsigned char irq)
{
    sky1311t_write_register(sky1311t, ADDR_IRQ_STA, irq);
}

static inline void sky1311t_tx_crc_enable(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_TX_CTRL, TX_CRC_EN|TX_PARITY_ODD|TX_POLE_HIGH);
}

static inline void sky1311t_tx_crc_disable(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_TX_CTRL, (!TX_CRC_EN)|TX_PARITY_ODD|TX_POLE_HIGH);
}

static inline void sky1311t_rx_crc_enable(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_RX_CTRL, RX_MIFARE_ON|RX_CRC_EN|RX_PARITY_EN|RX_PARITY_ODD);
}

static inline void sky1311t_rx_crc_disable(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_RX_CTRL, RX_MIFARE_ON|RX_PARITY_EN|RX_PARITY_ODD);
}

static inline void sky1311t_analog_init(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_ANA_CFG0, 0x00);
    sky1311t_write_register(sky1311t, ADDR_ANA_CFG1, 0xEC);
    sky1311t_write_register(sky1311t, ADDR_ANA_CFG2, ANA2_A);
    msleep(3);
}

static void sky1311t_pcd_init(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_send_command(sky1311t, CMD_SW_RST);
    sky1311t_analog_init(sky1311t);

    sky1311t_write_register(sky1311t, ADDR_TIME_OUT2, 0x8F); // time_out timer stop condition = beginning of RX SOF
    sky1311t_write_register(sky1311t, ADDR_TIME_OUT1, 0xFF);
    sky1311t_write_register(sky1311t, ADDR_TIME_OUT0, 0xFF);
    sky1311t_write_register(sky1311t, ADDR_RX_PUL_DETA, 0x64);

    sky1311t_write_register(sky1311t, ADDR_IRQ_EN, IRQ_M1_AUTO_EN|IRQ_TOUT_EN|IRQ_TX_EN|IRQ_RX_EN|IRQ_HIGH_EN|IRQ_LOW_EN);
}

static void sky1311t_pcd_reset(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_send_command(sky1311t, CMD_SW_RST);
    sky1311t_write_register(sky1311t, ADDR_ANA_CFG1, 0x00);
    sky1311t_write_register(sky1311t, ADDR_ANA_CFG4, 0x03);
    sky1311t_pd_disable(sky1311t);
}

static void sky1311t_picc_reset(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_modify_register(sky1311t, ADDR_ANA_CFG1, TX_EN, 0);
    msleep(1);
    sky1311t_modify_register(sky1311t, ADDR_ANA_CFG1, TX_EN, 1);
    msleep(3);
}

static void sky1311t_cardtype_set(struct sky1311t_drv_data *sky1311t,
        enum card_type type)
{
    switch(type) {
    case TYPEB_CARD:
        sky1311t_write_register(sky1311t, ADDR_ANA_CFG2, ANA2_B);      // analogB select
        sky1311t_write_register(sky1311t, ADDR_FSM_STATE, TYPE_B_SEL); // typeB select
        sky1311t_write_register(sky1311t, ADDR_CRC_CTRL, CRC_B);       // crcB enable
        break;
    case TYPEA_CARD:
    case MIFARE1_CARD:
    default:
        sky1311t_write_register(sky1311t, ADDR_ANA_CFG2, ANA2_A);      // analogA select
        sky1311t_write_register(sky1311t, ADDR_FSM_STATE, TYPE_A_SEL); // typeA select
        sky1311t_write_register(sky1311t, ADDR_TX_PUL_WID, 0x26);      // set to default value
        sky1311t_write_register(sky1311t, ADDR_CRC_CTRL, CRC_A);       // crcA enable
        sky1311t_write_register(sky1311t, ADDR_M1_CTRL, 0x00);         // disable M1 operation
        sky1311t_write_register(sky1311t, ADDR_ANA_CFG3, 0x80);
    }
}

static sta_result_t sky1311t_fifo_tx(struct sky1311t_drv_data *sky1311t,
        unsigned char tx_type, unsigned char *tx_buf, unsigned short tx_size)
{
    unsigned short tx_res = tx_size;
    unsigned char irq_sta;
    volatile unsigned short timeout;

    sky1311t_send_command(sky1311t, CMD_IDLE);   // reset state machine to Idle mode
    sky1311t_send_command(sky1311t, CMD_CLR_FF); // clear FIFO
    sky1311t_clear_all_irqsta(sky1311t);

    switch(tx_type) {
    case TYPE_A:
        sky1311t_write_register(sky1311t, ADDR_TX_BYTE_NUM, tx_size & 0x00ff);
        if (tx_size > 255) {
            sky1311t_write_register(sky1311t, ADDR_TX_BIT_NUM, ((tx_size & 0x0f00)>>4|8)); // write the length to tx_byte register
        }
        break;
    case TYPE_B:
        sky1311t_write_register(sky1311t, ADDR_TX_B_BYTE_NUM, tx_size & 0x00ff);
        if(tx_size > 255) {
            sky1311t_write_register(sky1311t, ADDR_TX_B_EGT_NUM, (tx_size & 0x0300)>>4);
        }
        break;
    default:
        break;
    }

    if (tx_size <= 64) {
        sky1311t_write_fifo(sky1311t, tx_buf, tx_size);
        sky1311t_send_command(sky1311t, CMD_TX_RX); // transceive & into receive mode
    } else {
        sky1311t_write_fifo(sky1311t, tx_buf, 64);
        sky1311t_send_command(sky1311t, CMD_TX_RX);
        tx_res = tx_size - 64;
        while(tx_res > 0) {
            if (sky1311t_irq_done(sky1311t->pdata)) {
                sky1311t_read_register(sky1311t, ADDR_IRQ_STA, &irq_sta);
                if (irq_sta & IRQ_LOW) {   // FIFO low
                    if (tx_res >= 56) {
                        sky1311t_write_fifo(sky1311t, &tx_buf[tx_size - tx_res], 56);
                        tx_res -= 56;
                    } else {
                        sky1311t_write_fifo(sky1311t, &tx_buf[tx_size - tx_res], tx_res);
                        tx_res = 0;
                    }
                    sky1311t_clear_all_irqsta(sky1311t);
                } else {
                    return ERROR;
                }
            }
        }
    }

    while(1) {
        timeout = 0x1FFFUL;
        while(!sky1311t_irq_done(sky1311t->pdata) && timeout--);
        if (!sky1311t_irq_done(sky1311t->pdata)) {
            return ERROR;
        }
        sky1311t_read_register(sky1311t, ADDR_IRQ_STA, &irq_sta);
        if (irq_sta & IRQ_TX) {
            sky1311t_clear_irqsta(sky1311t, IRQ_TX);
            return OK;
        } else {
            sky1311t_clear_all_irqsta(sky1311t);
        }
    }
}

static sta_result_t sky1311t_fifo_rx(struct sky1311t_drv_data *sky1311t,
        unsigned char rx_type, unsigned char rate_type, unsigned char *rx_buf, unsigned short *rx_size)
{
    unsigned short rx_buf_cnt = 0;
    unsigned char byte_num_h;
    unsigned char byte_num_l;
    unsigned char fifo_len;
    unsigned char bit_n = 0;
    unsigned char irq_sta;
    unsigned char err_sta;
    volatile unsigned short timeout;

    while(1) {
        timeout = 0x1FFFUL;
        while(!sky1311t_irq_done(sky1311t->pdata) && timeout--);
        if (timeout == 0) {
            return NORESPONSE;
        }
        sky1311t_read_register(sky1311t, ADDR_IRQ_STA, &irq_sta);
        sky1311t_read_register(sky1311t, ADDR_ERR_STA, &err_sta);
        if (irq_sta & IRQ_TOUT) {    // tiemout
            sky1311t_send_command(sky1311t, CMD_IDLE);
            sky1311t_clear_all_irqsta(sky1311t);
            return NORESPONSE;
        } else if (irq_sta & IRQ_HIGH) {    // FIFO High
            sky1311t_read_fifo(sky1311t, &rx_buf[rx_buf_cnt], 56); // load next 56 bytes into FIFO
            rx_buf_cnt += 56;
            sky1311t_clear_irqsta(sky1311t, IRQ_HIGH);
        } else if (irq_sta & IRQ_RX) {      // Received
            sky1311t_read_register(sky1311t, ADDR_FIFO_LEN, &fifo_len);
            if ((fifo_len <= 3) && (err_sta & 0xC0)) {
                sky1311t_send_command(sky1311t, CMD_CLR_FF); // noise occur, restart the rx
                sky1311t_send_command(sky1311t, CMD_RX);
                sky1311t_clear_all_irqsta(sky1311t);
            } else {
                sky1311t_clear_all_irqsta(sky1311t);
                if(err_sta & 0xC0)
                    return ERROR;
                sky1311t_read_register(sky1311t, ADDR_FIFO_LEN, &fifo_len);  // get FIFO length
                sky1311t_read_fifo(sky1311t, &rx_buf[rx_buf_cnt], fifo_len); // get data ,FIFO-->rx_buf
                rx_buf_cnt += fifo_len;

                switch(rx_type) {
                case TYPE_B:
                    sky1311t_read_register(sky1311t, ADDR_RX_B_CTRL, &byte_num_h);
                    sky1311t_read_register(sky1311t, ADDR_RX_B_BYTE_NUM, &byte_num_l);
                    *rx_size = ((byte_num_h & 0x80) << 1) | byte_num_l;
                    break;
                case TYPE_A:
                default:
                    if (rate_type) {
                        sky1311t_read_register(sky1311t, ADDR_RATE_RX_BIT, &byte_num_h);
                        sky1311t_read_register(sky1311t, ADDR_RATE_RX_BYTE, &byte_num_l);
                    } else {
                        sky1311t_read_register(sky1311t, ADDR_RX_NUM_H, &byte_num_h);
                        sky1311t_read_register(sky1311t, ADDR_RX_NUM_L, &byte_num_l);
                        bit_n = (byte_num_h & 0xF0) >> 4;
                        if (bit_n) {
                            sky1311t_read_fifo(sky1311t, &rx_buf[rx_buf_cnt], 1);
                        }
                    }
                    *rx_size = ((byte_num_h & 0x01) << 8) | byte_num_l;
                    if (bit_n) {
                        *rx_size = ((*rx_size) + 1);
                    }
                }
                return OK;
            }
        } else {
            sky1311t_clear_all_irqsta(sky1311t);
            return NORESPONSE;
        }
    }
}

#if 0
static sta_result_t sky1311t_read_block(struct sky1311t_drv_data *sky1311t,
        unsigned char block_id, unsigned char *block_data)
{
    sta_result_t sta;
    unsigned short tmpsize;
    unsigned char tmpbuf[16];

    sky1311t_tx_crc_enable(sky1311t);
    sky1311t_rx_crc_enable(sky1311t);

    tmpbuf[0] = M1_READ;
    tmpbuf[1] = block_id;
    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 2);
    if (sta != OK)
        return sta;
    sta = sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, block_data, &tmpsize);

    sky1311t_tx_crc_disable(sky1311t);
    sky1311t_rx_crc_disable(sky1311t);

    return sta;
}
#endif

static void sky1311t_picc_haltA(struct sky1311t_drv_data *sky1311t)
{
    sky1311t_write_register(sky1311t, ADDR_FIFO, 0x50);
    sky1311t_write_register(sky1311t, ADDR_FIFO, 0x00);
    sky1311t_write_register(sky1311t, ADDR_TX_BYTE_NUM, 0x02);
    sky1311t_send_command(sky1311t, CMD_TX);
}

static sta_result_t sky1311t_trans_collision(struct sky1311t_drv_data *sky1311t,
        unsigned char *tx_buf, unsigned char tx_len, unsigned char bit_num)
{
    unsigned char irq_sta;
    volatile unsigned short timeout;

    if (tx_len > 7)
        return ERROR;

    sky1311t_send_command(sky1311t, CMD_IDLE);   // reset state machine to Idle mode
    sky1311t_send_command(sky1311t, CMD_CLR_FF); // clear FIFO
    sky1311t_clear_all_irqsta(sky1311t);

    /* write numbers */
    sky1311t_write_register(sky1311t, ADDR_TX_BYTE_NUM, tx_len);
    sky1311t_write_register(sky1311t, ADDR_TX_BIT_NUM, bit_num);
    sky1311t_write_fifo(sky1311t, tx_buf, tx_len); // write data to FIFO
    sky1311t_send_command(sky1311t, CMD_TX_RX);    // transceive & into receive mode

    /* wait TX finished */
    while(1) {
        timeout = 0x1FFFUL;                                      // timeout count
        while(!sky1311t_irq_done(sky1311t->pdata) && timeout--); // waiting for TX STOP IRQ
        if (!sky1311t_irq_done(sky1311t->pdata))
            return TIMEOUT;
        sky1311t_read_register(sky1311t, ADDR_IRQ_STA, &irq_sta);
        if (irq_sta & IRQ_TOUT) {
            sky1311t_send_command(sky1311t, CMD_IDLE);
            sky1311t_clear_all_irqsta(sky1311t);
            return TIMEOUT;
        } else if (irq_sta & IRQ_TX) {
            sky1311t_clear_irqsta(sky1311t, IRQ_TX);
            break;
        } else {
            sky1311t_clear_all_irqsta(sky1311t);
        }
    }

    /* wait recv data finished */
    timeout = 0x1FFFUL;                                      // timeout count
    while(!sky1311t_irq_done(sky1311t->pdata) && timeout--); // waiting for TX STOP IRQ
    if (!sky1311t_irq_done(sky1311t->pdata))
        return TIMEOUT;
    sky1311t_read_register(sky1311t, ADDR_IRQ_STA, &irq_sta);
    if (irq_sta & IRQ_TOUT) {
        sky1311t_send_command(sky1311t, CMD_IDLE);
        sky1311t_clear_all_irqsta(sky1311t);
        return TIMEOUT;
    } else if (!(irq_sta & IRQ_RX)) {
        sky1311t_clear_all_irqsta(sky1311t);
        return ERROR;
    }

    sky1311t_clear_all_irqsta(sky1311t);
    return OK;
}

static sta_result_t sky1311t_picc_wakeupA(struct sky1311t_drv_data *sky1311t,
        unsigned char *atqa)
{
    sta_result_t sta;
    unsigned short tmpsize;
    unsigned char tmpbuf[1];

    tmpbuf[0] = WUPA;
    sky1311t_write_register(sky1311t, ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);      // TX odd parity, no CRC
    sky1311t_write_register(sky1311t, ADDR_RX_CTRL, RX_PARITY_EN|RX_PARITY_ODD);      // RX odd parity, no CRC
    sky1311t_write_register(sky1311t, ADDR_TX_BIT_NUM, 0x07);
    sky1311t_write_register(sky1311t, ADDR_TX_BYTE_NUM, 0x01);
    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 1);
    if (sta != OK)
        return sta;
    return sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, atqa, &tmpsize);
}

static sta_result_t sky1311t_picc_requestA(struct sky1311t_drv_data *sky1311t,
        unsigned char *atqa)
{
    sta_result_t sta;
    unsigned short tmpsize;
    unsigned char tmpbuf[1];

    tmpbuf[0] = REQA;
    sky1311t_write_register(sky1311t, ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD); // TX odd parity, no CRC
    sky1311t_write_register(sky1311t, ADDR_RX_CTRL, RX_PARITY_EN|RX_PARITY_ODD); // RX odd parity, no CRC
    sky1311t_write_register(sky1311t, ADDR_TX_BIT_NUM, 0x07);
    sky1311t_write_register(sky1311t, ADDR_TX_BYTE_NUM, 0x01);

    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 1);
    if (sta != OK)
        return sta;
    sta = sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, atqa, &tmpsize);
    if (sta == OK && tmpsize == 2)
        return OK;
    else
        return NORESPONSE;
}


static unsigned char sky1311t_picc_anticollA(struct sky1311t_drv_data *sky1311t,
        unsigned char sel, unsigned char rand_bit, unsigned char *recv_uid)
{
    unsigned char i = 0;
    unsigned char nvb = 0x20;
    unsigned char cur_valid_bytes = 0, cur_valid_bits = 0;
    unsigned char rx_num_h, rx_num_l;
    unsigned char recv_bytes, recv_bits;
    unsigned char has_collision = 0;
    unsigned char tmpsize = 0;
    unsigned char tmpbuf[12];
    unsigned char rx_buf[12];

    sky1311t_write_register(sky1311t, ADDR_RX_NUM_H, COLL_EN); // enable anti-collision
    sky1311t_write_register(sky1311t, ADDR_TX_CTRL, TX_POLE_HIGH|TX_PARITY_ODD);

    tmpbuf[0] = sel;
    tmpbuf[1] = nvb;
    if (OK != sky1311t_trans_collision(sky1311t, tmpbuf, 2, 8)) {
        sky1311t_write_register(sky1311t, ADDR_RX_NUM_H, 0x00); // disable anti-collision
        return ERROR;
    }

    udelay(5);

    do {
        sky1311t_read_register(sky1311t, ADDR_RX_NUM_L, &rx_num_l);
        sky1311t_read_register(sky1311t, ADDR_RX_NUM_H, &rx_num_h);
        recv_bytes = rx_num_l;
        recv_bits = (rx_num_h & 0xF0) >> 4;
        sky1311t_read_register(sky1311t, ADDR_FIFO_LEN, &tmpsize);
        sky1311t_read_fifo(sky1311t, rx_buf, tmpsize);

        sky1311t_read_register(sky1311t, ADDR_RX_NUM_H, &rx_num_h);
        if ((rx_num_h & RX_FORBID) == 0)   // no collision happens
            has_collision = 0;
        else
            has_collision = 1;

        if (cur_valid_bits == 0) {
            for (i = 0; i < tmpsize; i++) {
                recv_uid[cur_valid_bytes++] = rx_buf[i];
            }
            cur_valid_bits = recv_bits;
        } else {
             if (recv_bytes == 0) {
                recv_uid[cur_valid_bytes - 1] += rx_buf[0] << cur_valid_bits;
                cur_valid_bits += recv_bits;
                if (cur_valid_bits == 8) {
                    cur_valid_bytes++;
                    cur_valid_bits = 0;
                }
            } else {
                recv_uid[cur_valid_bytes-1] += rx_buf[0];
                for (i = 1; i < tmpsize; i++) {
                    recv_uid[cur_valid_bytes++] = rx_buf[i];
                }
                cur_valid_bits = recv_bits;
            }
        }

        if (has_collision) {
            udelay(5);
            if (cur_valid_bits == 0) {
                recv_uid[cur_valid_bytes] = rand_bit;
                cur_valid_bytes++;
                cur_valid_bits = 1;
            } else if (cur_valid_bits == 7) {
                recv_uid[cur_valid_bytes-1] += rand_bit << 7;
                cur_valid_bytes++;
                cur_valid_bits = 0;
            } else {
                recv_uid[cur_valid_bytes-1] += rand_bit << cur_valid_bits;
                cur_valid_bits++;
            }

            nvb = 0x20+(((cur_valid_bytes-1)<<4) | cur_valid_bits);
            tmpbuf[0] = sel;
            tmpbuf[1] = nvb;
            for (i = 0; i < cur_valid_bytes; i++) {
                tmpbuf[2+i] = recv_uid[i];
            }
            if (OK != sky1311t_trans_collision(sky1311t, tmpbuf, cur_valid_bytes+2, cur_valid_bits)) {
                sky1311t_write_register(sky1311t, ADDR_RX_NUM_H, 0x00); // disable anti-collision
                return ERROR;
            }
        }
    } while(has_collision);

    sky1311t_write_register(sky1311t, ADDR_RX_NUM_H, 0x00); // disable anti-collision
    return OK;
}

static sta_result_t sky1311t_picc_ats(struct sky1311t_drv_data *sky1311t,
        unsigned char *ats)
{
    sta_result_t sta;
    unsigned short tmpsize;
    unsigned char tmpbuf[2];

    tmpbuf[0] = 0xE0;
    tmpbuf[1] = 0x80;

    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 2);
    if (sta != OK)
        return sta;
    sta = sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, ats, &tmpsize);
    if (sta != OK)
        return sta;
    if (sta == OK && tmpsize == ats[0] + 2) {
        return OK;
    } else {
        return NORESPONSE;
    }
}

static unsigned char sky1311t_picc_selectA(struct sky1311t_drv_data *sky1311t,
        unsigned char sel, unsigned char *uid, unsigned char *sak)
{
    sta_result_t sta;
    unsigned short tmpsize;
    unsigned char tmpbuf[10];

    sky1311t_write_register(sky1311t, ADDR_TX_CTRL, TX_CRC_EN|TX_PARITY_ODD|TX_POLE_HIGH); // TX odd parity, with CRC
    sky1311t_write_register(sky1311t, ADDR_RX_CTRL, RX_CRC_EN|RX_PARITY_EN|RX_PARITY_ODD);
    sky1311t_write_register(sky1311t, ADDR_TX_BIT_NUM, 0x08);                              // complete bytes

    tmpbuf[0] = sel;
    tmpbuf[1] = 0x70;
    tmpbuf[2] = uid[0];
    tmpbuf[3] = uid[1];
    tmpbuf[4] = uid[2];
    tmpbuf[5] = uid[3];
    tmpbuf[6] = uid[4];

    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 7);
    if (sta != OK)
        return sta;
    sta = sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, sak, &tmpsize);
    if (sta == OK && tmpsize == 3)
        return OK;
    else
        return NORESPONSE;
}

#if 0
static unsigned char sky1311t_m1_authentication(struct sky1311t_drv_data *sky1311t,
        unsigned char auth_mode, unsigned char *m1key, unsigned char *uid, unsigned char block_id)
{
    sta_result_t sta;
    unsigned char suc;
    unsigned char tmpbuf[16];
    unsigned short tmpsize;
    volatile unsigned short timeout;

    sky1311t_write_register(sky1311t, ADDR_M1_CTRL, 0x00);
    sky1311t_tx_crc_enable(sky1311t);
    sky1311t_rx_crc_disable(sky1311t);

    // Loading m1Key
    sky1311t_write_register(sky1311t, ADDR_M1_KEY, m1key[5]);
    sky1311t_write_register(sky1311t, ADDR_M1_KEY, m1key[4]);
    sky1311t_write_register(sky1311t, ADDR_M1_KEY, m1key[3]);
    sky1311t_write_register(sky1311t, ADDR_M1_KEY, m1key[2]);
    sky1311t_write_register(sky1311t, ADDR_M1_KEY, m1key[1]);
    sky1311t_write_register(sky1311t, ADDR_M1_KEY, m1key[0]);

    // Loading UID
    sky1311t_write_register(sky1311t, ADDR_M1_ID, uid[3]);
    sky1311t_write_register(sky1311t, ADDR_M1_ID, uid[2]);
    sky1311t_write_register(sky1311t, ADDR_M1_ID, uid[1]);
    sky1311t_write_register(sky1311t, ADDR_M1_ID, uid[0]);

    sky1311t_write_register(sky1311t, ADDR_M1_CTRL, 0x53); // pass1

    tmpbuf[0] = auth_mode;
    tmpbuf[1] = block_id;
    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 2);
    if (sta != OK)
        return sta;
    sta = sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, tmpbuf, &tmpsize);
    if(sta != OK)
      return sta;

    sky1311t_write_register(sky1311t, ADDR_M1_CTRL, 0x05); // pass2
    sky1311t_tx_crc_disable(sky1311t);
    sky1311t_rx_crc_disable(sky1311t);
    get_random_bytes(tmpbuf, 4);

    timeout = 0x1FFFUL;
    do {
        sky1311t_read_register(sky1311t, ADDR_M1_SUC_STATE, &suc);
    } while((suc & 0x01) == 0 && timeout--);

    sky1311t_read_register(sky1311t, ADDR_M1_SUC64_3, &tmpbuf[4]);
    sky1311t_read_register(sky1311t, ADDR_M1_SUC64_2, &tmpbuf[5]);
    sky1311t_read_register(sky1311t, ADDR_M1_SUC64_1, &tmpbuf[6]);
    sky1311t_read_register(sky1311t, ADDR_M1_SUC64_0, &tmpbuf[7]);

    sta = sky1311t_fifo_tx(sky1311t, TYPE_A, tmpbuf, 8);    // send Token AB
    if (sta != OK)
        return sta;
    sky1311t_write_register(sky1311t, ADDR_M1_CTRL, 0x09);  // pass3
    sta = sky1311t_fifo_rx(sky1311t, TYPE_A, RATE_OFF, tmpbuf, &tmpsize);
    if(sta != OK)
        return sta;

    // auth Token AB
    sky1311t_read_register(sky1311t, ADDR_M1_SUC96_3, &tmpbuf[10]);
    sky1311t_read_register(sky1311t, ADDR_M1_SUC96_2, &tmpbuf[11]);
    sky1311t_read_register(sky1311t, ADDR_M1_SUC96_1, &tmpbuf[12]);
    sky1311t_read_register(sky1311t, ADDR_M1_SUC96_0, &tmpbuf[13]);
    if (tmpbuf[0] == tmpbuf[10] &&
        tmpbuf[1] == tmpbuf[11] &&
        tmpbuf[2] == tmpbuf[12] &&
        tmpbuf[3] == tmpbuf[13]) {
        return OK;
    } else {
        return NORESPONSE;
    }
}
#endif

static int sky1311t_typeA_operation(struct sky1311t_drv_data *sky1311t)
{
    unsigned char sak;
    unsigned char tmpbuf[64];
    //unsigned char loop = 0;
    struct card_info *card = &sky1311t->card;

    if (OK != sky1311t_picc_wakeupA(sky1311t, tmpbuf)) {
        dev_dbg(sky1311t->dev, "%s wakeupA failed\n", __FUNCTION__);
        goto err_typeA_operation;
    }

    if (OK != sky1311t_picc_anticollA(sky1311t, SEL1, 0x01, card->uid)) {
        dev_dbg(sky1311t->dev, "%s anticollA failed\n", __FUNCTION__);
        goto err_typeA_operation;
    }

    if (OK != sky1311t_picc_selectA(sky1311t, SEL1, card->uid, tmpbuf)) {
        dev_dbg(sky1311t->dev, "%s selectA failed\n", __FUNCTION__);
        goto err_typeA_operation;
    }

    if (card->only_r_uid)
        goto out_typeA_operation;

    sak = tmpbuf[0];
    switch(sak & 0x24) {
    case 0x20: // UID complete, PICC compliant with ISO/IEC 14443-4
        if (OK != sky1311t_picc_ats(sky1311t, tmpbuf)) {
            dev_dbg(sky1311t->dev, "%s ATS failed\n", __FUNCTION__);
            goto err_typeA_operation;
        }
        break;

    case 0x00: // UID complete, PICC not compliant with ISO/IEC 14443-4
        if (!card->only_r_uid) {
            dev_info(sky1311t->dev, "Don't support read block for the time being\n");
            goto err_typeA_operation;
        }
#if 0
        if (OK != sky1311t_m1_authentication(sky1311t,
                card->keytype, card->m1key, card->uid, card->block_id)) {
            loop++;
            if (loop < 8) {
                sky1311t_picc_haltA(sky1311t);
                sky1311t_pcd_init(sky1311t);
                sky1311t_cardtype_set(sky1311t, card->type);
                goto again;
            } else {
                dev_dbg(sky1311t->dev, "%s authentication failed\n", __FUNCTION__);
                goto err_typeA_operation;
            }
        }

        if (OK != sky1311t_read_block(sky1311t, card->block_id, card->block_data)) {
            dev_dbg(sky1311t->dev, "%s read block failed\n", __FUNCTION__);
            goto err_typeA_operation;
        }
#endif
        sky1311t_picc_haltA(sky1311t);
        break;

    default:
        dev_err(sky1311t->dev, "Unknown sak value: 0x%02x\n", sak);
        goto err_typeA_operation;
    }

#if 0  /* for debug */
    for (loop = 0; loop < 7; loop++) {
        printk("uid[%d]=0x%02x\n", loop, card->uid[loop]);
    }
#endif

out_typeA_operation:
    sky1311t_pcd_reset(sky1311t);
    return 0;

err_typeA_operation:
    sky1311t_pcd_reset(sky1311t);
    return -1;
}

static int sky1311t_typeB_operation(struct sky1311t_drv_data *sky1311t)
{
    return 0;
}

static int sky1311t_dev_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sky1311t_drv_data *sky1311t =
                container_of(miscdev, struct sky1311t_drv_data, miscdev);

    if (atomic_read(&sky1311t->opened)) {
        dev_err(sky1311t->dev, "busy, multi open is not supported\n");
        return -EBUSY;
    }

    sky1311t->has_card_in = false;
    sky1311t->card_remove = true;
    atomic_inc(&sky1311t->opened);

    return 0;
}

static int sky1311t_dev_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sky1311t_drv_data *sky1311t =
                container_of(miscdev, struct sky1311t_drv_data, miscdev);

    del_timer_sync(&sky1311t->timer);
    atomic_dec(&sky1311t->opened);
    sky1311t_pcd_reset(sky1311t);
    return 0;
}

static long sky1311t_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sky1311t_drv_data *sky1311t =
                container_of(miscdev, struct sky1311t_drv_data, miscdev);
    struct card_info *card = &sky1311t->card;
    card_info_t __user *p_user = (void __user *)arg;

    switch(cmd) {
    case SKY1311T_IOC_S_TIMER_DELAY_HZ:
        if (copy_from_user((void *)&sky1311t->hz, p_user, sizeof(sky1311t->hz))) {
            return -EFAULT;
        }
        if (sky1311t->hz > HZ) {
            sky1311t->hz = HZ;
        }
        del_timer_sync(&sky1311t->timer);
        sky1311t->timer.expires = jiffies + sky1311t->hz;
        mod_timer(&sky1311t->timer, sky1311t->timer.expires);
        break;

    case SKY1311T_IOC_R_CARD_BLOCK:
        if (filp->f_flags & O_NONBLOCK) {
            dev_err(sky1311t->dev, "Error, dev is opened with O_NONBLOCK\n");
            return -EAGAIN;
        }
        if (wait_event_interruptible(sky1311t->wait, sky1311t->has_card_in)) {
            return -ERESTARTSYS;
        }
    case SKY1311T_IOC_R_CARD:
        mutex_lock(&sky1311t->lock);
        if (copy_from_user((void *)card, p_user, sizeof(struct card_info))) {
            mutex_unlock(&sky1311t->lock);
            return -EFAULT;
        }

        sky1311t_pd_enable(sky1311t);
        sky1311t_pcd_init(sky1311t);
        sky1311t_picc_reset(sky1311t);
        sky1311t_write_register(sky1311t, ADDR_ANA_CFG3, 0x81);
        sky1311t_cardtype_set(sky1311t, card->type);

        switch(card->type) {
        case TYPEA_CARD:
        case MIFARE1_CARD:
            if (sky1311t_typeA_operation(sky1311t) == 0) {
                if (copy_to_user(p_user, (void *)card, sizeof(struct card_info))) {
                    mutex_unlock(&sky1311t->lock);
                    return -EFAULT;
                }
                sky1311t->has_card_in = false;
            } else {
                sky1311t->timeout_cnt++;
                if (sky1311t->timeout_cnt > TIMEOUT_CNT_MAX) {
                    /**
                     * Card operation timeout, force has_card_in to false.
                     */
                    sky1311t->has_card_in = false;
                    dev_err(sky1311t->dev, "Timeout, make sure the card is in RF field\n");
                }
                mutex_unlock(&sky1311t->lock);
                return -ENODEV;
            }
            break;
        case TYPEB_CARD:
            sky1311t_typeB_operation(sky1311t);
            break;
        default:
            dev_err(sky1311t->dev, "Unknown card type: %d\n", card->type);
            mutex_unlock(&sky1311t->lock);
            return -EINVAL;
        }

        mutex_unlock(&sky1311t->lock);
        break;

    case SKY1311T_IOC_START_C_CARD_IN:
        mutex_lock(&sky1311t->lock);
        sky1311t->timer.expires = jiffies + sky1311t->hz;
        mod_timer(&sky1311t->timer, sky1311t->timer.expires);
        mutex_unlock(&sky1311t->lock);
        break;

    case SKY1311T_IOC_STOP_C_CARD_IN:
        del_timer_sync(&sky1311t->timer);
        cancel_work_sync(&sky1311t->work);

        mutex_lock(&sky1311t->lock);
        sky1311t_pd_enable(sky1311t);
        sky1311t_send_command(sky1311t, CMD_SW_RST);
        sky1311t_write_register(sky1311t, ADDR_ANA_CFG1, 0x00);
        sky1311t_write_register(sky1311t, ADDR_ANA_CFG4, 0x03);
        sky1311t_pd_disable(sky1311t);
        sky1311t_set_cs(sky1311t->pdata, 1);
        mutex_unlock(&sky1311t->lock);
        break;

    default:
        dev_err(sky1311t->dev, "Not supported CMD:0x%x\n", cmd);
        return -ENODEV;
    }

    return 0;
}

static struct file_operations sky1311t_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = sky1311t_dev_open,
    .release        = sky1311t_dev_release,
    .unlocked_ioctl = sky1311t_dev_ioctl,
};

static void sky1311t_work_handler(struct work_struct *work)
{
    struct sky1311t_drv_data *sky1311t =
                container_of(work, struct sky1311t_drv_data, work);
    unsigned char tmpbuf[32];

    /**
     * For check RFCard in
     */
    mutex_lock(&sky1311t->lock);
    sky1311t_pd_enable(sky1311t);
    sky1311t_pcd_init(sky1311t);
    sky1311t_picc_reset(sky1311t);
    sky1311t_cardtype_set(sky1311t, TYPEA_CARD);

    if (OK == sky1311t_picc_requestA(sky1311t, tmpbuf)) {
        if (sky1311t->card_remove) {
            sky1311t->card_remove = false;
            sky1311t->has_card_in = true;
            sky1311t->timeout_cnt = 0;
            del_timer(&sky1311t->timer);
            wake_up_interruptible(&sky1311t->wait);
        } else {
            sky1311t->timer.expires = jiffies + sky1311t->hz;
            mod_timer(&sky1311t->timer, sky1311t->timer.expires);
        }
    } else {
        sky1311t->card_remove = true;
        sky1311t->timer.expires = jiffies + sky1311t->hz;
        mod_timer(&sky1311t->timer, sky1311t->timer.expires);
    }

    sky1311t_picc_haltA(sky1311t);
    sky1311t_pcd_reset(sky1311t);
    mutex_unlock(&sky1311t->lock);
}

static void sky1311t_timer_handler(unsigned long data)
{
    struct sky1311t_drv_data *sky1311t = (struct sky1311t_drv_data *)data;

    /**
     * start workqueue
     */
    schedule_work(&sky1311t->work);
}

static inline int sky1311t_chip_id_probe(struct sky1311t_drv_data *sky1311t)
{
    unsigned char reg_val = 0;

    sky1311t_pd_enable(sky1311t);
    sky1311t_read_register(sky1311t, ADDR_ANA_CFG4, &reg_val);
    sky1311t_pd_disable(sky1311t);

    if ((reg_val & 0x0F) == 0x04)
        return 0;

    dev_err(sky1311t->dev, "sky1311t check ChipID failed. read value=0x%x not expected value=0x%x.\n", reg_val, 0x04);
    return -ENODEV;
}

static void sky1311t_gpio_free(struct sky1311t_drv_data *sky1311t)
{
    struct sky1311t_platform_data *pdata = sky1311t->pdata;

    gpio_free(pdata->ic_int_pin);
    gpio_free(pdata->ic_pd2_pin);
}

static int sky1311t_gpio_init(struct sky1311t_drv_data *sky1311t)
{
    struct sky1311t_platform_data *pdata = sky1311t->pdata;
    int errno;

    if (gpio_is_valid(pdata->ic_pd2_pin)) {
        errno = gpio_request(pdata->ic_pd2_pin, "ic_pd2_pin");
        if (errno < 0) {
            dev_err(sky1311t->dev, "Failed to request GPIO%d, errno %d\n",
                    pdata->ic_pd2_pin, errno);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->ic_pd2_pin, 0);
    } else {
        dev_err(sky1311t->dev, "Invalid ic_pd2_pin: %d\n", pdata->ic_pd2_pin);
        errno = -ENODEV;
        goto err_gpio_request2;
    }

    if (gpio_is_valid(pdata->ic_int_pin)) {
        errno = gpio_request(pdata->ic_int_pin, "ic_int_pin");
        if (errno < 0) {
            dev_err(sky1311t->dev, "Failed to request GPIO%d, errno %d\n",
                    pdata->ic_int_pin, errno);
            goto err_gpio_request1;
        }
        gpio_direction_input(pdata->ic_int_pin);
    } else {
        dev_err(sky1311t->dev, "Invalid ic_int_pin: %d\n", pdata->ic_int_pin);
        errno = -ENODEV;
        goto err_gpio_request2;
    }

    sky1311t_set_cs(pdata, 1);
    return 0;

err_gpio_request2:
    gpio_free(pdata->ic_pd2_pin);
err_gpio_request1:
    return errno;
}

static int sky1311t_probe(struct spi_device *spi)
{
    struct sky1311t_drv_data *sky1311t = NULL;
    struct sky1311t_platform_data *pdata = NULL;
    int errno = 0;

    pdata = dev_get_platdata(&spi->dev);
    if (pdata == NULL) {
        dev_err(&spi->dev, "dev.platform_data cannot be NULL\n");
        errno = -ENODEV;
        goto err_sky1311t_probe;
    }

    sky1311t = kzalloc(sizeof(struct sky1311t_drv_data), GFP_KERNEL);
    if (sky1311t == NULL) {
        dev_err(&spi->dev, "Failed to allocate memory for drvdata\n");
        errno = -ENOMEM;
        goto err_sky1311t_probe;
    }

    dev_set_drvdata(&spi->dev, sky1311t);
    sky1311t->spi = spi;
    sky1311t->dev = &spi->dev;
    sky1311t->pdata = pdata;
    mutex_init(&sky1311t->lock);
    atomic_set(&sky1311t->opened, 0);

    errno = sky1311t_gpio_init(sky1311t);
    if (errno < 0)
        goto err_gpio_init;

    errno = sky1311t_chip_id_probe(sky1311t);
    if (errno < 0) {
        dev_err(sky1311t->dev, "sky1311t ChipID probe failed.\n");
        goto err_chip_id_probe;
    }

    sky1311t->miscdev.minor = MISC_DYNAMIC_MINOR;
    sky1311t->miscdev.name = SKY1311T_DEV_NAME;
    sky1311t->miscdev.fops = &sky1311t_dev_fops;
    errno = misc_register(&sky1311t->miscdev);
    if (errno < 0) {
        dev_err(sky1311t->dev, "Unable to register miscdev, errno %d\n", errno);
        goto err_misc_register;
    }

    sky1311t->spi->max_speed_hz = CONFIG_SKY1311T_SPI_SPEED;
    if ((errno = spi_setup(sky1311t->spi)) < 0) {
        dev_err(sky1311t->dev, "Failed to change spi speed, errno %d\n", errno);
        goto err_spi_setup;
    }

    init_waitqueue_head(&sky1311t->wait);
    INIT_WORK(&sky1311t->work, sky1311t_work_handler);

    init_timer(&sky1311t->timer);
    sky1311t->timer.data = (unsigned long)sky1311t;
    sky1311t->timer.function = sky1311t_timer_handler;
    sky1311t->hz = HZ / 4;

    dev_info(sky1311t->dev, "Driver probe successfully\n");
    return 0;

err_spi_setup:
    misc_deregister(&sky1311t->miscdev);
err_misc_register:
err_chip_id_probe:
    sky1311t_gpio_free(sky1311t);
err_gpio_init:
    kfree(sky1311t);
err_sky1311t_probe:
    return errno;
}

static int sky1311t_remove(struct spi_device *spi)
{
    struct sky1311t_drv_data *sky1311t = dev_get_drvdata(&spi->dev);

    del_timer_sync(&sky1311t->timer);
    cancel_work_sync(&sky1311t->work);
    sky1311t_gpio_free(sky1311t);
    misc_deregister(&sky1311t->miscdev);
    kfree(sky1311t);

    return 0;
}

static int sky1311t_suspend(struct spi_device *spi, pm_message_t mesg)
{
    struct sky1311t_drv_data *sky1311t = dev_get_drvdata(&spi->dev);

    sky1311t_pd_disable(sky1311t);
    sky1311t_set_cs(sky1311t->pdata, 1);

    return 0;
}

static int sky1311t_resume(struct spi_device *spi)
{
    struct sky1311t_drv_data *sky1311t = dev_get_drvdata(&spi->dev);

    sky1311t->has_card_in = false;
    sky1311t->card_remove = true;

    return 0;
}

static struct spi_device_id sky1311t_id_table[] = {
    { SKY1311T_DEV_NAME, 0 },
    { }
};

static struct spi_driver sky1311t_drv = {
    .driver = {
        .name  = SKY1311T_DEV_NAME,
        .owner = THIS_MODULE,
    },
    .probe    = sky1311t_probe,
    .remove   = sky1311t_remove,
    .suspend  = sky1311t_suspend,
    .resume   = sky1311t_resume,
    .id_table = sky1311t_id_table,
};

static int __init sky1311t_init(void)
{
    return spi_register_driver(&sky1311t_drv);
}

static void __exit sky1311t_exit(void)
{
    spi_unregister_driver(&sky1311t_drv);
}

module_init(sky1311t_init);
module_exit(sky1311t_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("sky1311t driver");
