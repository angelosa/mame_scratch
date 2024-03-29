// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_NETWORK_I82559_PCI_H
#define MAME_NETWORK_I82559_PCI_H

#pragma once

#include "pci_slot.h"

class i82559_pci_device : public pci_card_device
{
public:
	i82559_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

protected:
	i82559_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

//	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	required_memory_region m_pxe_rom;

private:
	// ...
};

DECLARE_DEVICE_TYPE(I82559_PCI, i82559_pci_device)

#endif // MAME_NETWORK_I82559_PCI_H
