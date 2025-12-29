// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Intel 82557 / 82558 / 82559 / 82550 / 82551 / 82562 Ethernet Network Adapters

TODO:
- Throws PXE-E05 error when checksumming the ROM in pcipc, goes out of bounds.
  Documentation claims that 82559 and beyond requires PCI rev 2.2, pcipc is 2.1 ...

**************************************************************************************************/

#include "emu.h"
#include "i82559_pci.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


//DEFINE_DEVICE_TYPE(I82557_PCI, i82557_pci_device,   "i82557_pci",   "Intel 82557-based PCI Ethernet Adapter card")
//DEFINE_DEVICE_TYPE(I82558_PCI, i82558_pci_device,   "i82558_pci",   "Intel 82558-based PCI Ethernet Adapter card")
DEFINE_DEVICE_TYPE(I82559_PCI, i82559_pci_device,   "i82559_pci",   "Intel 82559-based PCI Ethernet Adapter card")
//DEFINE_DEVICE_TYPE(I82550_PCI, i82550_pci_device,   "i82550_pci",   "Intel 82550-based PCI Ethernet Adapter card")
//DEFINE_DEVICE_TYPE(I82551_PCI, i82551_pci_device,   "i82551_pci",   "Intel 82551-based PCI Ethernet Adapter card")
//DEFINE_DEVICE_TYPE(I82562_PCI, i82562_pci_device,   "i82562_pci",   "Intel 82562-based PCI Ethernet Adapter card")



i82559_pci_device::i82559_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_pxe_rom(*this, "pxe_rom")
{
	// TODO: confirm device, revision and subdevice IDs
	// NOTE: revision 0x00 will cause error above, 0x0c (for 82550) will outright lockup MAME
	set_ids(0x80861229, 0x0c, 0x020000, 0x80861229);
}

i82559_pci_device::i82559_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82559_pci_device(mconfig, I82559_PCI, tag, owner, clock)
{
}

ROM_START( i82559 )
	ROM_REGION32_LE( 0x10000, "pxe_rom", ROMREGION_ERASEFF )
	// Flash ROM on Intel Pro/100 S Network Adapter.
	// Network chip is Intel 82550EY with 25MHz crystal attached.
	ROM_SYSTEM_BIOS( 0, "pro100s", "82550EY Pro/S card" )
	ROMX_LOAD( "at49bv512.u4", 0x0000, 0x10000, CRC(4e5b119c) SHA1(6501239d2f6187cbd7318a42702cd5a0bc8d17d8), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *i82559_pci_device::device_rom_region() const
{
	return ROM_NAME(i82559);
}

void i82559_pci_device::device_add_mconfig(machine_config &config)
{

}

void i82559_pci_device::device_start()
{
	pci_card_device::device_start();

//	add_map( 4*1024, M_MEM, FUNC(i82559_pci_device::csr_memory_map));
//	add_map( 32,     M_IO, FUNC(i82559_pci_device::csr_io_map));
//	add_map( 1*1024*1024, M_MEM, FUNC(i82559_pci_device::flash_map));
	add_rom((u8 *)m_pxe_rom->base(), 0x10000);
	expansion_rom_base = 0xc8000;

	// INTA#
	intr_pin = 1;
}

void i82559_pci_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: to be checked
	command = 0x0000;
	status = 0x0000;

	remap_cb();
}

void i82559_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}
