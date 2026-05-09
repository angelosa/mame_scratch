// license:BSD-3-Clause
// copyright-holders: Angelo Salese

// a failed attempt at converting Conny bootleg with the newer MD semantics.
// It sure has a stroke with z80 open bus shenanigans ...

#include "emu.h"
#include "bus/sms_ctrl/controllers.h"
#include "bus/sms_ctrl/smsctrl.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/intelfsh.h"
#include "machine/sega_md_ioport.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"
#include "video/ym7101.h"

#include "screen.h"
#include "speaker.h"


namespace {

class megadrive_conny_state : public driver_device
{
public:
	megadrive_conny_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_md68kcpu(*this, "md68kcpu")
		, m_mdz80cpu(*this, "mdz80cpu")
		, m_mdscreen(*this, "mdscreen")
		, m_md_vdp(*this, "md_vdp")
		, m_opn(*this, "opn")
		, m_md_68k_sound_view(*this, "md_68k_sound_view")
		, m_md_ctrl_ports(*this, { "md_ctrl1", "md_ctrl2", "md_exp" })
		, m_md_ioports(*this, "md_ioport%u", 1U)
	{ }

	void megadrive_conny(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	void md_68k_map(address_map &map) ATTR_COLD;
	void md_cpu_space_map(address_map &map);
	void md_68k_z80_map(address_map &map) ATTR_COLD;
	void md_z80_map(address_map &map) ATTR_COLD;
	void md_z80_io(address_map &map) ATTR_COLD;
	void md_ioctrl_map(address_map &map) ATTR_COLD;
private:

	// MD side
	required_device<m68000_device> m_md68kcpu;
	required_device<z80_device> m_mdz80cpu;
	required_device<screen_device> m_mdscreen;
//	required_memory_bank m_tmss_bank;
//	memory_view m_tmss_view;
//	required_device<megadrive_cart_slot_device> m_md_cart;
	required_device<ym7101_device> m_md_vdp;
	required_device<ym_generic_device> m_opn;
	memory_view m_md_68k_sound_view;
	required_device_array<sms_control_port_device, 3> m_md_ctrl_ports;
	required_device_array<megadrive_io_port_device, 3> m_md_ioports;

	u32 md_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	bool m_z80_reset = false;
	bool m_z80_busreq = false;
	u32 m_z80_main_address = 0;
	std::unique_ptr<u8[]> m_sound_program;

	void flush_z80_state();
};

u32 megadrive_conny_state::md_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_md_vdp->screen_update(screen, bitmap, cliprect);
	return 0;
}

void megadrive_conny_state::md_68k_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x3fffff).rom();

//  map(0x800000, 0x9fffff) unmapped or 32X
	map(0xa00000, 0xa07fff).view(m_md_68k_sound_view);
	m_md_68k_sound_view[0](0xa00000, 0xa07fff).before_delay(NAME([](offs_t) { return 1; })).m(*this, FUNC(megadrive_conny_state::md_68k_z80_map));
//  map(0xa07f00, 0xa07fff) Z80 VDP space (freezes machine if accessed from 68k)
//  map(0xa08000, 0xa0ffff) Z80 68k window (assume no DTACK), or just mirror of above according to TD HW notes?
//  map(0xa10000, 0xa100ff) I/O
	map(0xa10000, 0xa100ff).m(*this, FUNC(megadrive_conny_state::md_ioctrl_map));

//  map(0xa11000, 0xa110ff) memory mode register
//  map(0xa11100, 0xa111ff) Z80 BUSREQ/BUSACK
	map(0xa11100, 0xa11101).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			address_space &space = m_md68kcpu->space(AS_PROGRAM);
			u16 open_bus = space.read_word(m_md68kcpu->pc() - 2) & 0xfefe;
			u16 res = (m_mdz80cpu->busack_r() && !m_z80_reset) ^ 1;
			return (res << 8) | (res) | open_bus;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (!ACCESSING_BITS_0_7)
			{
				m_z80_busreq = !!BIT(~data, 8);
			}
			else if (!ACCESSING_BITS_8_15)
			{
				m_z80_busreq = !!BIT(~data, 0);
			}
			else // word access
			{
				m_z80_busreq = !!BIT(~data, 8);
			}
			flush_z80_state();
		})
	);
