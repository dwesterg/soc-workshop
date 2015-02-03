/* __KERNEL_DRIVER_COPYRIGHT__ */

#ifndef HW_PCIE_DMA_H
#define HW_PCIE_DMA_H


// DMA parameters to tweak
static const unsigned int ACL_PCIE_DMA_MAX_PINNED_MEM_SIZE = 1024*1024;
static const unsigned int ACL_PCIE_DMA_MAX_PINNED_MEM = 64; // x PINNED_MEM_SIZE above
static const unsigned int ACL_PCIE_DMA_MAX_ATT_PER_DESCRIPTOR = 128;

// Constants matched to the HW
static const unsigned int ACL_PCIE_DMA_MAX_DONE_COUNT = (1 << 16);
static const unsigned int ACL_PCIE_DMA_MAX_ATT_PAGE_SIZE = 4*1024;
static const unsigned int ACL_PCIE_DMA_MAX_ATT_SIZE = 256;
static const unsigned int ACL_PCIE_DMA_MAX_DESCRIPTORS = 128;

static const unsigned int ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK = 4*1024-1; // (ACL_PCIE_DMA_MAX_ATT_PAGE_SIZE-1);

#ifdef LINUX
#  define cl_ulong unsigned long
#endif

struct DMA_DESCRIPTOR {
   unsigned int read_address;
   unsigned int write_address;
   unsigned int bytes;
   unsigned int burst;
   unsigned int stride;
   unsigned int read_address_hi;
   unsigned int write_address_hi;
   unsigned int control;
};

struct DESCRIPTOR_UPDATE_DATA {
   unsigned int bytes;
   unsigned int att_entries;
   cl_ulong start;
};

#endif // HW_PCIE_DMA_H
