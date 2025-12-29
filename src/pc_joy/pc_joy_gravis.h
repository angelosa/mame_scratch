// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC_JOY_GRAVIS_H
#define MAME_BUS_PC_JOY_GRAVIS_H

#include "pc_joy.h"

class pc_joy_gravis_gamepad : public pc_basic_joy_device
{
public:
	pc_joy_gravis_gamepad(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// 2.5v on idle, 0v pushing left, 5v pushing right
	virtual uint8_t x1(int delta) override {
		if (BIT(m_digital->read(), 2))
			return 0;
		if (BIT(m_digital->read(), 3))
			return 1;
		return delta < 0x80;
	}
	virtual uint8_t x2(int delta) override { return 0xf0 >= delta; }
	virtual uint8_t y1(int delta) override {
		if (BIT(m_digital->read(), 0))
			return 0;
		if (BIT(m_digital->read(), 1))
			return 1;
		return delta < 0x80;
	}
	virtual uint8_t y2(int delta) override { return 0xf0 >= delta; }
	virtual uint8_t btn() override { return m_btn->read(); }

	virtual ioport_constructor device_input_ports() const override;

protected:
	required_ioport m_digital;
};

DECLARE_DEVICE_TYPE(PC_GRAVIS_GAMEPAD, pc_joy_gravis_gamepad)

#endif // MAME_BUS_PC_JOY_GRAVIS_H
