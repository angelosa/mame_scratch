// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_NEON250_H
#define MAME_VIDEO_NEON250_H

#pragma once

#include "machine/pci.h"
#include "video/pc_vga.h"

class neon250_device : public pci_device
{
public:
	neon250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map);
	void legacy_io_map(address_map &map);

protected:
	neon250_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	required_device<vga_device> m_svga;
	required_memory_region m_vga_rom;

//	bool m_vga_legacy_enable = false;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	void null_map(address_map &map);

	u32 unmap_log_r(offs_t offset, u32 mem_mask = ~0);
	void unmap_log_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};

DECLARE_DEVICE_TYPE(NEON250,   neon250_device)

#endif // MAME_VIDEO_NEON250_H
