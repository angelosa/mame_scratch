// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_DEV_H
#define MAME_BUS_MEGADRIVE_CART_DEV_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_dev_gems_device : public megadrive_rom_device
{
public:
	megadrive_dev_gems_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
//	virtual void device_reset

private:
	std::unique_ptr<u16[]> m_sram;

};

DECLARE_DEVICE_TYPE(MEGADRIVE_DEV_GEMS,    megadrive_dev_gems_device)

#endif // MAME_BUS_MEGADRIVE_CART_DEV_H
