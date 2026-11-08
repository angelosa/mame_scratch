// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_NEC_PC98_PCI_CBUS_H
#define MAME_NEC_PC98_PCI_CBUS_H

#pragma once

#include "machine/pci.h"

class pc98_cbus_bridge_device : public pci_device
{
public:
	pc98_cbus_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pc98_cbus_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	// ...
};

DECLARE_DEVICE_TYPE(PC98_CBUS_BRIDGE, pc98_cbus_bridge_device)

#endif // MAME_NEC_PC98_PCI_CBUS_H
