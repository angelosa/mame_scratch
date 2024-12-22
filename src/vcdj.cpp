// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Virtual CDJ Player a.k.a. Tech 12s DIY

TODO:
- Make the two decks standalone devices, possibly make slot configurable;
- Artwork;
- Separate headphone out;
- Volume metering, possibly in-time beat metering;
- BPM control (needs CDDA output finer interpolation);
- Equalizers;
- Allow turntablism;
- Read data tracks, for .mp3/.flac etc. playback;

**************************************************************************************************/

#include "emu.h"
#include "speaker.h"

#include "imagedev/cdromimg.h"
#include "sound/cdda.h"

namespace {

class virtual_cdj_state : public driver_device
{
public:
	virtual_cdj_state(const machine_config &mconfig, device_type type, const char *tag);

	void vcdj(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(mixer_changed);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device_array<cdrom_image_device, 2> m_cdrom;
	required_device_array<cdda_device, 2> m_cdda;
	required_ioport m_crossfader;
	required_ioport_array<2> m_in;
    required_ioport_array<2> m_vol;

	emu_timer *m_player_timer[2];
	template <unsigned N> TIMER_CALLBACK_MEMBER(player_cb);
	u32 m_lba[2], m_cue_offset[2];
	bool m_pause_state[2];
    u8 m_track_change[2];
	void mixer_gain_update();
};

virtual_cdj_state::virtual_cdj_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_cdrom(*this, "cdrom_%u", 0U)
	, m_cdda(*this, "cdda_%u", 0U)
	, m_crossfader(*this, "CROSSFADER")
	, m_in(*this, "IN%u", 0U)
    , m_vol(*this, "VOL%u", 0U)
{
}

void virtual_cdj_state::mixer_gain_update()
{
    const u8 crossfader = m_crossfader->read();
    const u8 left_input_gain = m_vol[0]->read();
    const u8 right_input_gain = m_vol[1]->read();

	const float left_gain = (crossfader / 15.0) * (left_input_gain / 255.0);
	m_cdda[0]->set_output_gain(ALL_OUTPUTS, left_gain);
	const float right_gain = ((0xf - crossfader) / 15.0) * (right_input_gain / 255.0);
	m_cdda[1]->set_output_gain(ALL_OUTPUTS, right_gain);

    popmessage("%02x %02x %02x -> %f %f", crossfader, left_input_gain, right_input_gain, left_gain, right_gain);
}

INPUT_CHANGED_MEMBER(virtual_cdj_state::mixer_changed)
{
	mixer_gain_update();
}

static INPUT_PORTS_START( vcdj )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Left Play / Pause") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Left Cue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Left Previous Track")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Left Next Track")

    PORT_START("VOL0")
    PORT_BIT( 0xff, 0x80, IPT_POSITIONAL_V ) PORT_POSITIONS(256) PORT_SENSITIVITY(70) PORT_KEYDELTA(3) PORT_CENTERDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(virtual_cdj_state::mixer_changed), 0) PORT_NAME("Left Volume")

	PORT_START("CROSSFADER")
	PORT_BIT( 0x0f, 0x07, IPT_POSITIONAL ) PORT_POSITIONS(16) PORT_SENSITIVITY(4) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_RIGHT) PORT_CODE_INC(KEYCODE_LEFT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(virtual_cdj_state::mixer_changed), 0) PORT_NAME("Crossfader")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Right Play / Pause") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Right Cue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Right Previous Track")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("Right Next Track")

    PORT_START("VOL1")
    PORT_BIT( 0xff, 0x80, IPT_POSITIONAL_V ) PORT_POSITIONS(256) PORT_SENSITIVITY(70) PORT_KEYDELTA(3) PORT_CENTERDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(virtual_cdj_state::mixer_changed), 0) PORT_NAME("Right Volume")
INPUT_PORTS_END


void virtual_cdj_state::machine_start()
{
	m_player_timer[0] = timer_alloc(FUNC(virtual_cdj_state::player_cb<0>), this);
	m_player_timer[1] = timer_alloc(FUNC(virtual_cdj_state::player_cb<1>), this);
}