//  map(0xa11200, 0xa112ff) Z80 RESET
	map(0xa11200, 0xa11201).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (!ACCESSING_BITS_0_7)
			{
				m_z80_reset = !!BIT(~data, 8);
			}
			else if (!ACCESSING_BITS_8_15)
			{
				m_z80_reset = !!BIT(~data, 0);
			}
			else // word access
			{
				m_z80_reset = !!BIT(~data, 8);
			}
			flush_z80_state();
		})
	);
//  map(0xa11300, 0xa113ff) <open bus>

//  map(0xa11400, 0xa1dfff) <unmapped> (no DTACK generation, freezes machine without additional HW)
//  map(0xa12000, 0xa120ff).m(m_exp, FUNC(...::fdc_map));
//	map(0xa13000, 0xa130ff).rw(m_md_cart, FUNC(megadrive_cart_slot_device::time_r), FUNC(megadrive_cart_slot_device::time_w));
//  map(0xa14000, 0xa14003) TMSS lock
//  map(0xa15100, 0xa153ff) 32X registers if present, <unmapped> otherwise
//  map(0xae0000, 0xae0003) Teradrive bus switch registers
//  map(0xc00000, 0xdfffff) VDP and PSG (with mirrors and holes)
//  $d00000 alias required by earthdef
	map(0xc00000, 0xc0001f).mirror(0x100000).m(m_md_vdp, FUNC(ym7101_device::if16_map));
	map(0xe00000, 0xe0ffff).mirror(0x1f0000).ram(); // Work RAM, usually accessed at $ff0000
}

// $a10000 base
void megadrive_conny_state::md_ioctrl_map(address_map &map)
{
	// version, should be 0 for Teradrive, bit 5 for expansion bus not connected yet
	map(0x00, 0x01).lr8(NAME([] () { return 1 << 5; }));
	map(0x02, 0x03).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::data_r), FUNC(megadrive_io_port_device::data_w));
	map(0x04, 0x05).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::data_r), FUNC(megadrive_io_port_device::data_w));
	map(0x06, 0x07).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::data_r), FUNC(megadrive_io_port_device::data_w));
	map(0x08, 0x09).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::ctrl_r), FUNC(megadrive_io_port_device::ctrl_w));
	map(0x0a, 0x0b).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::ctrl_r), FUNC(megadrive_io_port_device::ctrl_w));
	map(0x0c, 0x0d).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::ctrl_r), FUNC(megadrive_io_port_device::ctrl_w));
	map(0x0e, 0x0f).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::txdata_r), FUNC(megadrive_io_port_device::txdata_w));
	map(0x10, 0x11).r(m_md_ioports[0], FUNC(megadrive_io_port_device::rxdata_r));
	map(0x12, 0x13).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::s_ctrl_r), FUNC(megadrive_io_port_device::s_ctrl_w));
	map(0x14, 0x15).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::txdata_r), FUNC(megadrive_io_port_device::txdata_w));
	map(0x16, 0x17).r(m_md_ioports[1], FUNC(megadrive_io_port_device::rxdata_r));
	map(0x18, 0x19).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::s_ctrl_r), FUNC(megadrive_io_port_device::s_ctrl_w));
	map(0x1a, 0x1b).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::txdata_r), FUNC(megadrive_io_port_device::txdata_w));
	map(0x1c, 0x1d).r(m_md_ioports[2], FUNC(megadrive_io_port_device::rxdata_r));
	map(0x1e, 0x1f).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::s_ctrl_r), FUNC(megadrive_io_port_device::s_ctrl_w));
}

void megadrive_conny_state::md_cpu_space_map(address_map &map)
{
	map(0xfffff3, 0xfffff3).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 25; }));
	// TODO: IPL0 (external irq tied to VDP IE2)
	map(0xfffff5, 0xfffff5).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 26; }));
	map(0xfffff7, 0xfffff7).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 27; }));
	map(0xfffff9, 0xfffff9).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([this] () -> u8 {
		m_md_vdp->irq_ack();
		return 28;
	}));
	map(0xfffffb, 0xfffffb).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 29; }));
	map(0xfffffd, 0xfffffd).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([this] () -> u8 {
		m_md_vdp->irq_ack();
		return 30;
	}));
	map(0xffffff, 0xffffff).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 31; }));
}

