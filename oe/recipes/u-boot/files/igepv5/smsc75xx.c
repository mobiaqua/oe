/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (C) 2009 NVIDIA, Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define DEBUG

#include <asm/unaligned.h>
#include <common.h>
#include <usb.h>
#include <linux/mii.h>
#include "usb_ether.h"
#include <malloc.h>
#include <memalign.h>

/* SMSC LAN75xx based USB 2.0 Ethernet Devices */

/* Tx command words */
#define TX_CMD_A_FIRST_SEG_		0x00002000
#define TX_CMD_A_LAST_SEG_		0x00001000

/* Rx status word */
#define RX_STS_FL_			0x3FFF0000	/* Frame Length */
#define RX_STS_ES_			0x00008000	/* Error Summary */

/* SCSRs */
// #define ID_REV				0x00

// #define INT_STS				0x0C

#define TX_CFG				0x10
#define TX_CFG_ON_			0x00000004

//#define HW_CFG				0x14
#define HW_CFG_BIR_			0x00000080
#define HW_CFG_RXDOFF_		0x00000600
#define HW_CFG_MEF_			0x00000010
#define HW_CFG_BCE_			0x00000004
#define HW_CFG_LRST_		0x00000002

#ifdef __notdef
#define HW_CFG_LEDB			(0x00000100)
#define HW_CFG_BIR			(0x00000080)
#define HW_CFG_SBP			(0x00000040)
#define HW_CFG_IME			(0x00000020)
#define HW_CFG_MEF			(0x00000010)
#define HW_CFG_ETC			(0x00000008)
#define HW_CFG_BCE			(0x00000004)
#define HW_CFG_LRST			(0x00000002)
#define HW_CFG_SRST			(0x00000001)
#endif

#define PM_CTRL				0x0014
#define PM_CTL_PHY_RST_		0x00000010

#define AFC_CFG				0x2C

/*
 * Hi watermark = 15.5Kb (~10 mtu pkts)
 * low watermark = 3k (~2 mtu pkts)
 * backpressure duration = ~ 350us
 * Apply FC on any frame.
 */
#define AFC_CFG_DEFAULT			0x00F830A1

// #define E2P_CMD					0x40
#define E2P_CMD_BUSY_			0x80000000								
#define E2P_CMD_READ_			0x00000000
#define E2P_CMD_TIMEOUT_		0x00000400
#define E2P_CMD_LOADED_			0x00000200
#define E2P_CMD_ADDR_			0x000001FF

#define E2P_DATA				0x34

#define BURST_CAP				0x38

#define INT_EP_CTL				0x38
#define INT_EP_CTL_PHY_INT_		0x00008000

#define BULK_IN_DLY				0x3C

/* MAC CSRs */
#define MAC_CR_MCPAS_			0x00080000
#define MAC_CR_PRMS_			0x00040000
#define MAC_CR_HPFILT_			0x00002000
#define MAC_CR_TXEN_			0x00000008
#define MAC_CR_RXEN_			0x00000004

#define ADDRH					0x118
#define ADDRL					0x11C

#define MII_ADDR				0x120
#define MII_WRITE_				0x02
#define MII_BUSY_				0x01
#define MII_READ_				0x00 /* ~of MII Write bit */

// #define MII_DATA				0x124

#define FLOW					0x0A0

#define VLAN1				0x120

#define COE_CR				0x130
#define Tx_COE_EN_			0x00010000
#define Rx_COE_EN_			0x00000001

/* Vendor-specific PHY Definitions */
// #define PHY_INT_SRC			29

// #define PHY_INT_MASK			30
#define PHY_INT_MASK_ANEG_COMP_		((u16)0x0040)
#define PHY_INT_MASK_LINK_DOWN_		((u16)0x0010)
#define PHY_INT_MASK_DEFAULT_		(PHY_INT_MASK_ANEG_COMP_ | \
					 PHY_INT_MASK_LINK_DOWN_)

/* USB Vendor Requests */
#define USB_VENDOR_REQUEST_WRITE_REGISTER	0xA0
#define USB_VENDOR_REQUEST_READ_REGISTER	0xA1

/* Some extra defines */
#define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */

#define MAX_RX_FIFO_SIZE			(20 * 1024)
#define MAX_TX_FIFO_SIZE			(12 * 1024)
#define SMSC_CHIPNAME				"smsc75xx"
#define SMSC_DRIVER_VERSION			"1.0.0"
#define HS_USB_PKT_SIZE				(512)
#define FS_USB_PKT_SIZE				(64)
#define DEFAULT_HS_BURST_CAP_SIZE	(16 * 1024 + 5 * HS_USB_PKT_SIZE)
#define DEFAULT_FS_BURST_CAP_SIZE	(6 * 1024 + 33 * FS_USB_PKT_SIZE)
#define DEFAULT_BULK_IN_DELAY		(0x00002000)
#define MAX_SINGLE_PACKET_SIZE		(9000)
#define LAN75XX_EEPROM_MAGIC		(0x7500)
#define EEPROM_MAC_OFFSET			(0x01)
#define DEFAULT_TX_CSUM_ENABLE		(true)
#define DEFAULT_RX_CSUM_ENABLE		(true)
#define DEFAULT_TSO_ENABLE			(true)
#define SMSC75XX_INTERNAL_PHY_ID	(1)
#define smsc75xx_INTERNAL_PHY_ID	SMSC75XX_INTERNAL_PHY_ID
#define SMSC75XX_TX_OVERHEAD		(8)
#define MAX_RX_FIFO_SIZE			(20 * 1024)
#define MAX_TX_FIFO_SIZE			(12 * 1024)
#define USB_VENDOR_ID_SMSC			(0x0424)
#define USB_PRODUCT_ID_LAN7500		(0x7500)
#define USB_PRODUCT_ID_LAN7505		(0x7505)
#define RXW_PADDING					2


/* local defines */
#define smsc75xx_BASE_NAME "sms"
#define USB_CTRL_SET_TIMEOUT 5000
#define USB_CTRL_GET_TIMEOUT 5000
#define USB_BULK_SEND_TIMEOUT 5000
#define USB_BULK_RECV_TIMEOUT 5000

#define AX_RX_URB_SIZE 2048
#define PHY_CONNECT_TIMEOUT 5000

#define TURBO_MODE

/* Defines */
#define MII_ACCESS					(0x120)
#define MII_ACCESS_PHY_ADDR			(0x0000F800)
#define MII_ACCESS_PHY_ADDR_SHIFT	(11)
#define MII_ACCESS_REG_ADDR			(0x000007C0)
#define MII_ACCESS_REG_ADDR_SHIFT	(6)
#define MII_ACCESS_READ				(0x00000000)
#define MII_ACCESS_WRITE			(0x00000002)
#define MII_ACCESS_BUSY				(0x00000001)

