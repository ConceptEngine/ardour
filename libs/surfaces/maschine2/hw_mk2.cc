#include "canvas/colors.h"

#include "maschine2.h"
#include "hw_mk2.h"

using namespace ArdourSurface;

Maschine2Mk2::Maschine2Mk2 ()
{
	clear ();
}

void
Maschine2Mk2::clear ()
{
	memset (&ctrl_in, 0, sizeof (ctrl_in));
	memset (pad, 0, sizeof (pad));
}

void
Maschine2Mk2::read (hid_device* handle)
{
	uint8_t buf[256];
	int res = hid_read(handle, buf, 256);
	if (res < 1) {
		return;
	}

	// TODO parse incrementally if chunked at 64

	if (res == 25 && buf[0] == 0x01) {
		assert (sizeof(ctrl_in) == 24);
		memcpy(&ctrl_in, &buf[1], sizeof(ctrl_in));
		// TODO copy settings to generic/abstract interface
	}
	else if (res > 32 && buf[0] == 0x20) {
		for (int i = 0; i < 16; ++i) {
			uint8_t v0 = buf[1 + 2 * i];
			uint8_t v1 = buf[2 + 2 * i];
			uint8_t p = (v1 & 0xf0) >> 4;
			pad[p] = ((v1 & 0xf) << 8) | v0;
		}
		// TODO read complete 65 byte msg
		// expect buf[33] == 0x00
	}
}

void
Maschine2Mk2::write (hid_device* handle)
{
	uint8_t buf[265];

	/* 31 control buttons: 8 mst + 8 top + 8 pads + 7 mst
	 * 8-bit brightness
	 */
	buf[0] = 0x82; // ctrl-button set command

	// TODO map from generic/abstract interface
	buf[ 1] = ctrl_in.top_control     ? 0xff : 0x00;
	buf[ 2] = ctrl_in.top_step        ? 0xff : 0x00;
	buf[ 3] = ctrl_in.top_browse      ? 0xff : 0x00;
	buf[ 4] = ctrl_in.top_sampling    ? 0xff : 0x00;
	buf[ 5] = ctrl_in.top_left        ? 0xff : 0x00;
	buf[ 6] = ctrl_in.top_right       ? 0xff : 0x00;
	buf[ 7] = ctrl_in.top_all         ? 0xff : 0x00;
	buf[ 8] = ctrl_in.top_auto        ? 0xff : 0x00;

	buf[ 9] = ctrl_in.top_0           ? 0xff : 0x00;
	buf[10] = ctrl_in.top_1           ? 0xff : 0x00;
	buf[11] = ctrl_in.top_2           ? 0xff : 0x00;
	buf[12] = ctrl_in.top_3           ? 0xff : 0x00;
	buf[13] = ctrl_in.top_4           ? 0xff : 0x00;
	buf[14] = ctrl_in.top_5           ? 0xff : 0x00;
	buf[15] = ctrl_in.top_6           ? 0xff : 0x00;
	buf[16] = ctrl_in.top_7           ? 0xff : 0x00;

	buf[17] = ctrl_in.pads_scene      ? 0xff : 0x00;
	buf[18] = ctrl_in.pads_pattern    ? 0xff : 0x00;
	buf[19] = ctrl_in.pads_mode       ? 0xff : 0x00;
	buf[20] = ctrl_in.pads_navigate   ? 0xff : 0x00;
	buf[21] = ctrl_in.pads_duplicate  ? 0xff : 0x00;
	buf[22] = ctrl_in.pads_select     ? 0xff : 0x00;
	buf[23] = ctrl_in.pads_solo       ? 0xff : 0x00;
	buf[24] = ctrl_in.pads_mute       ? 0xff : 0x00;

	buf[25] = ctrl_in.mst_volume      ? 0xff : 0x00;
	buf[26] = ctrl_in.mst_swing       ? 0xff : 0x00;
	buf[27] = ctrl_in.mst_tempo       ? 0xff : 0x00;
	buf[28] = ctrl_in.mst_left        ? 0xff : 0x00;
	buf[29] = ctrl_in.mst_right       ? 0xff : 0x00;
	buf[30] = ctrl_in.mst_enter       ? 0xff : 0x00;
	buf[31] = ctrl_in.mst_note_repeat ? 0xff : 0x00;

	hid_write (handle, buf, 32);

	/* 8 group rgb|rgb + 8 on/off transport buttons */
	buf[0] = 0x81; // group + transport

	uint8_t gbtn = 0;
	gbtn |=  ctrl_in.groups_a ? 0x01 : 0;
	gbtn |=  ctrl_in.groups_b ? 0x02 : 0;
	gbtn |=  ctrl_in.groups_c ? 0x04 : 0;
	gbtn |=  ctrl_in.groups_d ? 0x08 : 0;
	gbtn |=  ctrl_in.groups_e ? 0x10 : 0;
	gbtn |=  ctrl_in.groups_f ? 0x20 : 0;
	gbtn |=  ctrl_in.groups_g ? 0x40 : 0;
	gbtn |=  ctrl_in.groups_h ? 0x80 : 0;

	for (int i = 0; i < 8; ++i) {
		uint32_t rgb;
		if (gbtn & (1<<i)) {
			rgb = 0xffffff;
		} else {
			rgb = 0x000000;
		}
		// left led of button
		buf[1 + i * 6] = (rgb >>  0) & 0xff;
		buf[2 + i * 6] = (rgb >>  8) & 0xff;
		buf[3 + i * 6] = (rgb >> 16) & 0xff;
		// right led of button
		buf[4 + i * 6] = (rgb >>  0) & 0xff;
		buf[5 + i * 6] = (rgb >>  8) & 0xff;
		buf[6 + i * 6] = (rgb >>  16) & 0xff;
	}

	buf[49] = ctrl_in.trs_restart ? 0xff : 0x00;
	buf[50] = ctrl_in.trs_left    ? 0xff : 0x00;
	buf[51] = ctrl_in.trs_right   ? 0xff : 0x00;
	buf[52] = ctrl_in.trs_grid    ? 0xff : 0x00;
	buf[53] = ctrl_in.trs_play    ? 0xff : 0x00;
	buf[54] = ctrl_in.trs_rec     ? 0xff : 0x00;
	buf[55] = ctrl_in.trs_erase   ? 0xff : 0x00;
	buf[56] = ctrl_in.trs_shift   ? 0xff : 0x00;

	hid_write (handle, buf, 57);

	/* 16 RGB grid pads */
	buf[0] = 0x80;
	for (int i = 0; i < 16; ++i) {
		float lvl = pad[i] / 4096.f;
		ArdourCanvas::Color c = ArdourCanvas::hsva_to_color (-30 + 270.f * lvl, .7, lvl * lvl, 1.0);

		buf[1 + i * 3] = (c >> 24) & 0xff; // r
		buf[2 + i * 3] = (c >> 16) & 0xff; // g
		buf[3 + i * 3] = (c >>  8) & 0xff; // b
	}
	hid_write (handle, buf, 49);


	// display
	for (int d = 0; d < 2; ++d) {
		memset (buf, 0, 265);
		buf[0] = 0xe0 | d;
		for (int l = 0; l < 8; ++l) {
			buf[3] = 8 * l;
			buf[5] = 0x20;
			buf[7] = 0x08;
			for (int p = 0; p < 256; ++p) {
				buf[9 + p] = 0x00; // bitmap
			}
			hid_write (handle, buf, 265);
		}
	}
}
