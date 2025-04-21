// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PICO|POWER REDWOOD based PMCs

References:
- https://bitsavers.trailing-edge.com/components/picoPower/PT86C768_Redwood1_2_199404.pdf
- PC-98 undocumented mem vol. 2 io_rwood.txt

TODO:
- stub interface;
- REDWOOD2 known to exist;

**************************************************************************************************/

#include "emu.h"
#include "redwood1.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(REDWOOD1, redwood1_device, "redwood1", "Pico|Power PT86C768 \"REDWOOD1\"")

redwood1_device::redwood1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, REDWOOD1, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("config_space", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor(FUNC(redwood1_device::config_map), this))
{
}

void redwood1_device::device_start()
{

}

void redwood1_device::device_reset()
{
	m_index = 0xffff;
}

device_memory_interface::space_config_vector redwood1_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

u16 redwood1_device::address_r() { return m_index; }
void redwood1_device::address_w(u16 data) { m_index = data; }
u16 redwood1_device::data_r() { return space().read_word(m_index); }
void redwood1_device::data_w(u16 data) { space().write_word(m_index, data); }

void redwood1_device::config_map(address_map &map)
{
// ...
//  map(0x0200, 0x0200) Shadow RAM Read Enable Control
//  map(0x0201, 0x0201) Shadow RAM Write Enable Control
//  map(0x020e, 0x020e) <reserved>
//  ^ fake pc9821ne accesses this for bank control and Flash ROM enable
//    a BIOS section for the JEIDA interface, 4 banks for the MENU, a RAM drive and a "glue logic" bank ...
}

// NOTE: it's paired with the GOLDEN GATE on pc9821nf only,
// but that means it Chip Select on access words ONLY regardless of the existance or not.

// void pc9821_note_state::pc9821ne_io(address_map &map)
//{
//	pc9821_io(map);
//map(0x0900, 0x0901).lrw16(
//    NAME([this] (offs_t offset, u16 mem_mask) {
//        if (!ACCESSING_BITS_0_15)
//        {
//            logerror("Warning: Golden Gate $900 access R %04x\n", mem_mask);
//            return (u16)0xffff;
//        }
//
//        return m_pmc->address_r();
//    }),
//    NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
//        if (!ACCESSING_BITS_0_15)
//        {
//            logerror("Warning: Golden Gate $900 access W %04x & %04x\n", data, mem_mask);
//            return;
//        }
//
//        m_pmc->address_w(data);
//    })
//);
//map(0x0906, 0x0907).lrw16(
//    NAME([this] (offs_t offset, u16 mem_mask) {
//        if (!ACCESSING_BITS_0_15)
//        {
//            logerror("Warning: Golden Gate $906 access R %04x\n", mem_mask);
//            return (u16)0xffff;
//        }
//
//        return m_pmc->data_r();
//    }),
//    NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
//        if (!ACCESSING_BITS_0_15)
//        {
//            logerror("Warning: Golden Gate $906 access W %04x & %04x\n", data, mem_mask);
//            return;
//        }
//
//        m_pmc->data_w(data);
//    })
//);

