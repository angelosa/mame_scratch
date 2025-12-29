// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MACHINE_REDWOOD1_H
#define MAME_MACHINE_REDWOOD1_H

#pragma once

class redwood1_device : public device_t,
                        public device_memory_interface
{
public:
	redwood1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u16 address_r();
	void address_w(u16 data);
	u16 data_r();
	void data_w(u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	void config_map(address_map &map) ATTR_COLD;
private:
	const address_space_config m_space_config;

	u16 m_index;
};

DECLARE_DEVICE_TYPE(REDWOOD1, redwood1_device);

#endif // MAME_MACHINE_REDWOOD1_H
