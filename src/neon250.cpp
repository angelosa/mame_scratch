// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

VideoLogic PowerVR Neon 250

**************************************************************************************************/

#include "emu.h"
#include "neon250.h"

#define LOG_WARN      (1U << 1)
#define LOG_TODO      (1U << 2) // log unimplemented registers

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_TODO)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGTODO(...)            LOGMASKED(LOG_TODO, __VA_ARGS__)


DEFINE_DEVICE_TYPE(NEON250,   neon250_device,   "neon250",   "VideoLogic PowerVR Neon 250")

neon250_device::neon250_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	// PowerVR Neon 250 AGP (NEC branded)
	set_ids_agp(0x10330067, 0x00, 0x10100120);
}

neon250_device::neon250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: neon250_device(mconfig, NEON250, tag, owner, clock)
{
}

ROM_START( neon250 )
	ROM_REGION32_LE( 0x20000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "neon250", "NEC Neon 250" )
	ROMX_LOAD( "n0020331.bin", 0x000000, 0x20000, CRC(e76008f5) SHA1(1ddda1494d5b32148e2c9e7a6557f00ee9cffea6), ROM_BIOS(0) )
//	ROMX_LOAD( "ppvr201i.bin", 0x000000, 0x10000, CRC(b6e763c9) SHA1(1e1fecbb663dde5295a9db98dde70a4e1bd8338d), ROM_BIOS(0) )
//	ROMX_LOAD( "b1002816.bin", 0x000000, 0x10000, CRC(d3cf607a) SHA1(8fea7c3e678bc585ed5729d7f1a4b203bae8d15d), ROM_BIOS(0) )
	// from VGA Legacy MkIII, bad
//	ROMX_LOAD( "neon250agp.vbi", 0x000000, 0x009200, CRC(41a46c28) SHA1(f99482b906436d9b1db25e68de5a2bc96a192128) BAD_DUMP, ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *neon250_device::device_rom_region() const
{
	return ROM_NAME(neon250);
}

void neon250_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_svga, FUNC(vga_device::screen_update));

	// TODO: change me
	VGA(config, m_svga, 0);
	m_svga->set_screen("screen");
	m_svga->set_vram_size(32*1024*1024);
}

void neon250_device::null_map(address_map &map)
{
	
}


void neon250_device::device_start()
{
	pci_device::device_start();

//	add_map(  1*1024*1024, M_MEM, FUNC(neon250_device::mmio_map));
//	add_map( 32*1024*1024, M_MEM, FUNC(neon250_device::vram_aperture_map));
	add_map(  1*1024*1024, M_MEM, FUNC(neon250_device::null_map));
	add_map(  1*1024*1024, M_MEM, FUNC(neon250_device::null_map));
	add_map(  1*1024*1024, M_MEM, FUNC(neon250_device::null_map));
	add_map(  1*1024*1024, M_MEM, FUNC(neon250_device::null_map));
	add_map(  1*1024*1024, M_MEM, FUNC(neon250_device::null_map));

	add_rom((u8 *)m_vga_rom->base(), 0x10000);
	expansion_rom_base = 0xc0000;

	// INTA#
	// TODO: unconfirmed
	intr_pin = 1;
}

void neon250_device::device_reset()
{
	pci_device::device_reset();

	// TODO: to be checked
	command = 0x0000;
	status = 0x0000;

	remap_cb();
}

void neon250_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

u32 neon250_device::unmap_log_r(offs_t offset, u32 mem_mask)
{
	LOGTODO("MMIO Unemulated [%08x] & %08x R\n", offset * 4, mem_mask);
	return 0;
}

void neon250_device::unmap_log_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGTODO("MMIO Unemulated [%08x] %08x & %08x W\n", offset * 4, data, mem_mask);
}


// TODO: this should really be a subclass of VGA
void neon250_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(neon250_device::vram_r), FUNC(neon250_device::vram_w));
}

void neon250_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_svga, FUNC(vga_device::io_map));
}

uint8_t neon250_device::vram_r(offs_t offset)
{
	return downcast<vga_device *>(m_svga.target())->mem_r(offset);
}

void neon250_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<vga_device *>(m_svga.target())->mem_w(offset, data);
}

void neon250_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	printf("%02x\n", command);
	if (command & 7)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(neon250_device::vram_r)), write8sm_delegate(*this, FUNC(neon250_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &neon250_device::legacy_io_map);
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}
