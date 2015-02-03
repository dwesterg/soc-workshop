/* __KERNEL_DRIVER_COPYRIGHT__ */

/* Constants for access VSEC section of hard PCIe block in devices
 * that support Configuration via Protocol (CvP). */

/* All data in this file is taken from qquartus/pgm/h/cvp_drv.h and
 * quartus/pgm/cvp_dr/cvp_drv.c */


/* Configuration data is written to this offset, BAR 0 */
#define OFFSET_NONE          0x0

/* offset of VSEC within configuration space */
#define OFFSET_VSEC          0x200

/* offset of sections of VSEC, relative to OFFSET_VSEC */
#define OFFSET_ALTERA_MARKER 0x08
#define OFFSET_CVP_STATUS    0x1E
#define OFFSET_CVP_MODE_CTRL 0x20
#define OFFSET_CVP_NUMCLKS   0X21
#define OFFSET_CVP_DATA      0x28
#define OFFSET_CVP_PROG_CTRL 0x2C
#define OFFSET_UNC_IE_STATUS 0x34
#define OFFSET_UNC_IE_MASK   0x38 
#define OFFSET_C_IE_STATUS   0x3C 
#define OFFSET_C_IE_MASK     0x40 

/* bit masks to extract specific bit value from 8-bit VSEC value. */
#define MASK_DATA_ENCRYPTED    0x01 // bit 0 of CVP_STATUS
#define MASK_DATA_COMPRESSED   0x02 // bit 1 of CVP_STATUS
#define MASK_CVP_CONFIG_READY  0x04 // bit 2 of CVP_STATUS
#define MASK_CVP_CONFIG_ERROR  0x08 // bit 3 of CVP_STATUS
#define MASK_CVP_EN            0x10 // bit 4 of CVP_STATUS
#define MASK_USER_MODE         0x20 // bit 5 of CVP_STATUS
#define MASK_PLD_CLK_IN_USE    0x01 // bit 8 of CVP_STATUS (ie, bit 0 of byte offset CVP_STATUS+1)
#define MASK_CVP_MODE          0x01 // bit 0 of CVP_MODE_CTRL
#define MASK_HIP_CLK_SEL       0X02 // bit 1 of CVP_MODE_CTRL
#define MASK_CVP_CONFIG        0x01 // bit 0 of CVP_PROG_CTRL
#define MASK_START_XFER        0x02 // bit 1 of CVP_PROG_CTRL
#define MASK_CVP_CFG_ERR_LATCH 0x20 // bit 5 of UNC_IE_STATUS

/* VSEC bit definitions. 
 * The driver will map VSEC_BITs to offset and mask above. */
enum VSEC_BIT { 
  DATA_ENCRYPTED = 0,
  DATA_COMPRESSED,
  CVP_CONFIG_READY,
  CVP_CONFIG_ERROR,
  CVP_EN,
  USER_MODE,
  PLD_CLK_IN_USE,
  CVP_MODE,
  HIP_CLK_SEL,
  CVP_CONFIG,
  START_XFER,
  CVP_CFG_ERR_LATCH
};
