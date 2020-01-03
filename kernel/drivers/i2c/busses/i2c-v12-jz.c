/* drivers/i2c/busses/i2c-v12-jz.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 *      http://www.ingenic.com
 *      Sun Jiwei<jwsun@ingenic.cn>
 *
 * I2C adapter driver for the Ingenic I2C controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#define	I2C_CTRL		(0x00)
#define	I2C_TAR     		(0x04)
#define	I2C_SAR     		(0x08)
#define	I2C_DC      		(0x10)
#define	I2C_SHCNT		(0x14)
#define	I2C_SLCNT		(0x18)
#define	I2C_FHCNT		(0x1C)
#define	I2C_FLCNT		(0x20)
#define	I2C_INTST		(0x2C)
#define	I2C_INTM		(0x30)
#define I2C_RXTL		(0x38)
#define I2C_TXTL		(0x3c)
#define	I2C_CINTR		(0x40)
#define	I2C_CRXUF		(0x44)
#define	I2C_CRXOF		(0x48)
#define	I2C_CTXOF		(0x4C)
#define	I2C_CRXREQ		(0x50)
#define	I2C_CTXABRT		(0x54)
#define	I2C_CRXDONE		(0x58)
#define	I2C_CACT		(0x5C)
#define	I2C_CSTP		(0x60)
#define	I2C_CSTT		(0x64)
#define	I2C_CGC    		(0x68)
#define	I2C_ENB     		(0x6C)
#define	I2C_STA     		(0x70)
#define I2C_TXFLR		(0x74)
#define I2C_RXFLR		(0x78)
#define I2C_SDAHD		(0x7C)
#define	I2C_TXABRT		(0x80)
#define I2C_DMACR            	(0x88)
#define I2C_DMATDLR          	(0x8c)
#define I2C_DMARDLR     	(0x90)
#define	I2C_SDASU		(0x94)
#define	I2C_ACKGC		(0x98)
#define	I2C_ENSTA		(0x9C)
#define I2C_FLT			(0xA0)

/* I2C Control Register (I2C_CTRL) */
#define I2C_CTRL_SLVDIS		(1 << 6)	/* after reset slave is disabled */
#define I2C_CTRL_REST		(1 << 5)
#define I2C_CTRL_MATP		(1 << 4)	/* 1: 10bit address 0: 7bit addressing */
#define I2C_CTRL_SATP		(1 << 3)	/* 1: 10bit address 0: 7bit address */
#define I2C_CTRL_SPDF		(2 << 1)	/* fast mode 400kbps */
#define I2C_CTRL_SPDS		(1 << 1)	/* standard mode 100kbps */
#define I2C_CTRL_MD		(1 << 0)	/* master enabled */

/* I2C Status Register (I2C_STA) */
#define I2C_STA_SLVACT		(1 << 6)	/* Slave FSM is not in IDLE state */
#define I2C_STA_MSTACT		(1 << 5)	/* Master FSM is not in IDLE state */
#define I2C_STA_RFF		(1 << 4)	/* RFIFO if full */
#define I2C_STA_RFNE		(1 << 3)	/* RFIFO is not empty */
#define I2C_STA_TFE		(1 << 2)	/* TFIFO is empty */
#define I2C_STA_TFNF		(1 << 1)	/* TFIFO is not full  */
#define I2C_STA_ACT		(1 << 0)	/* I2C Activity Status */

/* i2c interrupt status (I2C_INTST) */
#define I2C_INTST_IGC           (1 << 11)
#define I2C_INTST_ISTT          (1 << 10)
#define I2C_INTST_ISTP          (1 << 9)
#define I2C_INTST_IACT          (1 << 8)
#define I2C_INTST_RXDN          (1 << 7)
#define I2C_INTST_TXABT         (1 << 6)
#define I2C_INTST_RDREQ         (1 << 5)
#define I2C_INTST_TXEMP         (1 << 4)
#define I2C_INTST_TXOF          (1 << 3)
#define I2C_INTST_RXFL          (1 << 2)
#define I2C_INTST_RXOF          (1 << 1)
#define I2C_INTST_RXUF          (1 << 0)

/* i2c interrupt mask status (I2C_INTM) */
#define I2C_INTM_MIGC		(1 << 11)
#define I2C_INTM_MISTT		(1 << 10)
#define I2C_INTM_MISTP		(1 << 9)
#define I2C_INTM_MIACT		(1 << 8)
#define I2C_INTM_MRXDN		(1 << 7)
#define I2C_INTM_MTXABT		(1 << 6)
#define I2C_INTM_MRDREQ		(1 << 5)
#define I2C_INTM_MTXEMP		(1 << 4)
#define I2C_INTM_MTXOF		(1 << 3)
#define I2C_INTM_MRXFL		(1 << 2)
#define I2C_INTM_MRXOF		(1 << 1)
#define I2C_INTM_MRXUF		(1 << 0)

