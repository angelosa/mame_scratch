// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_MACHINE_WILDCAT_HOST_H
#define MAME_MACHINE_WILDCAT_HOST_H

#pragma once

#include "pci.h"

class wildcat_host_device : public pci_host_device
{
public:
	template <typename T>
	wildcat_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int ram_size)
		: wildcat_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10040007, 0x00, 0x10040007);

		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}

	wildcat_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size) { m_ram_size = ram_size; }


protected:
	wildcat_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void regenerate_mapping();

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	required_device<cpu_device> m_host_cpu;
//	std::vector<uint32_t> m_ram;

	u32 m_ram_size = 0;
};

DECLARE_DEVICE_TYPE(WILDCAT_HOST, wildcat_host_device)

#endif // MAME_MACHINE_WILDCAT_HOST_H
