// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

3Dlabs Permedia

TODO:
- placeholder for roms;
- black screen when mounted;

**************************************************************************************************/

#include "emu.h"
#include "permedia.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PERMEDIA_PCI, permedia_device,   "permedia",   "3Dlabs Permedia")

permedia_device::permedia_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
	// TODO: can change class code
	set_ids(0x104c3d04, 0x01, 0x030100, 0x104c3d04);

}

permedia_device::permedia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: permedia_device(mconfig, PERMEDIA_PCI, tag, owner, clock)

{
}

ROM_START( permedia )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASE00 )
	ROM_SYSTEM_BIOS( 0, "c3000",   "PowerColor C3000 Rev. 1.0" )
	ROMX_LOAD( "permedia-c3000.vbi", 0x0000, 0x8000, CRC(97f80ddf) SHA1(f370aa17239a340d415e1ca20cdd2c982f365f04), ROM_BIOS(0) )

	// dataexpert pm3040 pci (permedia 3000).bin (bad, from The Retro Web)
//	ROMX_LOAD( "dataexpert pm3040 pci.bin",  0x00000, 0x06600, BAD_DUMP CRC(763f016b) SHA1(e25b58cd532e5872be303439168330bd5e2641b6), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *permedia_device::device_rom_region() const
{
	return ROM_NAME(permedia);
}


void permedia_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	// TODO: TVP3026 + TVP4010 cores
	// former is same RAMDAC as Matrox cards
	VGA(config, m_vga);
	m_vga->set_screen("screen");
	// 8MB max SGRAM
	m_vga->set_vram_size(8*1024*1024);
}

void permedia_device::device_start()
{
	pci_card_device::device_start();

	add_map(   128*1024, M_MEM, FUNC(permedia_device::control_map));
	add_map(8*1024*1024, M_MEM, FUNC(permedia_device::aperture1_map));
	add_map(8*1024*1024, M_MEM, FUNC(permedia_device::aperture2_map));
	// TODO: following two have extra bits that can disable BAR mapping
	add_map(         16, M_IO,  FUNC(permedia_device::aux_map));
	add_map(   256*1024, M_MEM, FUNC(permedia_device::delta_map));

	add_rom((u8 *)m_bios->base(), 0x8000 );
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
}

void permedia_device::device_reset()
{
	pci_card_device::device_reset();

	// bus master capable
	command = 0x0000;
	command_mask = 0x0027;
	// Fast back-to-back, medium DEVSEL#
	status = 0x0280;

	// TODO: both can be 0x00 / 0x40 / 0x80 / 0xc0 (needs lspci to tell)
	minimum_grant = 0x00;
	maximum_latency = 0x00;


	remap_cb();
}

void permedia_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);

//  map(0xf8, 0xf8) CFGIndirectAddress
//  map(0xfc, 0xfc) CFGIndirectData
}

void permedia_device::control_map(address_map &map)
{

}

void permedia_device::aperture1_map(address_map &map)
{

}

void permedia_device::aperture2_map(address_map &map)
{

}

void permedia_device::aux_map(address_map &map)
{
}

// GLINT compatible map
void permedia_device::delta_map(address_map &map)
{

}

void permedia_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(permedia_device::vram_r), FUNC(permedia_device::vram_w));
}

void permedia_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(vga_device::io_map));
}

uint8_t permedia_device::vram_r(offs_t offset)
{
	return downcast<vga_device *>(m_vga.target())->mem_r(offset);
}

void permedia_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<vga_device *>(m_vga.target())->mem_w(offset, data);
}

void permedia_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(permedia_device::vram_r)), write8sm_delegate(*this, FUNC(permedia_device::vram_w)));

	if (BIT(command, 0))
		io_space->install_device(0x0000, 0xffff, *this, &permedia_device::legacy_io_map);
}