#define I2C_DC_REST		(1 << 10)
#define I2C_DC_STP		(1 << 9)
#define I2C_DC_READ    		(1 << 8)

#define I2C_ENB_I2CENB		(1 << 0)	/* Enable the i2c */

#define I2C_FIFO_LEN		(CONFIG_I2C_FIFO_LEN)
#define TX_LEVEL		(I2C_FIFO_LEN / 2)
#define RX_LEVEL		(I2C_FIFO_LEN / 2 - 1)
#define TIMEOUT			0xff
#define DEBUG_INFO		2
#define DEBUG_WARN		1

/*
 *  msg_end_type: The bus control which need to be send at end of transfer.
 *  @MSG_END_STOP: Send stop pulse at end of transfer.
 *  @MSG_END_REPEAT_START: Send repeat start at end of transfer.
 */
enum msg_end_type {
	MSG_END_STOP,
	MSG_END_CONTINUE,
	MSG_END_REPEAT_START,
};

/* I2C Transmit Abort Status Register (I2C_TXABRT) */
static const char *abrt_src[] = {
	"I2C_TXABRT_ABRT_7B_ADDR_NOACK",
	"I2C_TXABRT_ABRT_10ADDR1_NOACK",
	"I2C_TXABRT_ABRT_10ADDR2_NOACK",
	"I2C_TXABRT_ABRT_XDATA_NOACK",
	"I2C_TXABRT_ABRT_GCALL_NOACK",
	"I2C_TXABRT_ABRT_GCALL_READ",
	"I2C_TXABRT_ABRT_HS_ACKD",
	"I2C_TXABRT_SBYTE_ACKDET",
	"I2C_TXABRT_ABRT_HS_NORSTRT",
	"I2C_TXABRT_SBYTE_NORSTRT",
	"I2C_TXABRT_ABRT_10B_RD_NORSTRT",
	"I2C_TXABRT_ABRT_MASTER_DIS",
	"I2C_TXABRT_ARB_LOST",
	"I2C_TXABRT_SLVFLUSH_TXFIFO",
	"I2C_TXABRT_SLV_ARBLOST",
	"I2C_TXABRT_SLVRD_INTX",
};

/* I2C standard mode high count register(I2CSHCNT) */
#define I2CSHCNT_ADJUST(n)      (((n) - 8) < 6 ? 6 : ((n) - 8))
/* I2C standard mode low count register(I2CSLCNT) */
#define I2CSLCNT_ADJUST(n)      (((n) - 1) < 8 ? 8 : ((n) - 1))
/* I2C fast mode high count register(I2CFHCNT) */
#define I2CFHCNT_ADJUST(n)      (((n) - 8) < 6 ? 6 : ((n) - 8))
/* I2C fast mode low count register(I2CFLCNT) */
#define I2CFLCNT_ADJUST(n)      (((n) - 1) < 8 ? 8 : ((n) - 1))

struct i2c_jz {
	void __iomem *iomem;
	int irq;
	struct clk *clk;
	struct i2c_adapter adap;

	enum msg_end_type w_end_type;
	enum msg_end_type r_end_type;
	unsigned char *rbuf;
	unsigned char *wbuf;
	unsigned int rd_len;
	int len;

	struct completion complete;

	int debug;
	unsigned int rate;
};

static inline unsigned short i2c_readl(struct i2c_jz *i2c,
		unsigned short offset);
#ifdef I2C_DEBUG