#define MII_DATA					(0x124)
#define MII_DATA_MASK				(0x0000FFFF)

#define RX_ADDRH					(0x118)
#define RX_ADDRH_MASK				(0x0000FFFF)
#define RX_ADDRL					(0x11C)

/* Mode Control/Status Register */
#define PHY_MODE_CTRL_STS			(17)
#define MODE_CTRL_STS_EDPWRDOWN		((u16)0x2000)
#define MODE_CTRL_STS_ENERGYON		((u16)0x0002)

#define PHY_INT_SRC					(29)
#define PHY_INT_SRC_ENERGY_ON		((u16)0x0080)
#define PHY_INT_SRC_ANEG_COMP		((u16)0x0040)
#define PHY_INT_SRC_REMOTE_FAULT	((u16)0x0020)
#define PHY_INT_SRC_LINK_DOWN		((u16)0x0010)
#define PHY_INT_SRC_CLEAR_ALL		((u16)0xffff)

#define PHY_INT_MASK				(30)
#define PHY_INT_MASK_ENERGY_ON		((u16)0x0080)
#define PHY_INT_MASK_ANEG_COMP		((u16)0x0040)
#define PHY_INT_MASK_REMOTE_FAULT	((u16)0x0020)
#define PHY_INT_MASK_LINK_DOWN		((u16)0x0010)
#define PHY_INT_MASK_DEFAULT		(PHY_INT_MASK_ANEG_COMP | \
									PHY_INT_MASK_LINK_DOWN)

#define PHY_SPECIAL					(31)
#define PHY_SPECIAL_SPD				((u16)0x001C)
#define PHY_SPECIAL_SPD_10HALF		((u16)0x0004)
#define PHY_SPECIAL_SPD_10FULL		((u16)0x0014)
#define PHY_SPECIAL_SPD_100HALF		((u16)0x0008)
#define PHY_SPECIAL_SPD_100FULL		((u16)0x0018)

#define ADDR_FILTX					(0x300)
#define ADDR_FILTX_FB_VALID			(0x80000000)
#define ADDR_FILTX_FB_TYPE			(0x40000000)
#define ADDR_FILTX_FB_ADDRHI		(0x0000FFFF)
#define ADDR_FILTX_SB_ADDRLO		(0xFFFFFFFF)

#define RFE_CTL						(0x0060)
#define RFE_CTL_TCPUDP_CKM			(0x00001000)
#define RFE_CTL_IP_CKM				(0x00000800)
#define RFE_CTL_AB					(0x00000400)
#define RFE_CTL_AM					(0x00000200)
#define RFE_CTL_AU					(0x00000100)
#define RFE_CTL_VS					(0x00000080)
#define RFE_CTL_UF					(0x00000040)
#define RFE_CTL_VF					(0x00000020)
#define RFE_CTL_SPF					(0x00000010)
#define RFE_CTL_MHF					(0x00000008)
#define RFE_CTL_DHF					(0x00000004)
#define RFE_CTL_DPF					(0x00000002)
#define RFE_CTL_RST_RF				(0x00000001)

#define PMT_CTL						(0x0014)
#define PMT_CTL_PHY_PWRUP			(0x00000400)
#define PMT_CTL_RES_CLR_WKP_EN		(0x00000100)
#define PMT_CTL_DEV_RDY				(0x00000080)
#define PMT_CTL_SUS_MODE			(0x00000060)
#define PMT_CTL_SUS_MODE_0			(0x00000000)
#define PMT_CTL_SUS_MODE_1			(0x00000020)
#define PMT_CTL_SUS_MODE_2			(0x00000040)
#define PMT_CTL_SUS_MODE_3			(0x00000060)
#define PMT_CTL_PHY_RST				(0x00000010)
#define PMT_CTL_WOL_EN				(0x00000008)
#define PMT_CTL_ED_EN				(0x00000004)
#define PMT_CTL_WUPS				(0x00000003)
#define PMT_CTL_WUPS_NO				(0x00000000)
#define PMT_CTL_WUPS_ED				(0x00000001)
#define PMT_CTL_WUPS_WOL			(0x00000002)
#define PMT_CTL_WUPS_MULTI			(0x00000003)

#define HW_CFG				(0x0010)
#define HW_CFG_SMDET_STS		(0x00008000)
#define HW_CFG_SMDET_EN			(0x00004000)
#define HW_CFG_EEM			(0x00002000)
#define HW_CFG_RST_PROTECT		(0x00001000)
#define HW_CFG_PORT_SWAP		(0x00000800)
#define HW_CFG_PHY_BOOST		(0x00000600)
#define HW_CFG_PHY_BOOST_NORMAL		(0x00000000)
#define HW_CFG_PHY_BOOST_4		(0x00002000)
#define HW_CFG_PHY_BOOST_8		(0x00004000)
#define HW_CFG_PHY_BOOST_12		(0x00006000)
#define HW_CFG_LEDB			(0x00000100)
#define HW_CFG_BIR			(0x00000080)
#define HW_CFG_SBP			(0x00000040)
#define HW_CFG_IME			(0x00000020)
#define HW_CFG_MEF			(0x00000010)
#define HW_CFG_ETC			(0x00000008)
#define HW_CFG_BCE			(0x00000004)
#define HW_CFG_LRST			(0x00000002)
#define HW_CFG_SRST			(0x00000001)

#define FCT_RX_CTL			(0x0090)
#define FCT_RX_CTL_EN			(0x80000000)
#define FCT_RX_CTL_RST			(0x40000000)
#define FCT_RX_CTL_SBF			(0x02000000)
#define FCT_RX_CTL_OVERFLOW		(0x01000000)
#define FCT_RX_CTL_FRM_DROP		(0x00800000)
#define FCT_RX_CTL_RX_NOT_EMPTY		(0x00400000)
#define FCT_RX_CTL_RX_EMPTY		(0x00200000)
#define FCT_RX_CTL_RX_DISABLED		(0x00100000)
#define FCT_RX_CTL_RXUSED		(0x0000FFFF)

#define FCT_TX_CTL			(0x0094)
#define FCT_TX_CTL_EN			(0x80000000)
#define FCT_TX_CTL_RST			(0x40000000)
#define FCT_TX_CTL_TX_NOT_EMPTY		(0x00400000)
#define FCT_TX_CTL_TX_EMPTY		(0x00200000)
#define FCT_TX_CTL_TX_DISABLED		(0x00100000)
#define FCT_TX_CTL_TXUSED		(0x0000FFFF)

#define FCT_RX_FIFO_END			(0x0098)
#define FCT_RX_FIFO_END_MASK		(0x0000007F)

#define FCT_TX_FIFO_END			(0x009C)
#define FCT_TX_FIFO_END_MASK		(0x0000003F)

