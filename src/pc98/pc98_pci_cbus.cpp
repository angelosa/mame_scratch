// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"

#include "pc98_pci_cbus.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(PC98_CBUS_BRIDGE, pc98_cbus_bridge_device,   "pc98_cbus_bridge",   "NEC PC-98 PCI to C-Bus Bridge")


pc98_cbus_bridge_device::pc98_cbus_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
{
}

pc98_cbus_bridge_device::pc98_cbus_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc98_cbus_bridge_device(mconfig, PC98_CBUS_BRIDGE, tag, owner, clock)
{
	set_ids(0x10330001, 0x00, 0x068000, 0x10330001);
}

void pc98_cbus_bridge_device::device_add_mconfig(machine_config &config)
{

}

void pc98_cbus_bridge_device::device_start()
{
	pci_device::device_start();

	// INTA#
	intr_pin = 1;
}

void pc98_cbus_bridge_device::device_reset()
{
	pci_device::device_reset();

	// TODO: verify
	command = 0x010f;
	command_mask = 0x010f;
	status = 0x0200;

	// TODO: needs a PCI host (TBD!!!)
//	remap_cb();
}

void pc98_cbus_bridge_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}