#define PRINT_REG_WITH_ID(reg_name, id)					\
	dev_info(&(i2c->adap.dev),"--"#reg_name "    	0x%08x\n",i2c_readl(id,reg_name))

static void i2c_jz_dump_regs(struct i2c_jz *i2c)
{
	struct i2c_jz *i2c_id = i2c;

	PRINT_REG_WITH_ID(I2C_CTRL, i2c_id);
	PRINT_REG_WITH_ID(I2C_INTM, i2c_id);
	PRINT_REG_WITH_ID(I2C_RXTL, i2c_id);
	PRINT_REG_WITH_ID(I2C_TXTL, i2c_id);
	PRINT_REG_WITH_ID(0x78, i2c_id);
	return;

	PRINT_REG_WITH_ID(I2C_CTRL, i2c_id);
	PRINT_REG_WITH_ID(I2C_TAR, i2c_id);
	PRINT_REG_WITH_ID(I2C_SAR, i2c_id);
	//      PRINT_REG_WITH_ID(I2C_DC, i2c_id);
	PRINT_REG_WITH_ID(I2C_SHCNT, i2c_id);
	PRINT_REG_WITH_ID(I2C_SLCNT, i2c_id);
	PRINT_REG_WITH_ID(I2C_FHCNT, i2c_id);
	PRINT_REG_WITH_ID(I2C_FLCNT, i2c_id);
	PRINT_REG_WITH_ID(I2C_INTST, i2c_id);
	PRINT_REG_WITH_ID(I2C_INTM, i2c_id);
	PRINT_REG_WITH_ID(I2C_RXTL, i2c_id);
	PRINT_REG_WITH_ID(I2C_TXTL, i2c_id);
	PRINT_REG_WITH_ID(I2C_CINTR, i2c_id);
	PRINT_REG_WITH_ID(I2C_CRXUF, i2c_id);
	PRINT_REG_WITH_ID(I2C_CRXOF, i2c_id);
	PRINT_REG_WITH_ID(I2C_CTXOF, i2c_id);
	PRINT_REG_WITH_ID(I2C_CRXREQ, i2c_id);
	PRINT_REG_WITH_ID(I2C_CTXABRT, i2c_id);
	PRINT_REG_WITH_ID(I2C_CRXDONE, i2c_id);
	PRINT_REG_WITH_ID(I2C_CACT, i2c_id);
	PRINT_REG_WITH_ID(I2C_CSTP, i2c_id);
	PRINT_REG_WITH_ID(I2C_CSTT, i2c_id);
	PRINT_REG_WITH_ID(I2C_CGC, i2c_id);
	PRINT_REG_WITH_ID(I2C_ENB, i2c_id);
	PRINT_REG_WITH_ID(I2C_STA, i2c_id);
	/*debug trans & recive fifo count */
	PRINT_REG_WITH_ID(0x74, i2c_id);
	PRINT_REG_WITH_ID(0x78, i2c_id);

	PRINT_REG_WITH_ID(I2C_TXABRT, i2c_id);
	PRINT_REG_WITH_ID(I2C_DMACR, i2c_id);
	PRINT_REG_WITH_ID(I2C_DMATDLR, i2c_id);
	PRINT_REG_WITH_ID(I2C_DMARDLR, i2c_id);
	PRINT_REG_WITH_ID(I2C_SDASU, i2c_id);
	PRINT_REG_WITH_ID(I2C_ACKGC, i2c_id);
	PRINT_REG_WITH_ID(I2C_ENSTA, i2c_id);
	PRINT_REG_WITH_ID(I2C_SDAHD, i2c_id);
}
#endif

static inline unsigned short i2c_readl(struct i2c_jz *i2c,
		unsigned short offset)
{
	return readl(i2c->iomem + offset);
}

static inline void i2c_writel(struct i2c_jz *i2c, unsigned short offset,
		unsigned short value)
{
	writel(value, i2c->iomem + offset);
}

static int i2c_jz_disable(struct i2c_jz *i2c)
{
	int timeout = TIMEOUT;
	i2c_writel(i2c, I2C_ENB, 0);
	while ((i2c_readl(i2c, I2C_ENSTA) & I2C_ENB_I2CENB) && (--timeout > 0))
		msleep(1);

	if (timeout)
		return 0;

	printk("enable i2c%d failed\n", i2c->adap.nr);
	return -ETIMEDOUT;
}

static int i2c_jz_enable(struct i2c_jz *i2c)
{
	int timeout = TIMEOUT;

	i2c_writel(i2c, I2C_ENB, 1);
	while (!(i2c_readl(i2c, I2C_ENSTA) & I2C_ENB_I2CENB) && (--timeout > 0))
		msleep(1);

	if (timeout)
		return 0;

	printk("enable i2c%d failed\n", i2c->adap.nr);
	return -ETIMEDOUT;
}

static void i2c_jz_reset(struct i2c_jz *i2c)
{
	i2c_readl(i2c, I2C_CTXABRT);
	i2c_readl(i2c, I2C_INTST);

	i2c_jz_disable(i2c);
	udelay(10);
	i2c_jz_enable(i2c);
}

/* function: send read command
 * return:	0, successful
 *		1, txfifo valid entry is more than receive fifo, before send read command,
 *			must be read.
 *		2, txfifo count is 0 or rxfifo count is 0.
 * */
static inline unsigned int i2c_send_rcmd(struct i2c_jz *i2c)
{
	unsigned int tx_count, rx_count, count, tx_valid, rx_valid;

	tx_valid = i2c_readl(i2c, I2C_TXFLR);
	rx_valid = i2c_readl(i2c, I2C_RXFLR);
	tx_count = I2C_FIFO_LEN - tx_valid;
	rx_count = I2C_FIFO_LEN - rx_valid;

#ifdef CONFIG_I2C_DEBUG_INFO
	if(i2c->debug > DEBUG_INFO)
		dev_info(&(i2c->adap.dev),
				"%s, tx_valid = %d, rx_valid = %d,"
				" tx_count = %d, rx_count = %d\n",
				__func__, tx_valid, rx_valid, tx_count, rx_count);
#endif

	if (tx_valid > rx_count) {
		dev_warn(&(i2c->adap.dev),
				"\n\n###Warrning: I2C transfer fifo valid entry is more receive fifo, "
				"before send read cmd, please read data from "
				"the read fifo.\n\n");
		return 1;
	}

	if (!tx_count || !rx_count) {
		dev_warn(&(i2c->adap.dev),
				"\n\n###Warrning: I2C receive fifo or transfer fifo is full, "
				"before send read cmd, please read data from "
				"the read fifo or wait some time.\n\n");
		return 2;
	}

	count = min3(i2c->rd_len, tx_count, rx_count);

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_INFO)
		dev_info(&(i2c->adap.dev),
				"%s, Before send read cmd, "
				"need_send = %d, left_send = %d\n",
				__func__, count ,i2c->rd_len);
#endif

	i2c->rd_len -= count;

	if (!i2c->rd_len) {
		while (count > 1) {
			i2c_writel(i2c, I2C_DC, I2C_DC_READ);
			count--;
		}
		if (i2c->r_end_type == MSG_END_STOP) {
			i2c_writel(i2c, I2C_DC, I2C_DC_READ | I2C_DC_STP);
		} else {
			i2c_writel(i2c, I2C_DC, I2C_DC_READ);
		}
	} else {
		while (count > 0) {
			i2c_writel(i2c, I2C_DC, I2C_DC_READ);
			count--;
		}
	}

#ifdef CONFIG_I2C_DEBUG_INFO
	if(i2c->debug > DEBUG_INFO)
		dev_info(&(i2c->adap.dev),
				"%s, After send read cmd, "
				"left_send = %d\n",
				__func__, i2c->rd_len);
#endif
	return 0;
}

