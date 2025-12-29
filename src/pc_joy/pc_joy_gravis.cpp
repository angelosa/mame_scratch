// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc_joy_gravis.h"

DEFINE_DEVICE_TYPE(PC_GRAVIS_GAMEPAD, pc_joy_gravis_gamepad, "gravis_gamepad", "Gravis PC GamePad")


pc_joy_gravis_gamepad::pc_joy_gravis_gamepad(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: pc_basic_joy_device(mconfig, PC_GRAVIS_GAMEPAD, tag, owner, clock)
	, m_digital(*this, "btn")
{
}

static INPUT_PORTS_START( gravis_gamepad )
	PORT_START("btn")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button 1 (Red)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button 2 (Blue)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button 3 (Yellow)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button 4 (Green)")

	PORT_START("digital")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("x1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("y1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("x2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("y2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor pc_joy_gravis_gamepad::device_input_ports() const
{
	return INPUT_PORTS_NAME( gravis_gamepad );
}