void virtual_cdj_state::machine_reset()
{
	m_player_timer[0]->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));
	m_player_timer[1]->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));
	m_lba[0] = m_cue_offset[0] = 0;
	m_lba[1] = m_cue_offset[1] = 0;
	mixer_gain_update();
}

template <unsigned N> TIMER_CALLBACK_MEMBER(virtual_cdj_state::player_cb)
{
	const u8 player_input = m_in[N]->read();

	printf("%d %d %d (%02x)\n", N, m_lba[N], m_cue_offset[N], player_input);

	if (!m_cdrom[N]->exists())
	{
		m_pause_state[N] = !!BIT(player_input, 0);
        m_lba[N] = m_cue_offset[N] = 0;
        m_cdda[N]->stop_audio();
        printf("\tstopped\n");
		return;
	}

    if (player_input & 0xc)
    {
        if (m_track_change[N])
        {
            return;
        }

        int track = m_cdrom[N]->get_track(m_lba[N]);
        if (BIT(player_input, 2))
        {
            const u32 cur_track_marker = m_cdrom[N]->get_track_start(track);
            track --;
            const u32 prev_track_marker = m_cdrom[N]->get_track_start(std::max(track, 0));
            printf("%d previous track change %d (%d)\n", track, m_lba[N], m_lba[N] - cur_track_marker);
            m_lba[N] = (m_lba[N] - cur_track_marker <= 150) ? prev_track_marker : cur_track_marker;
            //m_cue_offset[N] = 0;
        }

        if (BIT(player_input, 3))
        {
            track ++;
            m_lba[N] = m_cdrom[N]->get_track_start(std::min(track, m_cdrom[N]->get_last_track()));
            //m_cue_offset[N] = 0;
            printf("%d next track change %d\n", track, m_lba[N]);
        }

        m_track_change[N] = player_input & 0xc;
        return;
    }

    m_track_change[N] = 0;

	if (BIT(player_input, 0) == m_pause_state[N])
    {
        printf("\tpaused\n");
        if (BIT(player_input, 1))
        {
            m_cue_offset[N] = m_lba[N];
            printf("\tsave cue point %08x\n", m_cue_offset[N]);
        }
		return;
    }

    if (BIT(player_input, 1))
    {
        m_lba[N] = m_cue_offset[N];
        printf("\tload cue point %d\n", m_cue_offset[N]);
        return;
    }

	const u32 lba_value = m_lba[N];

    if (lba_value >= m_cdrom[N]->get_track_start(0xaa))
    {
		m_pause_state[N] = !!BIT(player_input, 0);
        m_lba[N] = 0;
        m_cdda[N]->stop_audio();
        printf("\toverflow stop\n");
        return;
    }

	m_cdda[N]->start_audio(lba_value, 1);
    m_lba[N]++;

    //const u32 msf = cdrom_file::lba_to_msf(lba_value);
    //popmessage("%02x:%02x:%02x", (msf >> 16) & 0xff, (msf >> 8) & 0xff, msf & 0xff);
    printf("%d\n", m_cdrom[N]->get_track_start(0xaa));
}

void virtual_cdj_state::vcdj(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	CDROM(config, m_cdrom[0]);

	CDDA(config, m_cdda[0]);
	m_cdda[0]->set_cdrom_tag(m_cdrom[0]);
	//m_cdda[0]->audio_end_cb().set ...
	m_cdda[0]->add_route(0, "lspeaker", 1.00);
	m_cdda[0]->add_route(1, "rspeaker", 1.00);

	CDROM(config, m_cdrom[1]);

	CDDA(config, m_cdda[1]);
	m_cdda[1]->set_cdrom_tag(m_cdrom[1]);
	//m_cdda[1]->audio_end_cb().set ...
	m_cdda[1]->add_route(0, "lspeaker", 1.00);
	m_cdda[1]->add_route(1, "rspeaker", 1.00);
}

ROM_START( vcdj )
ROM_END

} // anonymous namespace

CONS( 2024, vcdj, 0, 0, vcdj, vcdj, virtual_cdj_state, empty_init, "MAME", "Virtual CDJ player", MACHINE_NOT_WORKING )