#ifdef CONFIG_I2C_DEBUG_INFO
static ssize_t enable_debug(struct device *dev, struct device_attribute *attr,
		                        const char *buf, size_t size)
{
	int rc;
	unsigned long   enable;
	struct i2c_jz *i2c = (struct i2c_jz *)dev_get_drvdata(dev);

	rc = strict_strtoul(buf, 0, &enable);
	if (rc)
		return rc;

	if ((enable >= 0) && (enable <= DEBUG_INFO + 1))
		i2c->debug = enable;
	else
		goto err;

	return size;
err:
	pr_err("Please input correct number(enable >= 0 && enable <= 5)"
			" to disable or enable debug info print\n");
	return -EAGAIN;
}

static struct device_attribute attributes[] = {
	__ATTR(debug_info, 0666, NULL, enable_debug),
};

static int create_debug_sysfs_interface(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto err;
	return 0;

err:
	for( ; i >= 0; i--)
		device_remove_file(dev, attributes + i);
	return -1;
}
#endif

static irqreturn_t i2c_jz_irq(int irqno, void *dev_id)
{
	unsigned short tmp, intst, intmsk;
	struct i2c_jz *i2c = dev_id;

	intst = i2c_readl(i2c, I2C_INTST);
	intmsk = i2c_readl(i2c, I2C_INTM);

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_INFO)
		dev_info(&(i2c->adap.dev), "--I2C irq register INTST:0x%08x\n", intst);
