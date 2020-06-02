#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int map_NO = is_mmio(addr);
  if(map_NO != -1) {
  	return mmio_read(addr, len, map_NO);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int map_NO = is_mmio(addr);
  if(map_NO != -1) {
  	mmio_write(addr, len, data, map_NO);
  	return;
  }
  memcpy(guest_to_host(addr), &data, len);
}

bool data_cross_page(vaddr_t addr, int len) {
  if (((addr & 0xfff) + len) >= 0x1000)
    return true;
  return false;
}

paddr_t page_translate(vaddr_t addr, bool is_write) {
  if (cpu.cr0.paging) {
    paddr_t pdbase = (cpu.cr3.page_directory_base << 12) | (((addr >> 22) & 0x3ff) << 2);
    PDE pde;
    pde.val = paddr_read(pdbase, sizeof(PDE));
    assert(pde.present);
    paddr_t ptbase = (pde.page_frame << 12) | (((addr >> 12) & 0x3ff) << 2);
    PTE pte;
    pte.val = paddr_read(ptbase, sizeof(PTE));
    assert(pte.present);
    paddr_t paddr = (pte.page_frame << 12) | (addr & 0xfff);
    pde.accessed = 1;
    paddr_write(pdbase, sizeof(PDE), pde.val);
    pte.accessed = 1;
    if (is_write) {
      pte.dirty = 1;
    }
    paddr_write(ptbase, sizeof(PTE), pte.val);
    return paddr;
  }
  else {
    return addr;
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (data_cross_page(addr, len)) {
    //assert(0);
    int low_len = 0x1000 - (addr & 0xfff);
    vaddr_t high_vaddr = addr + low_len;
    paddr_t low_paddr = page_translate(addr, false);
    uint32_t data = 0;
    data = paddr_read(low_paddr, low_len);
    paddr_t high_paddr = page_translate(high_vaddr, false);
    data = (paddr_read(high_paddr, len - low_len) << low_len) | data;
    return data;
  }
  else {
    paddr_t paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  }
  //return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (data_cross_page(addr, len)) {
    //assert(0);
    int low_len = 0x1000 - (addr & 0xfff);
    int high_len = len - low_len;
    vaddr_t high_vaddr = addr + low_len;
    paddr_t low_paddr = page_translate(addr, true);
    paddr_write(low_paddr, low_len, (data << high_len) >> high_len);
    paddr_t high_paddr = page_translate(high_vaddr, true);
    paddr_write(high_paddr, high_len, data >> low_len);
  }
  else {
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
  //paddr_write(addr, len, data);
}