#define FCT_FLOW			(0x00A0)
#define FCT_FLOW_THRESHOLD_OFF		(0x00007F00)
#define FCT_FLOW_THRESHOLD_OFF_SHIFT	(8)
#define FCT_FLOW_THRESHOLD_ON		(0x0000007F)

#define ID_REV				(0x0000)

#define FPGA_REV			(0x0004)

#define BOND_CTL			(0x0008)

#define INT_STS				(0x000C)
#define INT_STS_RDFO_INT		(0x00400000)
#define INT_STS_TXE_INT			(0x00200000)
#define INT_STS_MACRTO_INT		(0x00100000)
#define INT_STS_TX_DIS_INT		(0x00080000)
#define INT_STS_RX_DIS_INT		(0x00040000)
#define INT_STS_PHY_INT_		(0x00020000)
#define INT_STS_MAC_ERR_INT		(0x00008000)
#define INT_STS_TDFU			(0x00004000)
#define INT_STS_TDFO			(0x00002000)
#define INT_STS_GPIOS			(0x00000FFF)
#define INT_STS_CLEAR_ALL		(0xFFFFFFFF)

#define E2P_CMD				(0x0040)
#define E2P_CMD_BUSY			(0x80000000)
#define E2P_CMD_MASK			(0x70000000)
#define E2P_CMD_READ			(0x00000000)
#define E2P_CMD_EWDS			(0x10000000)
#define E2P_CMD_EWEN			(0x20000000)
#define E2P_CMD_WRITE			(0x30000000)
#define E2P_CMD_WRAL			(0x40000000)
#define E2P_CMD_ERASE			(0x50000000)
#define E2P_CMD_ERAL			(0x60000000)
#define E2P_CMD_RELOAD			(0x70000000)
#define E2P_CMD_TIMEOUT			(0x00000400)
#define E2P_CMD_LOADED			(0x00000200)
#define E2P_CMD_ADDR			(0x000001FF)

#define LED_GPIO_CFG			(0x0018)
#define LED_GPIO_CFG_LED2_FUN_SEL	(0x80000000)
#define LED_GPIO_CFG_LED10_FUN_SEL	(0x40000000)
#define LED_GPIO_CFG_LEDGPIO_EN		(0x0000F000)
#define LED_GPIO_CFG_LEDGPIO_EN_0	(0x00001000)
#define LED_GPIO_CFG_LEDGPIO_EN_1	(0x00002000)
#define LED_GPIO_CFG_LEDGPIO_EN_2	(0x00004000)
#define LED_GPIO_CFG_LEDGPIO_EN_3	(0x00008000)
#define LED_GPIO_CFG_GPBUF		(0x00000F00)
#define LED_GPIO_CFG_GPBUF_0		(0x00000100)
#define LED_GPIO_CFG_GPBUF_1		(0x00000200)
#define LED_GPIO_CFG_GPBUF_2		(0x00000400)
#define LED_GPIO_CFG_GPBUF_3		(0x00000800)
#define LED_GPIO_CFG_GPDIR		(0x000000F0)
#define LED_GPIO_CFG_GPDIR_0		(0x00000010)
#define LED_GPIO_CFG_GPDIR_1		(0x00000020)
#define LED_GPIO_CFG_GPDIR_2		(0x00000040)
#define LED_GPIO_CFG_GPDIR_3		(0x00000080)
#define LED_GPIO_CFG_GPDATA		(0x0000000F)
#define LED_GPIO_CFG_GPDATA_0		(0x00000001)
#define LED_GPIO_CFG_GPDATA_1		(0x00000002)
#define LED_GPIO_CFG_GPDATA_2		(0x00000004)
#define LED_GPIO_CFG_GPDATA_3		(0x00000008)

/* Interrupt Endpoint status word bitfields */
#define INT_ENP_RDFO_INT		((u32)BIT(22))
#define INT_ENP_TXE_INT			((u32)BIT(21))
#define INT_ENP_TX_DIS_INT		((u32)BIT(19))
#define INT_ENP_RX_DIS_INT		((u32)BIT(18))
#define INT_ENP_PHY_INT			((u32)BIT(17))
#define INT_ENP_MAC_ERR_INT		((u32)BIT(15))
#define INT_ENP_RX_FIFO_DATA_INT	((u32)BIT(12))

/* MAC CSRs */
#define MAC_CR				(0x100)
#define MAC_CR_ADP			(0x00002000)
#define MAC_CR_ADD			(0x00001000)
#define MAC_CR_ASD			(0x00000800)
#define MAC_CR_INT_LOOP			(0x00000400)
#define MAC_CR_BOLMT			(0x000000C0)
#define MAC_CR_FDPX			(0x00000008)
#define MAC_CR_CFG			(0x00000006)
#define MAC_CR_CFG_10			(0x00000000)
#define MAC_CR_CFG_100			(0x00000002)
#define MAC_CR_CFG_1000			(0x00000004)
#define MAC_CR_RST			(0x00000001)

#define MAC_RX				(0x104)
#define MAC_RX_MAX_SIZE			(0x3FFF0000)
#define MAC_RX_MAX_SIZE_SHIFT		(16)
#define MAC_RX_FCS_STRIP		(0x00000010)
#define MAC_RX_FSE			(0x00000004)
#define MAC_RX_RXD			(0x00000002)
#define MAC_RX_RXEN			(0x00000001)

#define MAC_TX				(0x108)
#define MAC_TX_BFCS			(0x00000004)
#define MAC_TX_TXD			(0x00000002)
#define MAC_TX_TXEN			(0x00000001)

/* Tx command words */
#define TX_CMD_A_LSO			(0x08000000)
#define TX_CMD_A_IPE			(0x04000000)
#define TX_CMD_A_TPE			(0x02000000)
#define TX_CMD_A_IVTG			(0x01000000)
#define TX_CMD_A_RVTG			(0x00800000)
#define TX_CMD_A_FCS			(0x00400000)
#define TX_CMD_A_LEN			(0x000FFFFF)

#define TX_CMD_B_MSS			(0x3FFF0000)
#define TX_CMD_B_MSS_SHIFT		(16)
#define TX_MSS_MIN			((u16)8)
#define TX_CMD_B_VTAG			(0x0000FFFF)

/* Rx command words */
#define RX_CMD_A_ICE			(0x80000000)
#define RX_CMD_A_TCE			(0x40000000)
#define RX_CMD_A_IPV			(0x20000000)
#define RX_CMD_A_PID			(0x18000000)
#define RX_CMD_A_PID_NIP		(0x00000000)
#define RX_CMD_A_PID_TCP		(0x08000000)
#define RX_CMD_A_PID_UDP		(0x10000000)
#define RX_CMD_A_PID_PP			(0x18000000)
#define RX_CMD_A_PFF			(0x04000000)
#define RX_CMD_A_BAM			(0x02000000)
#define RX_CMD_A_MAM			(0x01000000)
#define RX_CMD_A_FVTG			(0x00800000)
#define RX_CMD_A_RED			(0x00400000)
#define RX_CMD_A_RWT			(0x00200000)
#define RX_CMD_A_RUNT			(0x00100000)
#define RX_CMD_A_LONG			(0x00080000)
#define RX_CMD_A_RXE			(0x00040000)
#define RX_CMD_A_DRB			(0x00020000)
#define RX_CMD_A_FCS			(0x00010000)
#define RX_CMD_A_UAM			(0x00008000)
#define RX_CMD_A_LCSM			(0x00004000)
#define RX_CMD_A_LEN			(0x00003FFF)