#endif

	if ((intst & I2C_INTST_TXABT) && (intmsk & I2C_INTM_MTXABT)) {
		dev_err(&(i2c->adap.dev),
				"%s %d, I2C transfer error, ABORT interrupt\n",
				__func__, __LINE__);
		goto END_TRSF_IRQ_HND;
	}

	if ((intst & I2C_INTST_ISTP) && (intmsk & I2C_INTM_MISTP)) {
		i2c_readl(i2c, I2C_CSTP);	/* clear STP bit */
#ifdef CONFIG_I2C_DEBUG_INFO
		if (i2c->debug > DEBUG_INFO) {
			dev_info(&(i2c->adap.dev),
					"%s, Now stop condition has occurred,"
					"and left data length is %d\n",
					__func__, i2c->len);
		}
#endif

		if (i2c->len == 0)
			goto END_TRSF_IRQ_HND;
	}

	if ((intmsk & I2C_INTM_MTXEMP) && (intst & I2C_INTST_TXEMP)) {
		if (!i2c->len) {
			if (i2c->w_end_type == MSG_END_REPEAT_START) {
				goto END_TRSF_IRQ_HND;
			} else {
				tmp = i2c_readl(i2c, I2C_INTM);
				tmp &= ~I2C_INTM_MTXEMP;
				i2c_writel(i2c, I2C_INTM, tmp);
			}
		} else {
			while ((i2c->len > 0) &&
					(i2c_readl(i2c, I2C_STA) & I2C_STA_TFNF)) {
				tmp = *i2c->wbuf++;
				if (i2c->len == 1) {
					if (i2c->w_end_type == MSG_END_STOP)
						tmp |= I2C_DC_STP;
				}

				i2c_writel(i2c, I2C_DC, tmp);
				i2c->len -= 1;
			}

			if (i2c->len == 0) {
				i2c_writel(i2c, I2C_TXTL, 0);
			}
		}
	}

	if ((intst & I2C_INTST_RXFL) && (intmsk & I2C_INTM_MRXFL)) {
#ifdef CONFIG_I2C_DEBUG_INFO
		if (i2c->debug >= DEBUG_INFO) {
			dev_info(&(i2c->adap.dev),
					"%s, Before read I2C_DC, "
					"left_send_cmd = %d, left_read_cnt = %d,"
					" rx_valid = %d, tx_valid = %d\n",
					__func__, i2c->rd_len, i2c->len,
					i2c_readl(i2c, I2C_RXFLR), i2c_readl(i2c, I2C_TXFLR));
		}
#endif

		while ((i2c_readl(i2c, I2C_STA) & I2C_STA_RFNE) &&
				(i2c->len > 0)) {
			tmp = i2c_readl(i2c, I2C_DC) & 0xff;
			*i2c->rbuf++ = tmp;
			i2c->len--;
		}

#ifdef CONFIG_I2C_DEBUG_INFO
		if (i2c->debug >= DEBUG_INFO)
			dev_info(&(i2c->adap.dev),
					"%s, After read I2C_DC, "
					"left_read_cnt = %d,"
					" rx_valid = %d, tx_valid = %d\n",
					__func__, i2c->len,
					i2c_readl(i2c, I2C_RXFLR), i2c_readl(i2c, I2C_TXFLR));
#endif

		if (i2c->len == 0) {
			goto END_RECE_IRQ_HND;
		}

		if (i2c->len <= I2C_FIFO_LEN) {
			i2c_writel(i2c, I2C_RXTL, i2c->len - 1);
		}

		if (i2c_send_rcmd(i2c)) {
			dev_err(&(i2c->adap.dev),
					"%s %d, I2C controller has BUG,"
					" RXFLR or TXFLR can not clear\n",
					__func__, __LINE__);
			BUG();
		}

#ifdef CONFIG_I2C_DEBUG_INFO
		if (i2c->debug > DEBUG_INFO)
			dev_info(&(i2c->adap.dev),
					"%s, After send read command, "
					"left_send_cmd = %d, "
					"left_read_cnt = %d\n",
					__func__, i2c->rd_len, i2c->len);
#endif
	}

	if ((intst & I2C_INTST_RXOF) && (intmsk & I2C_INTM_MRXOF)) {
		dev_err(&(i2c->adap.dev),
				"%s %d, I2C transfer error, RXFIFO over full\n",
				__func__, __LINE__);
		i2c_readl(i2c, I2C_CRXOF);	/* clear RXOF bit */
	}

	if ((intst & I2C_INTST_TXOF) && (intmsk & I2C_INTM_MTXOF)) {
		dev_err(&(i2c->adap.dev),
				"%s %d, I2C transfer error, TXFIFO over full\n",
				__func__, __LINE__);
		i2c_readl(i2c, I2C_CTXOF);	/* clear TXOF bit */
		goto END_TRSF_IRQ_HND;
	}

	return IRQ_HANDLED;
END_RECE_IRQ_HND:
END_TRSF_IRQ_HND:
	i2c_writel(i2c, I2C_INTM, 0);
	complete(&i2c->complete);
	return IRQ_HANDLED;
}

static void txabrt(struct i2c_jz *i2c, int src)
{
	int i;

	dev_err(&(i2c->adap.dev), "--I2C txabrt:\n");
	for (i = 0; i < 16; i++) {
		if (src & (0x1 << i))
			dev_info(&(i2c->adap.dev), "--I2C TXABRT[%d]=%s\n", i,
					abrt_src[i]);
	}
}

static inline int xfer_read(struct i2c_jz *i2c, unsigned char *buf, int len,
		enum msg_end_type end_type)
{
	int ret = 0;
	long timeout;
	unsigned short tmp;
	unsigned int wait_complete_timeout_ms;

	wait_complete_timeout_ms =
		len * 1000 * 9 * 2 / i2c->rate + CONFIG_I2C_JZV10_WAIT_MS;

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_WARN)
		dev_info(&(i2c->adap.dev),
				"%s, Begin read msg, want to read length is %d\n",
				__func__, len);
