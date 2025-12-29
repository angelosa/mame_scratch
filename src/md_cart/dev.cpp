// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Sega GEMS dev cart 171-5862A

TODO:
- preliminary, cart has optional battery (for the SRAM) and a bunch of dipswitches dictating the
configuration. The defaults of which should really be from SW list features
(something that MAME can't do afaik) ...

**************************************************************************************************/

#include "emu.h"
#include "dev.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_DEV_GEMS, megadrive_dev_gems_device, "megadrive_dev_gems", "Megadrive Sega GEMS dev cart")

megadrive_dev_gems_device::megadrive_dev_gems_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_DEV_GEMS, tag, owner, clock)
{
}

void megadrive_dev_gems_device::device_start()
{
	megadrive_rom_device::device_start();
	m_sram = std::make_unique<u16[]>(0x4000);
}

void megadrive_dev_gems_device::cart_map(address_map &map)
{
	// ROM is 0x40000, but only first 32KB is valid
	map(0x00'0000, 0x00'7fff).bankr(m_rom);
	map(0x01'0000, 0x01'7fff).lrw16(
		NAME([this] (offs_t offset) { return m_sram[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_sram[offset]); })
	);
}