#define RX_CMD_B_CSUM			(0xFFFF0000)
#define RX_CMD_B_CSUM_SHIFT		(16)
#define RX_CMD_B_VTAG			(0x0000FFFF)


/* local vars */
static int curr_eth_dev; /* index for name of next device detected */

/* driver private */
struct smsc75xx_private {
	size_t rx_urb_size;  	/* maximum USB URB size */
	u32 mac_cr;  			/* MAC control register value */
	int have_hwaddr;  		/* 1 if we have a hardware MAC address */
};

/*
 * smsc75xx infrastructure commands
 */
static int smsc75xx_write_reg(struct ueth_data *dev, u32 index, u32 data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	cpu_to_le32s(&data);
	tmpbuf[0] = data;

	len = usb_control_msg(dev->pusb_dev, usb_sndctrlpipe(dev->pusb_dev, 0),
		USB_VENDOR_REQUEST_WRITE_REGISTER,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		00, index, tmpbuf, sizeof(data), USB_CTRL_SET_TIMEOUT);
	if (len != sizeof(data)) {
		debug("smsc75xx_write_reg failed: index=%d, data=%d, len=%d",
		      index, data, len);
		return -1;
	}
	return 0;
}

static int smsc75xx_read_reg(struct ueth_data *dev, u32 index, u32 *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	len = usb_control_msg(dev->pusb_dev, usb_rcvctrlpipe(dev->pusb_dev, 0),
		USB_VENDOR_REQUEST_READ_REGISTER,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		00, index, tmpbuf, sizeof(data), USB_CTRL_GET_TIMEOUT);
	*data = tmpbuf[0];
	if (len != sizeof(data)) {
		debug("smsc75xx_read_reg failed: index=%d, len=%d",
		      index, len);
		return -1;
	}
	le32_to_cpus(data);
	return 0;
}

/* Loop until the read is completed with timeout */
static int smsc75xx_phy_wait_not_busy (struct ueth_data *dev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc75xx_read_reg(dev, MII_ACCESS, &val);
		if (!(val & MII_ACCESS_BUSY))
			return 0;
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	return -1;
}

static int smsc75xx_wait_ready(struct ueth_data *dev)
{
	int timeout = 0;

	do {
		u32 buf;
		int ret;
		ret = smsc75xx_read_reg(dev, PMT_CTL, &buf);
		if (ret < 0) {
			debug("Failed to read PMT_CTL: %d\n", ret);
			return ret;
		}
		if (buf & PMT_CTL_DEV_RDY)
			return 0;

		udelay(10 * 1000);
		timeout++;
	} while (timeout < 100);

	debug("timeout waiting for device ready\n");
	return -1;
}


static int smsc75xx_mdio_read(struct ueth_data *dev, int phy_id, int idx)
{
	u32 val, addr;
	int ret;

	/* confirm MII not busy */
	if (smsc75xx_phy_wait_not_busy(dev)) {
		debug("MII is busy in smsc75xx_mdio_read\n");
		return -1;
	}
	phy_id &= 0x1f;
	idx &= 0x1f;
	addr = ((phy_id << MII_ACCESS_PHY_ADDR_SHIFT) & MII_ACCESS_PHY_ADDR)
		| ((idx << MII_ACCESS_REG_ADDR_SHIFT) & MII_ACCESS_REG_ADDR)
		| MII_ACCESS_READ | MII_ACCESS_BUSY;
	
	ret = smsc75xx_write_reg(dev, MII_ACCESS, addr);
	if (ret < 0) {
		debug("Error writing MII_ACCESS\n");
		return -1;
	}

	ret = smsc75xx_phy_wait_not_busy(dev);
	if (ret < 0) {
		debug("Timed out reading MII reg %02X\n", idx);
		return -1;
	}

	ret = smsc75xx_read_reg(dev, MII_DATA, &val);
	if (ret < 0) {
		debug("Error reading MII_DATA\n");
		return -1;
	}

	return (u16)(val & 0xFFFF);
}

static void smsc75xx_mdio_write(struct ueth_data *dev, int phy_id, int idx,
				int regval)
{
	u32 val, addr;
	int ret;

	/* confirm MII not busy */
	if (smsc75xx_phy_wait_not_busy(dev)) {
		debug("MII is busy in smsc75xx_mdio_write\n");
		return;
	}

	val = regval;
	ret = smsc75xx_write_reg(dev, MII_DATA, val);
	if (ret < 0) {
		debug("Error writing MII_DATA\n");
		return;
	}
	
	phy_id &= 0x1f;
	idx &= 0x1f;
	addr = ((phy_id << MII_ACCESS_PHY_ADDR_SHIFT) & MII_ACCESS_PHY_ADDR)
		| ((idx << MII_ACCESS_REG_ADDR_SHIFT) & MII_ACCESS_REG_ADDR)
		| MII_ACCESS_WRITE | MII_ACCESS_BUSY;
	ret = smsc75xx_write_reg(dev, MII_ACCESS, addr);	
	if (ret < 0) {
		debug("Error writing MII_ACCESS\n");
		return;
	}

	ret = smsc75xx_phy_wait_not_busy(dev);
	if (ret < 0) {
		debug("Timed out writing MII reg %02X\n", idx);
		return;
	}
}

#ifdef __notdef
static int smsc75xx_eeprom_confirm_not_busy(struct ueth_data *dev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc75xx_read_reg(dev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_))
			return 0;
		udelay(40);
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	debug("EEPROM is busy\n");
	return -1;
}

static int smsc75xx_wait_eeprom(struct ueth_data *dev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc75xx_read_reg(dev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_) || (val & E2P_CMD_TIMEOUT_))
			break;
		udelay(40);
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	if (val & (E2P_CMD_TIMEOUT_ | E2P_CMD_BUSY_)) {
		debug("EEPROM read operation timeout\n");
		return -1;
	}
	return 0;
}