#endif

	memset(buf, 0, len);

	i2c->rd_len = len;
	i2c->len = len;
	i2c->rbuf = buf;
	i2c->r_end_type = end_type;

	i2c_readl(i2c, I2C_CSTP);	/* clear STP bit */
	i2c_readl(i2c, I2C_CTXOF);	/* clear TXOF bit */
	i2c_readl(i2c, I2C_CTXABRT);	/* clear TXABRT bit */

	if (len <= I2C_FIFO_LEN) {
		i2c_writel(i2c, I2C_RXTL, len - 1);
	} else {
		i2c_writel(i2c, I2C_RXTL, RX_LEVEL);
	}

	while (i2c_readl(i2c, I2C_STA) & I2C_STA_RFNE) {
		i2c_readl(i2c, I2C_DC);
	}
	if (i2c_send_rcmd(i2c))
		BUG();

	tmp = I2C_INTM_MRXFL | I2C_INTM_MTXABT;
	if (end_type == MSG_END_STOP)
		tmp |= I2C_INTM_MISTP;
	i2c_writel(i2c, I2C_INTM, tmp);

	timeout = wait_for_completion_timeout(&i2c->complete,
			msecs_to_jiffies
			(wait_complete_timeout_ms));
	if (!timeout) {
		dev_err(&(i2c->adap.dev), "--I2C irq read timeout\n");
#ifdef I2C_DEBUG
		i2c_jz_dump_regs(i2c);
#endif
		ret = -ETIMEDOUT;
	}

	tmp = i2c_readl(i2c, I2C_TXABRT);
	if (tmp) {
		txabrt(i2c, tmp);
		if (tmp > 0x1 && tmp < 0x10)
			ret = -ENXIO;
		else
			ret = -EIO;
		// ABRT_GCALL_READ
		if (tmp & (1 << 5)) {
			ret = -EAGAIN;
		}
		i2c_readl(i2c, I2C_CTXABRT);
	}
	if (ret < 0)
		i2c_jz_reset(i2c);

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_WARN)
		dev_info(&(i2c->adap.dev),
				"%s, Reading msg over\n", __func__);
#endif

	return ret;
}

static inline int xfer_write(struct i2c_jz *i2c, unsigned char *buf, int len,
		enum msg_end_type end_type)
{
	int ret = 0;
	long timeout = TIMEOUT;
	unsigned short reg_tmp;
	unsigned int wait_complete_timeout_ms;

	wait_complete_timeout_ms =
		len * 1000 * 9 * 2 / i2c->rate + CONFIG_I2C_JZV10_WAIT_MS;

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_WARN)
		dev_info(&(i2c->adap.dev),
				"%s, Begin write msg, want to write length is %d\n",
				__func__, len);
#endif

	i2c->wbuf = buf;
	i2c->len = len;

	i2c_writel(i2c, I2C_TXTL, TX_LEVEL);

	i2c_readl(i2c, I2C_CSTP);	/* clear STP bit */
	i2c_readl(i2c, I2C_CTXOF);	/* clear TXOF bit */
	i2c_readl(i2c, I2C_CTXABRT);	/* clear TXABRT bit */

	i2c->w_end_type = end_type;
	while ((i2c_readl(i2c, I2C_STA) & I2C_STA_TFNF) && (i2c->len > 0)) {
		reg_tmp = *i2c->wbuf++;
		if (i2c->len == 1) {
			if (end_type == MSG_END_STOP) {
				reg_tmp |= I2C_DC_STP;
			}
		}
		i2c_writel(i2c, I2C_DC, reg_tmp);

		i2c->len -= 1;
	}

	if (i2c->len == 0) {
		i2c_writel(i2c, I2C_TXTL, 0);
	}

	reg_tmp = I2C_INTM_MTXEMP | I2C_INTM_MTXABT | I2C_INTM_MTXOF;
	if (end_type == MSG_END_STOP)
		reg_tmp |= I2C_INTM_MISTP;

	i2c_writel(i2c, I2C_INTM, reg_tmp);

	timeout = wait_for_completion_timeout(&i2c->complete,
			msecs_to_jiffies
			(wait_complete_timeout_ms));

	if (!timeout) {
		dev_err(&(i2c->adap.dev), "--I2C pio write wait timeout\n");
#ifdef I2C_DEBUG
		i2c_jz_dump_regs(i2c);
#endif
		i2c_jz_reset(i2c);
		ret = -ETIMEDOUT;
	}

	reg_tmp = i2c_readl(i2c, I2C_TXABRT);
	if (reg_tmp) {
		txabrt(i2c, reg_tmp);
		if (reg_tmp > 0x1 && reg_tmp < 0x10)
			ret = -ENXIO;
		else
			ret = -EIO;
		//after I2C_TXABRT_ABRT_XDATA_NOACK error,this required core to resend
		if (reg_tmp & 8) {
			ret = -EAGAIN;
		}
		i2c_readl(i2c, I2C_CTXABRT);
	}

	if (ret < 0)
		i2c_jz_reset(i2c);

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_WARN)
		dev_info(&(i2c->adap.dev),
				"%s, Write msg over\n", __func__);
#endif
	return ret;
}

static int i2c_disable_clk(struct i2c_jz *i2c)
{
	int timeout = 10;
	int tmp = i2c_readl(i2c, I2C_STA);

	while ((tmp & I2C_STA_MSTACT) && (--timeout > 0)) {
		udelay(90);
		tmp = i2c_readl(i2c, I2C_STA);
	}
	if (timeout > 0) {
		clk_disable(i2c->clk);
		return 0;
	} else {
		dev_err(&(i2c->adap.dev),
				"--I2C disable clk timeout, I2C_STA = 0x%x\n", tmp);
		i2c_jz_reset(i2c);
		clk_disable(i2c->clk);
		return -ETIMEDOUT;
	}
}

