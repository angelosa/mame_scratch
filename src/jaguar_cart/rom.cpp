// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  jaguar_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(JAGUAR_ROM_STD,     jaguar_rom_device,    "jaguar_rom",    "Atari Jaguar Standard Cart")


jaguar_rom_device::jaguar_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock), device_jaguar_cart_interface(mconfig, *this)
	, m_rom_base(nullptr)
//	, m_nvram_base(nullptr)
	, m_rom_size(0)
//	, m_nvram_size(0)
{
}

jaguar_rom_device::jaguar_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaguar_rom_device(mconfig, JAGUAR_ROM_STD, tag, owner, clock)
{
}


void jaguar_rom_device::device_start()
{
}

void jaguar_rom_device::device_add_mconfig(machine_config &config)
{
}

std::error_condition jaguar_rom_device::load()
{
	memory_region *const romregion = memregion("^rom");
	m_rom_base = reinterpret_cast<const u16 *>(romregion->base());
	m_rom_size = romregion->bytes() / 2;

//  if (m_rom_size > 0x40'0000)
//      return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be no larger than 4M)");

//	memory_region *const nvramregion = memregion("^nvram");
//	if (nvramregion)
//	{
//		m_nvram_base = reinterpret_cast<u8 *>(nvramregion->base());
//		m_nvram_size = nvramregion->bytes();
//
//		if (m_nvram_size & (m_nvram_size - 1))
//			return image_error::BADSOFTWARE;
//
//		save_pointer(NAME(m_nvram_base), m_nvram_size);
//		battery_load(m_nvram_base, m_nvram_size, 0xff);
//	}

	return std::error_condition();
}


void jaguar_rom_device::unload()
{
//	if (m_nvram_base)
//		battery_save(m_nvram_base, m_nvram_size);
}


/*-------------------------------------------------
 read/write
 -------------------------------------------------*/

u16 jaguar_rom_device::rom_r(offs_t offset)
{
	if (offset < m_rom_size)
		return m_rom_base[offset];
	else
		return 0xffff;
}

//u8 jaguar_rom_device::nvram_r(offs_t offset)
//{
//	if (m_nvram_base)
//		return m_nvram_base[offset & (m_nvram_size - 1)];
//	else
//		return 0xff;
//}
//
//void jaguar_rom_device::nvram_w(offs_t offset, u8 data)
//{
//	if (m_nvram_base)
//		m_nvram_base[offset & (m_nvram_size - 1)] = data;
//}