void megadrive_conny_state::flush_z80_state()
{
	m_mdz80cpu->set_input_line(INPUT_LINE_RESET, m_z80_reset ? ASSERT_LINE : CLEAR_LINE);
	m_mdz80cpu->set_input_line(Z80_INPUT_LINE_BUSREQ, m_z80_busreq ? CLEAR_LINE : ASSERT_LINE);
	if (m_z80_reset)
		m_opn->reset();
	if (m_z80_reset || !m_z80_busreq)
		m_md_68k_sound_view.select(0);
	else
		m_md_68k_sound_view.disable();
}

void megadrive_conny_state::md_68k_z80_map(address_map &map)
{
	map(0x0000, 0x3fff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			u16 res = m_sound_program[(offset << 1) ^ 1] | (m_sound_program[offset << 1] << 8);
			return res;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (!ACCESSING_BITS_0_7)
			{
				m_sound_program[(offset<<1)] = (data & 0xff00) >> 8;
			}
			else if (!ACCESSING_BITS_8_15)
			{
				m_sound_program[(offset<<1) ^ 1] = (data & 0x00ff);
			}
			else
			{
				m_sound_program[(offset<<1)] = (data & 0xff00) >> 8;
			}
		})
	);
	map(0x4000, 0x4003).mirror(0x1ffc).rw(m_opn, FUNC(ym_generic_device::read), FUNC(ym_generic_device::write));
}

void megadrive_conny_state::md_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).lrw8(
		NAME([this] (offs_t offset) {
			return m_sound_program[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sound_program[offset] = data;
		})
	);
	map(0x4000, 0x4003).mirror(0x1ffc).rw(m_opn, FUNC(ym_generic_device::read), FUNC(ym_generic_device::write));
	map(0x6000, 0x60ff).lw8(NAME([this] (offs_t offset, u8 data) {
		m_z80_main_address = ((m_z80_main_address >> 1) | ((data & 1) << 23)) & 0xff8000;
	}));
	map(0x7f00, 0x7f1f).m(m_md_vdp, FUNC(ym7101_device::if8_map));
	map(0x8000, 0xffff).lrw8(
		NAME([this] (offs_t offset) {
			address_space &space68k = m_md68kcpu->space();
			u8 ret = space68k.read_byte(m_z80_main_address + offset);
			return ret;

		}),
		NAME([this] (offs_t offset, u8 data) {
			address_space &space68k = m_md68kcpu->space();
			space68k.write_byte(m_z80_main_address + offset, data);
		})
	);
}

void megadrive_conny_state::md_z80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	// juuouki reads $bf in irq service (SMS VDP left-over?)
	map(0x00, 0xff).nopr();
	// TODO: SMS mode thru cart
}

void megadrive_conny_state::machine_start()
{
	m_sound_program = std::make_unique<u8[]>(0x2000);

	save_pointer(NAME(m_sound_program), 0x2000);

	save_item(NAME(m_z80_reset));
	save_item(NAME(m_z80_busreq));
	save_item(NAME(m_z80_main_address));
}

void megadrive_conny_state::machine_reset()
{
	m_z80_reset = true;
	flush_z80_state();
}