static int i2c_jz_xfer(struct i2c_adapter *adap, struct i2c_msg *msg, int count)
{
	int i, ret = 0, len = 0;
	struct i2c_jz *i2c = adap->algo_data;

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_WARN) {
		printk("\n\n\n");
		dev_info(&(i2c->adap.dev),
				"%s, Begin master xfer, want to transfer msg count is %d\n",
				__func__, count);
	}
#endif
	clk_enable(i2c->clk);

	i2c_writel(i2c, I2C_TAR, msg->addr);

	for (i = 0; i < count; i++, msg++) {
		enum msg_end_type end_type = MSG_END_STOP;
		if (i < (count - 1)) {
			if (msg[i + 1].flags & I2C_M_NOSTART) {
				end_type = MSG_END_CONTINUE;	/* have no STOP and START */
			} else {
				end_type = MSG_END_REPEAT_START;	/* have no STOP but have RESTART */
			}
		}

		INIT_COMPLETION(i2c->complete);

#ifdef CONFIG_I2C_DEBUG_INFO
		if (i2c->debug > DEBUG_WARN)
			dev_info(&(i2c->adap.dev),
					"%s, Now transfer msg: %d\n", __func__, i);
#endif
		if (msg->flags & I2C_M_RD) {
			bool true = msg->flags & I2C_M_RECV_LEN;
			if(true){
				ret = xfer_read(i2c, msg->buf, msg->len, MSG_END_CONTINUE);
				len = msg->buf[0];
				ret = xfer_read(i2c, &(msg->buf[msg->len]), len, end_type);
				msg->len = msg->len + len;
			}
			else{
				ret = xfer_read(i2c, msg->buf, msg->len, end_type);
			}
		} else {
			ret = xfer_write(i2c, msg->buf, msg->len, end_type);
		}
		if (ret < 0) {
			clk_disable(i2c->clk);
			goto ERR;
		}
	}

	if (i2c_disable_clk(i2c)) {
		ret = -ETIMEDOUT;
		goto ERR;
	}

#ifdef CONFIG_I2C_DEBUG_INFO
	if (i2c->debug > DEBUG_WARN)
		dev_info(&(i2c->adap.dev),
				"%s, Transfer msg over\n\n\n", __func__);
#endif

ERR:
	return ret ? : i;
}

static u32 i2c_jz_functionality(struct i2c_adapter *adap)
{
	unsigned int ret;
	ret = I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_READ_BLOCK_DATA;

	return ret;
}

static const struct i2c_algorithm i2c_jz_algorithm = {
	.master_xfer = i2c_jz_xfer,
	.functionality = i2c_jz_functionality,
};

static int i2c_set_speed(struct i2c_jz *i2c, int rate)
{
	/*ns */
	long dev_clk = clk_get_rate(i2c->clk);
	long cnt_high = 0;	/* HIGH period count of the SCL clock */
	long cnt_low = 0;	/* LOW period count of the SCL clock */
	long setup_time = 0;
	long hold_time = 0;
	unsigned short tmp;

	i2c->rate = rate;
	if (i2c_jz_disable(i2c))
		dev_info(&(i2c->adap.dev), "i2c not disable\n");
	if (rate <= 100000) {
		tmp = 0x43 | (1 << 5);	/* standard speed mode */
		i2c_writel(i2c, I2C_CTRL, tmp);
	} else {
		tmp = 0x45 | (1 << 5);	/* fast speed mode */
		i2c_writel(i2c, I2C_CTRL, tmp);
	}

	/*         high
	 *         ____     ____      ____      ____
	 *  clk __|  | |___|    |____|    |____|    |___
	 *           | | |
	 *           | | |
	 *           |_|_|     _________      ____
	 * data    __/ | |\___/         \____/    \____
	 *    setup->| |<|
	 *           ->| |<-hold
	 */

	//setup_time = (10 000 000/(rate*4)) + 1;
	setup_time = (dev_clk / (rate * 4));
	if (setup_time > 1)
		setup_time -= 1;
	//hold_time =  (10000000/(rate*4)) - 1;
	hold_time = (dev_clk / (rate * 4));

	/*         high
	 *         ____     ____
	 *  clk __|    |___|    |____
	 *              low
	 *        |<--period--->|
	 *
	 */
	cnt_high = dev_clk / (rate * 2);
	cnt_low = dev_clk / (rate * 2);

	dev_info(&(i2c->adap.dev), "set:%ld  hold:%ld dev=%ld h=%ld l=%ld\n",
			setup_time, hold_time, dev_clk, cnt_high, cnt_low);
	if (setup_time > 255)
		setup_time = 255;
	if (setup_time <= 0)
		setup_time = 1;
	if (hold_time > 0xFFFF)
		hold_time = 0xFFFF;

	if (rate <= 100000) {
		i2c_writel(i2c, I2C_SHCNT, I2CSHCNT_ADJUST(cnt_high));
		i2c_writel(i2c, I2C_SLCNT, I2CSLCNT_ADJUST(cnt_low));
	} else {
		i2c_writel(i2c, I2C_FHCNT, I2CFHCNT_ADJUST(cnt_high));
		i2c_writel(i2c, I2C_FLCNT, I2CFLCNT_ADJUST(cnt_low));
	}

	i2c_writel(i2c, I2C_SDASU, setup_time & 0xff);
	i2c_writel(i2c, I2C_SDAHD, hold_time);

	return 0;
}