static int smsc75xx_read_eeprom(struct ueth_data *dev, u32 offset, u32 length,
				u8 *data)
{
	u32 val;
	int i, ret;

	ret = smsc75xx_eeprom_confirm_not_busy(dev);
	if (ret)
		return ret;

	for (i = 0; i < length; i++) {
		val = E2P_CMD_BUSY_ | E2P_CMD_READ_ | (offset & E2P_CMD_ADDR_);
		smsc75xx_write_reg(dev, E2P_CMD, val);

		ret = smsc75xx_wait_eeprom(dev);
		if (ret < 0)
			return ret;

		smsc75xx_read_reg(dev, E2P_DATA, &val);
		data[i] = val & 0xFF;
		offset++;
	}
	return 0;
}
#endif

/*
 * mii_nway_restart - restart NWay (autonegotiation) for this interface
 *
 * Returns 0 on success, negative on error.
 */
static int mii_nway_restart(struct ueth_data *dev)
{
	int bmcr;
	int r = -1;

	/* if autoneg is off, it's an error */
	bmcr = smsc75xx_mdio_read(dev, dev->phy_id, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		bmcr |= BMCR_ANRESTART;
		smsc75xx_mdio_write(dev, dev->phy_id, MII_BMCR, bmcr);
		r = 0;
	}
	return r;
}

static int smsc75xx_phy_initialize(struct ueth_data *dev)
{
	int bmcr,ret, timeout = 0;
	smsc75xx_mdio_write(dev, dev->phy_id, MII_BMCR, BMCR_RESET);	
	do {
		udelay(10 * 1000);
		bmcr = smsc75xx_mdio_read(dev, dev->phy_id, MII_BMCR);
		if (bmcr < 0) {
			debug("Error reading MII_BMCR: 0x%u\n", bmcr);
			return bmcr;
		}
		timeout++;
	} while ((bmcr & BMCR_RESET) && (timeout < 100));
	if(timeout >= 100){
		debug("smsc75xx_phy_initialize() BMCR_RESET timeout\n");
		return -1;
	}
	smsc75xx_mdio_write(dev, dev->phy_id, MII_ADVERTISE,
		ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP |
		ADVERTISE_PAUSE_ASYM);
	smsc75xx_mdio_write(dev, dev->phy_id, MII_CTRL1000, ADVERTISE_1000FULL);
	
	/* read and write to clear phy interrupt status */
	ret = smsc75xx_mdio_read(dev, dev->phy_id, PHY_INT_SRC);
	if (ret < 0) {
		debug( "Error reading PHY_INT_SRC\n");
		return ret;
	}
	smsc75xx_mdio_write(dev, dev->phy_id, PHY_INT_SRC, 0xffff);

	smsc75xx_mdio_write(dev, dev->phy_id, PHY_INT_MASK, PHY_INT_MASK_DEFAULT);
	mii_nway_restart(dev);

	debug("phy initialised successfully\n");

	return 0;
}

static int smsc75xx_init_mac_address(struct eth_device *eth,
		struct ueth_data *dev)
{
#ifdef __notdef
	/* try reading mac address from EEPROM */
	if (smsc75xx_read_eeprom(dev, EEPROM_MAC_OFFSET, ETH_ALEN,
			eth->enetaddr) == 0) {
		if (is_valid_ether_addr(eth->enetaddr)) {
			/* eeprom values are valid so use them */
			debug("MAC address read from EEPROM\n");
			return 0;
		}
	}
#endif
	/*
	 * No eeprom, or eeprom values are invalid. Generating a random MAC
	 * address is not safe. Just return an error.
	 */
	return -1;
}

static int smsc75xx_write_hwaddr(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct smsc75xx_private *priv = dev->dev_priv;
	u32 addr_lo = __get_unaligned_le32(&eth->enetaddr[0]);
	u32 addr_hi = __get_unaligned_le16(&eth->enetaddr[4]);
	int ret;

	/* set hardware address */
	debug("** %s()\n", __func__);
	ret = smsc75xx_write_reg(dev, RX_ADDRH, addr_hi);
	if(ret < 0) 
		return ret;
	ret = smsc75xx_write_reg(dev, RX_ADDRL, addr_lo);
	if(ret < 0)
		return ret;
		
	addr_hi |= ADDR_FILTX_FB_VALID;
	ret = smsc75xx_write_reg(dev, ADDR_FILTX, addr_hi);
	if(ret < 0)
		return ret;
	ret = smsc75xx_write_reg(dev, ADDR_FILTX + 4, addr_lo);
	if (ret < 0)
		return ret;

	debug("MAC %pM\n", eth->enetaddr);
	priv->have_hwaddr = 1;
	return 0;
}

/* Enable or disable Tx & Rx checksum offload engines */
static int smsc75xx_set_csums(struct ueth_data *dev,
		int use_tx_csum, int use_rx_csum)
{
	u32 read_buf;
	int ret = smsc75xx_read_reg(dev, RFE_CTL, &read_buf);
	if (ret < 0)
		return ret;
	debug("smsc75xx_set_csums() >> RFE_CTL = 0x%08x\n", read_buf);
	
#warning "left implement tx checksum"

#ifdef __notdef
	if (use_tx_csum)
		read_buf |= Tx_COE_EN_;
	else
		read_buf &= ~Tx_COE_EN_;
#endif
	if (use_rx_csum)
		read_buf |= RFE_CTL_TCPUDP_CKM | RFE_CTL_IP_CKM;
	else
		read_buf &= ~(RFE_CTL_TCPUDP_CKM | RFE_CTL_IP_CKM);
	
	ret = smsc75xx_write_reg(dev, RFE_CTL, read_buf);
	if(ret < 0){
		debug("RFE_CTL: checksum write failed\n");
		return -1;
	}
	return 0;
}

static void smsc75xx_set_multicast(struct ueth_data *dev)
{
	struct smsc75xx_private *priv = dev->dev_priv;

	/* No multicast in u-boot */
	priv->mac_cr &= ~(MAC_CR_PRMS_ | MAC_CR_MCPAS_ | MAC_CR_HPFILT_);
}

/*
 * smsc75xx callbacks
 */