void megadrive_conny_state::megadrive_conny(machine_config &config)
{
	// MD section (NTSC)
	constexpr XTAL md_master_xtal(53.693175_MHz_XTAL);

	M68000(config, m_md68kcpu, md_master_xtal / 7);
	m_md68kcpu->set_addrmap(AS_PROGRAM, &megadrive_conny_state::md_68k_map);
	m_md68kcpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &megadrive_conny_state::md_cpu_space_map);
	// disallow TAS (gargoyle, cliffh, exmutant)
	m_md68kcpu->set_tas_write_callback(NAME([] (offs_t offset, u8 data) { }));

	Z80(config, m_mdz80cpu, md_master_xtal / 15);
	m_mdz80cpu->set_addrmap(AS_PROGRAM, &megadrive_conny_state::md_z80_map);
	m_mdz80cpu->set_addrmap(AS_IO, &megadrive_conny_state::md_z80_io);

	SCREEN(config, m_mdscreen, SCREEN_TYPE_RASTER);
	// NOTE: PAL is 423x312
	m_mdscreen->set_raw(md_master_xtal / 8, 427, 0, 320, 262, 0, 224);
	m_mdscreen->set_screen_update(FUNC(megadrive_conny_state::md_screen_update));

	YM7101(config, m_md_vdp, md_master_xtal / 4);
	m_md_vdp->set_mclk(md_master_xtal);
	m_md_vdp->set_screen(m_mdscreen);
	// TODO: actual DTACK
	// TODO: accessing 68k bus from x86, defers access?
	m_md_vdp->dtack_cb().set_inputline(m_md68kcpu, INPUT_LINE_HALT);
	m_md_vdp->mreq_cb().set([this] (offs_t offset) {
		// TODO: can't read outside of base cart and RAM
		//printf("%06x\n", offset);
		address_space &space68k = m_md68kcpu->space();
		u16 ret = space68k.read_word(offset, 0xffff);

		return ret;
	});
	m_md_vdp->vint_cb().set_inputline(m_md68kcpu, 6);
	m_md_vdp->hint_cb().set_inputline(m_md68kcpu, 4);
	m_md_vdp->sint_cb().set_inputline(m_mdz80cpu, INPUT_LINE_IRQ0);
	m_md_vdp->add_route(ALL_OUTPUTS, "md_speaker", 0.50, 0);
	m_md_vdp->add_route(ALL_OUTPUTS, "md_speaker", 0.50, 1);

	auto &hl(INPUT_MERGER_ANY_HIGH(config, "hl"));
	// TODO: gated thru VDP
	hl.output_handler().set_inputline(m_md68kcpu, 2);

	MEGADRIVE_IO_PORT(config, m_md_ioports[0], 0);
	m_md_ioports[0]->hl_handler().set("hl", FUNC(input_merger_device::in_w<0>));

	MEGADRIVE_IO_PORT(config, m_md_ioports[1], 0);
	m_md_ioports[1]->hl_handler().set("hl", FUNC(input_merger_device::in_w<1>));

	MEGADRIVE_IO_PORT(config, m_md_ioports[2], 0);
	m_md_ioports[2]->hl_handler().set("hl", FUNC(input_merger_device::in_w<2>));

	for (int N = 0; N < 3; N++)
	{
		SMS_CONTROL_PORT(config, m_md_ctrl_ports[N], sms_control_port_devices, N != 2 ? SMS_CTRL_OPTION_MD_PAD : nullptr);
		m_md_ctrl_ports[N]->th_handler().set(m_md_ioports[N], FUNC(megadrive_io_port_device::th_w));
		m_md_ctrl_ports[N]->set_screen(m_mdscreen);

		m_md_ioports[N]->set_in_handler(m_md_ctrl_ports[N], FUNC(sms_control_port_device::in_r));
		m_md_ioports[N]->set_out_handler(m_md_ctrl_ports[N], FUNC(sms_control_port_device::out_w));
	}

	SPEAKER(config, "md_speaker", 2).front();

	YM2612(config, m_opn, md_master_xtal / 7);
	m_opn->add_route(0, "md_speaker", 0.50, 0);
	m_opn->add_route(1, "md_speaker", 0.50, 1);
}


ROM_START( contrambc )
	ROM_REGION( 0x400000, "md68kcpu", 0 )
	ROM_LOAD16_BYTE( "rom.m3", 0x000000, 0x080000, CRC(990f824d) SHA1(95f769a9f9263479ca74fdaee12a3eae16cae432) )
	ROM_LOAD16_BYTE( "rom.m6", 0x000001, 0x080000, CRC(ab83e795) SHA1(cd2da23ffd5e0ca954bcdfd2a9f48e16cdd587a0) )
	ROM_LOAD16_BYTE( "rom.m2", 0x100000, 0x080000, CRC(28b9d1a9) SHA1(0a7536088e61239c5c777847e57b282289b006f2) )
	ROM_LOAD16_BYTE( "rom.m5", 0x100001, 0x080000, CRC(1c92ebf1) SHA1(33e7aff01fa9db7900a4e1a834772e128c06b148) )
ROM_END


} // anonymous namespace

GAME( 1995, contrambc, 0,        megadrive_conny,      0,    megadrive_conny_state, empty_init,    ROT0, "bootleg (Conny / Chuangyi)",    "Contra (Conny bootleg of Mega Drive version)",                                                    MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // doesn't seem to like megadriv_68k_check_z80_bus(), crashes after title, doesn't read start button even if credits are in.

