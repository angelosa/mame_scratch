// license:BSD-3-Clause
// copyright-holders:Angelo Salese
// attempt to derive from tms3615.cpp, channels 0~1 recognizable, others not so much

#include "emu.h"
#include "tms3631.h"

#define LIVE_AUDIO_VIEW 0

//#define VERBOSE 1
#include "logmacro.h"
#include <cstring>

// derived from page 9 of TMS3617 (F2)
const int tms3631_device::divisor[16] = {
	0, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 0, 0
};

const double tms3631_device::feet[16] = {
	-1.0, -0.5, -0.5, 0.0, -0.5, 0.0, 0.0, 0.5, -0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 1.0
};

DEFINE_DEVICE_TYPE(TMS3631, tms3631_device, "tms3631", "TMS3631")


tms3631_device::tms3631_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TMS3631, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_channel(nullptr)
	, m_samplerate(0)
	, m_basefreq(0)
	, m_output8(0)
	, m_enable(0)
{
	std::fill(std::begin(m_counter8), std::end(m_counter8), 0);
}


void tms3631_device::device_start()
{
	m_channel = stream_alloc(0, 2, clock() / 8);
	m_samplerate = clock() / 8;
	m_basefreq = clock();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms3631_device::sound_stream_update(sound_stream &stream)
{
	int samplerate = stream.sample_rate();

	constexpr sound_stream::sample_t VMAX = 1.0f / 8;
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		sound_stream::sample_t sum_l = 0, sum_r = 0;

		for (int index = 0; index < 8; index++)
		{
			const u8 tone = m_channel_data[index] & 0xf;
			const u8 octave = (m_channel_data[index] & 0x30) >> 4;
			const u8 div_data = divisor[tone] << octave;

			if (div_data)
			{
				m_counter8[index] -= (m_basefreq / div_data);

				while( m_counter8[index] <= 0 )
				{
					m_counter8[index] += samplerate;
					m_output8 ^= 1 << tone;
				}

				if (m_enable & (1 << index) && m_output8)
				{
					if ((index & 0x6) == 0)
					{
						sum_l += VMAX;
						sum_r += VMAX;
					}
					else if (index < 5)
					{
						sum_l += feet[(m_counter8[index] >> 16) & 0xf];
					}
					else
					{
						sum_r += feet[(m_counter8[index] >> 16) & 0xf];
					}
				}
			}

		}

		stream.put(0, sampindex, sum_l);
		stream.put(1, sampindex, sum_r);
	}
}

// TODO: external to the chip(s)?
void tms3631_device::enable_w(offs_t offset, u8 data)
{
	if (offset)
		popmessage("TMS3631: select with offset (RI105?)");
	m_enable = data;
}

void tms3631_device::data_w(u8 data)
{
	const int ce_state = BIT(data, 7);

	if (!m_ce && ce_state)
	{
		m_program_enable = true;
		m_channel_select = 0;
	}
	else if (m_ce && !ce_state)
	{
		m_program_enable = false;
	}

	if (m_program_enable)
	{
		const int wclk_state = BIT(data, 6);

		if (!m_wclk && wclk_state)
		{
			m_channel_data[m_channel_select] = data & 0x3f;
			m_channel->update();
			if (LIVE_AUDIO_VIEW)
				popmessage(print_audio_state());
		}
		else if (m_wclk && !wclk_state)
		{
			m_channel_select ++;
			m_channel_select &= 7;
		}

		m_wclk = wclk_state;
	}
	m_ce = ce_state;
}

std::string tms3631_device::print_audio_state()
{
	std::ostringstream outbuffer;

	for (int i = 0; i < 8; i++)
	{
		util::stream_format(outbuffer, "%02x ", m_channel_data[i]);
	}

	return outbuffer.str();
}