static int smsc75xx_init(struct eth_device *eth, bd_t *bd)
{
	int ret;
	u32 read_buf;
	u32 buf;
	u32 burst_cap;
	int timeout;
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct smsc75xx_private *priv = (struct smsc75xx_private *)dev->dev_priv;
#define TIMEOUT_RESOLUTION 50	/* ms */
	int link_detected;

	debug("** %s()\n", __func__);
	dev->phy_id = smsc75xx_INTERNAL_PHY_ID; /* fixed phy id */

	ret = smsc75xx_wait_ready(dev);
	if (ret < 0) {
		debug("device not ready in smsc75xx_reset\n");
		return ret;
	}
	/* Start lite Reset */
	ret = smsc75xx_read_reg(dev, HW_CFG, &buf);
	if (ret < 0) {
		debug( "Failed to read HW_CFG: %d\n", ret);
		return ret;
	}

	buf |= HW_CFG_LRST;
	ret = smsc75xx_write_reg(dev, HW_CFG, buf);
	if (ret < 0) {
		debug("Failed to write HW_CFG: %d\n", ret);
		return ret;
	}

	timeout = 0;
	do {
		udelay(10 * 1000);
		ret = smsc75xx_read_reg(dev, HW_CFG, &buf);
		if (ret < 0) {
			debug("Failed to read HW_CFG: %d\n", ret);
			return ret;
		}
		timeout++;
	} while ((buf & HW_CFG_LRST) && (timeout < 100));

	if (timeout >= 100) {
		debug("timeout on completion of Lite Reset\n");
		return -1;
	}
	
	debug("Lite reset complete, resetting PHY (1)\n");
	
	/* PHY Reset */
	ret = smsc75xx_read_reg(dev, PMT_CTL, &buf);
	if (ret < 0) {
		debug("Failed to read PMT_CTL: %d\n", ret);
		return ret;
	}

	buf |= PMT_CTL_PHY_RST;

	ret = smsc75xx_write_reg(dev, PMT_CTL, buf);
	if (ret < 0) {
		debug("Failed to write PMT_CTL: %d\n", ret);
		return ret;
	}

	timeout = 0;
	do {
		udelay(10 * 1000);
		ret = smsc75xx_read_reg(dev, PMT_CTL, &buf);
		if (ret < 0) {
			debug("Failed to read PMT_CTL: %d\n", ret);
			return ret;
		}
		timeout++;
	} while ((buf & PMT_CTL_PHY_RST) && (timeout < 100));

	if (timeout >= 100) {
		debug("timeout waiting for PHY Reset\n");
		return -1;
	}
	debug("PHY reset complete\n");
	
	if (!priv->have_hwaddr && smsc75xx_init_mac_address(eth, dev) == 0)
		priv->have_hwaddr = 1;
	
	if (!priv->have_hwaddr) {
		puts("Error: smsc75xx: No MAC address set - set usbethaddr\n");
		return -1;
	}
	
	if (smsc75xx_write_hwaddr(eth) < 0)
		return -1;

	ret = smsc75xx_read_reg(dev, HW_CFG, &buf);
	if (ret < 0){
		debug("Read Value from HW_CFG: failed %d\n", ret);
		return ret;		
	}
	
	buf |= HW_CFG_BIR;
	ret = smsc75xx_write_reg(dev, HW_CFG, buf);
	if (ret < 0){
		debug("Write HW_CFG_BIR failed %d\n", ret);
		return ret;
	}

	ret = smsc75xx_read_reg(dev, HW_CFG, &buf);
	if (ret < 0){
		debug("Read Value from HW_CFG: failed %d\n", ret);
		return ret;
	}
	
	debug("Read Value from HW_CFG after writing "
			"HW_CFG_BIR: 0x%08x\n", buf);

#ifdef TURBO_MODE
	if (dev->pusb_dev->speed == USB_SPEED_HIGH) {
		burst_cap = DEFAULT_HS_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
		priv->rx_urb_size = DEFAULT_HS_BURST_CAP_SIZE;
	} else {
		burst_cap = DEFAULT_FS_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
		priv->rx_urb_size = DEFAULT_FS_BURST_CAP_SIZE;
	}
#else
	burst_cap = 0;
	priv->rx_urb_size = MAX_SINGLE_PACKET_SIZE;
#endif
	debug("rx_urb_size=%ld\n", (ulong)priv->rx_urb_size);

	ret = smsc75xx_write_reg(dev, BURST_CAP, burst_cap);
	if (ret < 0)
		return ret;

	ret = smsc75xx_read_reg(dev, BURST_CAP, &buf);
	if (ret < 0)
		return ret;
	debug("Read Value from BURST_CAP after writing: 0x%08x\n", buf);

	buf = DEFAULT_BULK_IN_DELAY;
	ret = smsc75xx_write_reg(dev, BULK_IN_DLY, buf);
	if (ret < 0)
		return ret;

	ret = smsc75xx_read_reg(dev, BULK_IN_DLY, &buf);
	if (ret < 0)
		return ret;
	debug("Read Value from BULK_IN_DLY after writing: "
			"0x%08x\n", read_buf);
#ifdef TURBO_MODE	
	ret = smsc75xx_read_reg(dev, HW_CFG, &buf);
	if (ret < 0) {
		debug("Failed to read HW_CFG: %d\n", ret);
		return ret;
	}
	debug("TURBO_MODE -> HW_CFG: 0x%08x\n", buf);
	buf |= (HW_CFG_MEF | HW_CFG_BCE);
	ret = smsc75xx_write_reg(dev, HW_CFG, buf);
	if (ret < 0) {
		debug("Failed to write HW_CFG: %d\n", ret);
		return ret;
	}
	ret = smsc75xx_read_reg(dev, HW_CFG, &buf);
	if (ret < 0) {
		debug("Failed to read HW_CFG: %d\n", ret);
		return ret;
	}
	debug("TURBO_MODE ->HW_CFG: 0x%08x\n", buf);
#endif

	/* set FIFO sizes */
	buf = (MAX_RX_FIFO_SIZE - 512) / 512;
	ret = smsc75xx_write_reg(dev, FCT_RX_FIFO_END, buf);
	if (ret < 0) {
		debug("Failed to write FCT_RX_FIFO_END: %d\n", ret);
		return ret;
	}

	debug("FCT_RX_FIFO_END set to 0x%08x\n", buf);

	buf = (MAX_TX_FIFO_SIZE - 512) / 512;
	ret = smsc75xx_write_reg(dev, FCT_TX_FIFO_END, buf);
	if (ret < 0) {
		debug("Failed to write FCT_TX_FIFO_END: %d\n", ret);
		return ret;
	}
	debug("FCT_TX_FIFO_END set to 0x%08x\n", buf);

	ret = smsc75xx_write_reg(dev, INT_STS, INT_STS_CLEAR_ALL);
	if (ret < 0) {
		debug("Failed to write INT_STS: %d\n", ret);
		return ret;
	}

	ret = smsc75xx_read_reg(dev, ID_REV, &buf);
	if (ret < 0) {
		debug("Failed to read ID_REV: %d\n", ret);
		return ret;
	}

	debug("ID_REV = 0x%08x\n", buf);

	ret = smsc75xx_read_reg(dev, E2P_CMD, &buf);
	if (ret < 0) {
		debug("Failed to read E2P_CMD: %d\n", ret);
		return ret;
	}
	
	/* only set default GPIO/LED settings if no EEPROM is detected */
	if (!(buf & E2P_CMD_LOADED)) {
		ret = smsc75xx_read_reg(dev, LED_GPIO_CFG, &buf);
		if (ret < 0) {
			debug("Failed to read LED_GPIO_CFG: %d\n", ret);
			return ret;
		}
		buf &= ~(LED_GPIO_CFG_LED2_FUN_SEL | LED_GPIO_CFG_LED10_FUN_SEL);
		buf |= LED_GPIO_CFG_LEDGPIO_EN | LED_GPIO_CFG_LED2_FUN_SEL;
		ret = smsc75xx_write_reg(dev, LED_GPIO_CFG, buf);
		if (ret < 0) {
			debug("Failed to write LED_GPIO_CFG: %d\n", ret);
			return ret;
		}
	}

	ret = smsc75xx_write_reg(dev, FLOW, 0);
	if (ret < 0) {
		debug("Failed to write FLOW: %d\n", ret);
		return ret;
	}

	ret = smsc75xx_write_reg(dev, FCT_FLOW, 0);
	if (ret < 0) {
		debug("Failed to write FCT_FLOW: %d\n", ret);
		return ret;
	}

	/* Don't need rfe_ctl_lock during initialisation */
	ret = smsc75xx_read_reg(dev, RFE_CTL, &buf);
	if (ret < 0) {
		debug("Failed to read RFE_CTL: %d\n", ret);
		return ret;
	}

	buf |= RFE_CTL_AB | RFE_CTL_DPF;

	ret = smsc75xx_write_reg(dev, RFE_CTL, buf);
	if (ret < 0) {
		debug("Failed to write RFE_CTL: %d\n", ret);
		return ret;
	}

	ret = smsc75xx_read_reg(dev, RFE_CTL, &buf);
	if (ret < 0) {
		debug("Failed to read RFE_CTL: %d\n", ret);
		return ret;
	}

	debug("RFE_CTL set to 0x%08x\n", buf);	

	/* Disable checksum offload engines */
	ret = smsc75xx_set_csums(dev, 0, 0);
	if (ret < 0) {
		debug("Failed to set csum offload: %d\n", ret);
		return ret;
	}
	
	smsc75xx_set_multicast(dev);

	if (smsc75xx_phy_initialize(dev) < 0){
		debug("Failed to initialize PHY: %d\n", ret);
		return -1;
	}

	ret = smsc75xx_read_reg(dev, INT_EP_CTL, &buf);
	if (ret < 0) {
		debug("Failed to read INT_EP_CTL: %d\n", ret);
		return ret;
	}
	
	/* enable PHY interrupts */
	buf |= INT_ENP_PHY_INT;
	ret = smsc75xx_write_reg(dev, INT_EP_CTL, buf);
	if (ret < 0) {
		debug("Failed to write INT_EP_CTL: %d\n", ret);
		return ret;
	}

	/* allow mac to detect speed and duplex from phy */
	ret = smsc75xx_read_reg(dev, MAC_CR, &buf);
	if (ret < 0) {
		debug("Failed to read MAC_CR: %d\n", ret);
		return ret;
	}

	buf |= (MAC_CR_ADD | MAC_CR_ASD);
	ret = smsc75xx_write_reg(dev, MAC_CR, buf);
	if (ret < 0) {
		debug("Failed to write MAC_CR: %d\n", ret);
		return ret;
	}

	ret = smsc75xx_read_reg(dev, MAC_TX, &buf);
	if (ret < 0) {
		debug("Failed to read MAC_TX: %d\n", ret);
		return ret;
	}

	buf |= MAC_TX_TXEN;

	ret = smsc75xx_write_reg(dev, MAC_TX, buf);
	if (ret < 0) {
		debug("Failed to write MAC_TX: %d\n", ret);
		return ret;
	}

	debug("MAC_TX set to 0x%08x\n", buf);

	ret = smsc75xx_read_reg(dev, FCT_TX_CTL, &buf);
	if (ret < 0) {
		debug("Failed to read FCT_TX_CTL: %d\n", ret);
		return ret;
	}

	buf |= FCT_TX_CTL_EN;
	ret = smsc75xx_write_reg(dev, FCT_TX_CTL, buf);
	if (ret < 0) {
		debug("Failed to write FCT_TX_CTL: %d\n", ret);
		return ret;
	}

	debug("FCT_TX_CTL set to 0x%08x\n", buf);
#warning "smsc75xx_set_rx_max_frame_length() not defined"

#ifdef __notdef
	ret = smsc75xx_set_rx_max_frame_length(dev, dev->net->mtu + ETH_HLEN);
	if (ret < 0) {
		debug(dev->net, "Failed to set max rx frame length\n");
		return ret;
	}
#endif
	ret = smsc75xx_read_reg(dev, MAC_RX, &buf);
	if (ret < 0) {
		debug("Failed to read MAC_RX: %d\n", ret);
		return ret;
	}

	buf |= MAC_RX_RXEN;

	ret = smsc75xx_write_reg(dev, MAC_RX, buf);
	if (ret < 0) {
		debug("Failed to write MAC_RX: %d\n", ret);
		return ret;
	}

	debug("MAC_RX set to 0x%08x\n", buf);

	ret = smsc75xx_read_reg(dev, FCT_RX_CTL, &buf);
	if (ret < 0) {
		debug("Failed to read FCT_RX_CTL: %d\n", ret);
		return ret;
	}

	buf |= FCT_RX_CTL_EN;

	ret = smsc75xx_write_reg(dev, FCT_RX_CTL, buf);
	if (ret < 0) {
		debug("Failed to write FCT_RX_CTL: %d\n", ret);
		return ret;
	}

	debug("FCT_RX_CTL set to 0x%08x\n", buf);

	timeout = 0;
	do {
		link_detected = smsc75xx_mdio_read(dev, dev->phy_id, MII_BMSR)
			& BMSR_LSTATUS;
		if (!link_detected) {
			if (timeout == 0)
				printf("Waiting for Ethernet connection... ");
			udelay(TIMEOUT_RESOLUTION * 1000);
			timeout += TIMEOUT_RESOLUTION;
		}
	} while (!link_detected && timeout < PHY_CONNECT_TIMEOUT);
	if (link_detected) {
		if (timeout != 0)
			printf("done.\n");
	} else {
		printf("unable to connect.\n");
		return -1;
	}
	return 0;
}