static int i2c_jz_probe(struct platform_device *dev)
{
	int ret = 0;
	struct i2c_jz *i2c;
	struct resource *res;
	unsigned int reg_tmp;

	i2c = kzalloc(sizeof(struct i2c_jz), GFP_KERNEL);
	if (!i2c) {
		printk("Error: Now we can not malloc memory for I2C!\n");
		ret = -ENOMEM;
		goto ERR0;
	}

	i2c->adap.owner = THIS_MODULE;
	i2c->adap.algo = &i2c_jz_algorithm;
	i2c->adap.retries = 5;
	i2c->adap.timeout = 5;
	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &dev->dev;
	i2c->adap.nr = dev->id;
	sprintf(i2c->adap.name, "i2c%u", dev->id);

	i2c->clk = clk_get(&dev->dev, i2c->adap.name);
	if (!i2c->clk) {
		printk("Error: Now we can not get i2c%d clock!\n", dev->id);
		ret = -ENODEV;
		goto clk_failed;
	}

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	i2c->iomem = ioremap(res->start, resource_size(res));
	if (!i2c->iomem) {
		printk("Error: Now we can remap IO for I2C%d!\n", dev->id);
		ret = -ENOMEM;
		goto io_failed;
	}

	i2c->irq = platform_get_irq(dev, 0);
	ret =
		request_irq(i2c->irq, i2c_jz_irq, IRQF_DISABLED,
				dev_name(&dev->dev), i2c);
	if (ret) {
		printk("Error: Now we can request irq for I2C%d!\n", dev->id);
		ret = -ENODEV;
		goto irq_failed;
	}

	clk_enable(i2c->clk);

	res = platform_get_resource(dev, IORESOURCE_BUS, 0);

	i2c_set_speed(i2c, res->start * 1000);

#if 0
	reg_tmp = i2c_readl(i2c, I2C_DC);
	reg_tmp &= ~I2C_DC_STP;
	i2c_writel(i2c, I2C_DC, reg_tmp);
#endif

	reg_tmp = i2c_readl(i2c, I2C_CTRL);
	reg_tmp |= I2C_CTRL_REST;
	i2c_writel(i2c, I2C_CTRL, reg_tmp);

	// for jgao WHY?
	//  i2c_writel(i2c, I2C_FLT, 0xF);      /*set filter*/

	i2c_writel(i2c, I2C_INTM, 0x0);

	init_completion(&i2c->complete);

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&(i2c->adap.dev), KERN_INFO "I2C: Failed to add bus\n");
		goto adapt_failed;
	}

	platform_set_drvdata(dev, i2c);

	i2c_jz_enable(i2c);

	clk_disable(i2c->clk);

#ifdef CONFIG_I2C_DEBUG_INFO
	ret = create_debug_sysfs_interface(&dev->dev);
	if (ret < 0)
		dev_err(&i2c->adap.dev, "create debug sysfs interface failed\n");
#endif

	return 0;

adapt_failed:
	free_irq(i2c->irq, i2c);
irq_failed:
	iounmap(i2c->iomem);
io_failed:
	clk_put(i2c->clk);
clk_failed:
	kfree(i2c);
ERR0:
	return ret;
}

static int i2c_jz_remove(struct platform_device *dev)
{
	struct i2c_jz *i2c = platform_get_drvdata(dev);

	free_irq(i2c->irq, i2c);
	iounmap(i2c->iomem);
	clk_put(i2c->clk);
	i2c_del_adapter(&i2c->adap);
	kfree(i2c);

	return 0;
}

static struct platform_driver i2c_jz_driver = {
	.probe = i2c_jz_probe,
	.remove = i2c_jz_remove,
	.driver = {
		.name = "jz-i2c",
	},
};

static int __init i2c_jz_init(void)
{
	return platform_driver_register(&i2c_jz_driver);
}

subsys_initcall(i2c_jz_init);

static void __exit i2c_jz_exit(void)
{
	platform_driver_unregister(&i2c_jz_driver);
}

module_exit(i2c_jz_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("i2c driver for JZ SoCs");
