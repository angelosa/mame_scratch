// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

NCR / Symbios Logic / LSI Logic 53C810

NOTE:
- being pre-class code PCI this BIOS is incompatible with pcipc and later, expansion ROM gets
  disabled before header checks. Otherwise:
\- entrada just ignores it at current time (may be down to "auto config error");
\- a486ap4 and sy029c will load it and throw a "malfunctioning adapter";
\- ga586ip will load its own BIOS override (NCRPCI-3.06.00), hangs there while checking the
   adapter;

**************************************************************************************************/

#include "emu.h"
#include "ncr53c810_pci.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(NCR53C810_PCI, ncr53c810_pci_device,   "ncr53c810_pci",   "NCR/Symbios Logic 53C810 SCSI controller")


ncr53c810_pci_device::ncr53c810_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_bios(*this, "bios")
{
}

ncr53c810_pci_device::ncr53c810_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c810_pci_device(mconfig, NCR53C810_PCI, tag, owner, clock)
{
	set_ids(0x10000001, 0x00, 0x000000, 0x10000001);
}

ROM_START( ncr53c810_pci )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	// TODO: this just seems a common BIOS for all the NCR53C8XX variants
	// this card is romless ...
	ROM_SYSTEM_BIOS( 0, "32kb", "SDMS V4.12.00 Symbios SYM20810 32KB BIOS" )
	ROMX_LOAD( "pci_32.rom", 0x0000, 0x8000, CRC(7d05e2b9) SHA1(cf4f5cefe93633e06af99b4cec713b26cbbf4f88), ROM_BIOS(0) )
	// TODO: whatever is this fails to load
	ROM_SYSTEM_BIOS( 1, "64kb", "Symbios SYM20810 64KB BIOS" )
	ROMX_LOAD( "8xx_64.rom", 0x0000, 0xf000, CRC(1fbef043) SHA1(e2bba786f59adab4218c41bb52d8cf534af09a52), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *ncr53c810_pci_device::device_rom_region() const
{
	return ROM_NAME(ncr53c810_pci);
}


void ncr53c810_pci_device::device_add_mconfig(machine_config &config)
{

}

void ncr53c810_pci_device::device_start()
{
	pci_card_device::device_start();

	add_map( 256,  M_IO,  FUNC(ncr53c810_pci_device::io_map));
	// TODO: verify me
	add_map( 4096, M_MEM, FUNC(ncr53c810_pci_device::mem_map));

	add_rom((u8 *)m_bios->base(), 0x10000);
	expansion_rom_base = 0xc8000;

	intr_pin = 0;
}

void ncr53c810_pci_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x47;
	status = 0x00200;

	remap_cb();
}



void ncr53c810_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void ncr53c810_pci_device::io_map(address_map &map)
{

}

void ncr53c810_pci_device::mem_map(address_map &map)
{

}