static int smsc75xx_send(struct eth_device *eth, void* packet, int length)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	int err;
	int actual_len;
	u32 tx_cmd_a;
	u32 tx_cmd_b;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, msg,
				 PKTSIZE + sizeof(tx_cmd_a) + sizeof(tx_cmd_b));

	debug("** %s(), len %d, buf %#x\n", __func__, length, (int)msg);
	if (length > PKTSIZE)
		return -1;

	// tx_cmd_a = (u32)length | TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_;
	tx_cmd_a = (u32)(length & TX_CMD_A_LEN) | TX_CMD_A_FCS;
	tx_cmd_b = 0;
	// tx_cmd_b = (u32)length;
	cpu_to_le32s(&tx_cmd_a);
	cpu_to_le32s(&tx_cmd_b);

	/* prepend cmd_a and cmd_b */
	memcpy(msg, &tx_cmd_a, sizeof(tx_cmd_a));
	memcpy(msg + sizeof(tx_cmd_a), &tx_cmd_b, sizeof(tx_cmd_b));
	memcpy(msg + sizeof(tx_cmd_a) + sizeof(tx_cmd_b), (void *)packet,
	       length);
	err = usb_bulk_msg(dev->pusb_dev,
				usb_sndbulkpipe(dev->pusb_dev, dev->ep_out),
				(void *)msg,
				length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b),
				&actual_len,
				USB_BULK_SEND_TIMEOUT);
	debug("Tx: len = %u, actual = %u, err = %d\n",
	      length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b),
	      actual_len, err);
	return err;
}

