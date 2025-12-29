// license:BSD-3-Clause
// copyright-holders:
#ifndef MAME_BUS_JAGUAR_ROM_H
#define MAME_BUS_JAGUAR_ROM_H

#pragma once

#include "slot.h"


// ======================> jaguar_rom_device

class jaguar_rom_device : public device_t,
							public device_jaguar_cart_interface
{
public:
	// construction/destruction
	jaguar_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// load/unload
	virtual std::error_condition load() override;
	virtual void unload() override;

	// read/write
	virtual u16 rom_r(offs_t offset) override;
//	virtual u8 nvram_r(offs_t offset) override;
//	virtual void nvram_w(offs_t offset, u8 data) override;

protected:
	jaguar_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	const u16 *m_rom_base;
//	u8 *m_nvram_base;
	u32 m_rom_size;
//	u32 m_nvram_size;
};


DECLARE_DEVICE_TYPE(JAGUAR_ROM_STD,   jaguar_rom_device)

#endif // MAME_BUS_JAGUAR_ROM_H
