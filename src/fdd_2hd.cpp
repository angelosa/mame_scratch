// license:BSD-3-Clause
// copyright-holders:Angelo Salese
// TODO: doesn't load anything when hooked up (?)
/**************************************************************************************************

FDD 2HD bridge for 1st gen HW

**************************************************************************************************/

#include "emu.h"
#include "fdd_2hd.h"

#include "formats/img_dsk.h"
#include "formats/pc98_dsk.h"
#include "formats/pc98fdi_dsk.h"
#include "formats/fdd_dsk.h"
#include "formats/dcp_dsk.h"
#include "formats/dip_dsk.h"
#include "formats/nfd_dsk.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(FDD_2HD_BRIDGE, fdd_2hd_bridge_device, "pc98_fdd_2hd", "NEC PC-98 2HD FDD bridge")

fdd_2hd_bridge_device::fdd_2hd_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDD_2HD_BRIDGE, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

ROM_START( fdd_2hd )
	ROM_REGION( 0x8000, "bios", ROMREGION_ERASEFF )
	// from CSCP package
	ROM_LOAD( "2hdif.rom", 0x00000, 0x1000, BAD_DUMP CRC(9652011b) SHA1(b607707d74b5a7d3ba211825de31a8f32aec8146) )
ROM_END

const tiny_rom_entry *fdd_2hd_bridge_device::device_rom_region() const
{
	return ROM_NAME( fdd_2hd );
}

static void drives_2hd(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

static void floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_PC98_FORMAT);
	fr.add(FLOPPY_PC98FDI_FORMAT);
	fr.add(FLOPPY_FDD_FORMAT);
	fr.add(FLOPPY_DCP_FORMAT);
	fr.add(FLOPPY_DIP_FORMAT);
	fr.add(FLOPPY_NFD_FORMAT);
	// *nix/FreeBSD may distribute with this
	fr.add(FLOPPY_IMG_FORMAT);
}


void fdd_2hd_bridge_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set([this] (int state) {
		//if (BIT(m_ctrl, 2))
		m_bus->int_w(4, state);
	});
	m_fdc->drq_wr_callback().set([this] (int state) { m_bus->drq_w(2, state); });
	FLOPPY_CONNECTOR(config, "fdc:0", drives_2hd, "525hd", floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", drives_2hd, "525hd", floppy_formats);

}


void fdd_2hd_bridge_device::device_start()
{
	m_bus->set_dma_channel(2, this, true);

	m_fdc->set_rate(500000);
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();

	floppy0->set_rpm(360);
	floppy1->set_rpm(360);


	save_item(NAME(m_ctrl));
}

void fdd_2hd_bridge_device::device_reset()
{
	m_ctrl = 0;
}

void fdd_2hd_bridge_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: any option to disconnect the ROM?
		logerror("map ROM at 0xd7000-0xd7fff\n");
		m_bus->space(AS_PROGRAM).install_rom(
			0xd7000,
			0xd7fff,
			m_bios->base()
		);
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &fdd_2hd_bridge_device::io_map);
	}
}

void fdd_2hd_bridge_device::io_map(address_map &map)
{
	map(0x0090, 0x0090).r(m_fdc, FUNC(upd765a_device::msr_r));
	map(0x0092, 0x0092).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0x0094, 0x0094).rw(FUNC(fdd_2hd_bridge_device::ctrl_r), FUNC(fdd_2hd_bridge_device::ctrl_w));
}

u8 fdd_2hd_bridge_device::ctrl_r()
{
	return 0x44;
}

void fdd_2hd_bridge_device::ctrl_w(u8 data)
{
	//logerror("%02x ctrl\n",data);
	m_fdc->reset_w(BIT(data, 7));

	m_ctrl = data;
	if(data & 0x40)
	{
		m_fdc->set_ready_line_connected(0);
		m_fdc->ready_w(0);
	}
	else
		m_fdc->set_ready_line_connected(1);

//	if(!m_sys_type) // required for 9801f 2hd adapter bios
	{
		m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? ASSERT_LINE : CLEAR_LINE);
		m_fdc->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? ASSERT_LINE : CLEAR_LINE);
	}
}

u8 fdd_2hd_bridge_device::dack_r(int line)
{
	u8 res = m_fdc->dma_r();
	return res;
}

void fdd_2hd_bridge_device::dack_w(int line, u8 data)
{
	m_fdc->dma_w(data);
}

void fdd_2hd_bridge_device::eop_w(int state)
{
	m_fdc->tc_w(state);
}