static int smsc75xx_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	DEFINE_CACHE_ALIGN_BUFFER(unsigned char, recv_buf, AX_RX_URB_SIZE);
	unsigned char *buf_ptr;
	int err;
	int actual_len;
	u32 packet_len;
	int off = 0;

	debug("** %s()\n", __func__);
	err = usb_bulk_msg(dev->pusb_dev,
				usb_rcvbulkpipe(dev->pusb_dev, dev->ep_in),
				(void *)recv_buf,
				AX_RX_URB_SIZE,
				&actual_len,
				USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", AX_RX_URB_SIZE,
	      actual_len, err);
	if (err != 0) {
		debug("Rx: failed to receive\n");
		return -1;
	}
	if (actual_len > AX_RX_URB_SIZE) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -1;
	}

	buf_ptr = recv_buf;
	while (actual_len > 0) {
		u32 rx_cmd_a, rx_cmd_b, align_count, size;
		memcpy(&rx_cmd_a, buf_ptr, sizeof(rx_cmd_a));
		le32_to_cpus(&rx_cmd_a);
		memcpy(&rx_cmd_b, buf_ptr+4, sizeof(rx_cmd_b));
		off = sizeof(rx_cmd_a) + sizeof(rx_cmd_b) + RXW_PADDING;
		packet_len = size = (rx_cmd_a & RX_CMD_A_LEN) - RXW_PADDING;
		align_count = (4 - ((size + RXW_PADDING) % 4)) % 4;
		debug("Rx: packet length: %d  align: %d\n", packet_len, align_count);
		net_process_received_packet(buf_ptr + off, packet_len);
		/* Adjust for next iteration */
		actual_len -= off + packet_len;
		buf_ptr += off + packet_len;
		actual_len -= align_count;
		buf_ptr += align_count;
	}
	return err;
}

#ifdef __notdef
static int smsc75xx_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	DEFINE_CACHE_ALIGN_BUFFER(unsigned char, recv_buf, AX_RX_URB_SIZE);
	unsigned char *buf_ptr;
	int err;
	int actual_len;
	u32 packet_len;
	int cur_buf_align;

	debug("** %s()\n", __func__);
	err = usb_bulk_msg(dev->pusb_dev,
				usb_rcvbulkpipe(dev->pusb_dev, dev->ep_in),
				(void *)recv_buf,
				AX_RX_URB_SIZE,
				&actual_len,
				USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", AX_RX_URB_SIZE,
	      actual_len, err);
	if (err != 0) {
		debug("Rx: failed to receive\n");
		return -1;
	}
	if (actual_len > AX_RX_URB_SIZE) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -1;
	}

	buf_ptr = recv_buf;
	while (actual_len > 0) {
		/*
		 * 1st 4 bytes contain the length of the actual data plus error
		 * info. Extract data length.
		 */
		if (actual_len < sizeof(packet_len)) {
			debug("Rx: incomplete packet length\n");
			return -1;
		}
		memcpy(&packet_len, buf_ptr, sizeof(packet_len));
		le32_to_cpus(&packet_len);
		if (packet_len & RX_STS_ES_) {
			debug("Rx: Error header=%#x", packet_len);
			return -1;
		}
		packet_len = ((packet_len & RX_STS_FL_) >> 16);

		if (packet_len > actual_len - sizeof(packet_len)) {
			debug("Rx: too large packet: %d\n", packet_len);
			return -1;
		}

		/* Notify net stack */
		NetReceive(buf_ptr + sizeof(packet_len), packet_len - 4);

		/* Adjust for next iteration */
		actual_len -= sizeof(packet_len) + packet_len;
		buf_ptr += sizeof(packet_len) + packet_len;
		cur_buf_align = (int)buf_ptr - (int)recv_buf;

		if (cur_buf_align & 0x03) {
			int align = 4 - (cur_buf_align & 0x03);

			actual_len -= align;
			buf_ptr += align;
		}
	}
	return err;
}
#endif

static void smsc75xx_halt(struct eth_device *eth)
{
	debug("** %s()\n", __func__);
}

/*
 * SMSC probing functions
 */
void smsc75xx_eth_before_probe(void)
{
	curr_eth_dev = 0;
}

struct smsc75xx_dongle {
	unsigned short vendor;
	unsigned short product;
};

static const struct smsc75xx_dongle smsc75xx_dongles[] = {
	{ 0x0424, 0x7500 }, /* SMSC7500 USB Ethernet Device */
	{ 0x0000, 0x0000 }	/* END - Do not remove */
};

/* Probe to see if a new device is actually an SMSC device */
int smsc75xx_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int i;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	for (i = 0; smsc75xx_dongles[i].vendor != 0; i++) {
		if (dev->descriptor.idVendor == smsc75xx_dongles[i].vendor &&
		    dev->descriptor.idProduct == smsc75xx_dongles[i].product)
			/* Found a supported dongle */
			break;
	}
	if (smsc75xx_dongles[i].vendor == 0)
		return 0;

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected\n");
	memset(ss, '\0', sizeof(struct ueth_data));

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and int.
	 * We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			if (iface->ep_desc[i].bEndpointAddress & USB_DIR_IN)
				ss->ep_in =
					iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
			else
				ss->ep_out =
					iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		    USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
		  ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (usb_set_interface(dev, iface_desc->bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out || !ss->ep_int) {
		debug("Problems with device\n");
		return 0;
	}
	dev->privptr = (void *)ss;

	/* alloc driver private */
	ss->dev_priv = calloc(1, sizeof(struct smsc75xx_private));
	if (!ss->dev_priv)
		return 0;

	return 1;
}

int smsc75xx_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
				struct eth_device *eth)
{
	debug("** %s()\n", __func__);
	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}
	sprintf(eth->name, "%s%d", smsc75xx_BASE_NAME, curr_eth_dev++);
	eth->init = smsc75xx_init;
	eth->send = smsc75xx_send;
	eth->recv = smsc75xx_recv;
	eth->halt = smsc75xx_halt;
	eth->write_hwaddr = smsc75xx_write_hwaddr;
	eth->priv = ss;
	return 1;
}
