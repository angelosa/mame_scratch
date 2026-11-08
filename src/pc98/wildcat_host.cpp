// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"

#include "wildcat_host.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(WILDCAT_HOST, wildcat_host_device,   "pcmc_wildcat_host",   "VLSI VL82C594-AFC2 \"Wildcat\"")


wildcat_host_device::wildcat_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, type, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
{
}

wildcat_host_device::wildcat_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wildcat_host_device(mconfig, WILDCAT_HOST, tag, owner, clock)
{
}

void wildcat_host_device::device_start()
{
	pci_host_device::device_start();
	set_spaces(&m_host_cpu->space(AS_PROGRAM), &m_host_cpu->space(AS_IO));

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	// TODO: check me out
	command = 0x0000;
	command_mask = 0x0;
	status = 0x00;

//	m_ram.resize(m_ram_size/4);

	intr_pin = 0;
}

void wildcat_host_device::device_reset()
{
	pci_host_device::device_reset();
}



void wildcat_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
}

void wildcat_host_device::regenerate_mapping()
{
	logerror("Regenerating mapping\n");
	//memory_space->unmap_readwrite(memory_window_start, memory_window_end);
	//io_space->unmap_readwrite(io_window_start, io_window_end);

	//map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
	//			io_window_start, io_window_end, io_offset, io_space);
	//remap_cb();
}


void wildcat_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);
}

