#include <math.h>

#include "maschine2.h"
#include "hw_mk2.h"

#include <pangomm/fontdescription.h>
#include <pango/pangocairo.h>

#include "canvas/colors.h"

using namespace ArdourSurface;

Maschine2Mk2::Maschine2Mk2 ()
{
	_surface[0] = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 256, 64);
	_surface[1] = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 256, 64);

	clear ();

	Pango::FontDescription fd ("Sans 10px");

	Cairo::RefPtr<Cairo::Context> cr0 = Cairo::Context::create (_surface[0]);
	PangoLayout* pl = pango_cairo_create_layout (cr0->cobj ());
	_layout[0] = Glib::wrap (pl);
	_layout[0]->set_font_description (fd);

	Cairo::RefPtr<Cairo::Context> cr1 = Cairo::Context::create (_surface[1]);
	pl = pango_cairo_create_layout (cr1->cobj ());
	_layout[1] = Glib::wrap (pl);
	_layout[1]->set_font_description (fd);


	// paint something...
	_layout[0]->set_text ("ARDOUR");
	cr0->move_to (10, 10);
	cr0->set_source_rgba (1, 1, 1, 1);
	pango_cairo_show_layout (cr0->cobj(), _layout[0]->gobj());
	cr0->rectangle (60, 10, 40, 40);
	cr0->fill ();

	cr1->set_source_rgba (1, 1, 1, 1);
	cr1->arc (128, 32, 27, 0, 2. * M_PI);
	cr1->fill ();
}

void
Maschine2Mk2::clear ()
{
	memset (&ctrl_in, 0, sizeof (ctrl_in));
	memset (pad, 0, sizeof (pad));

	Cairo::RefPtr<Cairo::Context> c = Cairo::Context::create (_surface[0]);
	c->set_operator (Cairo::OPERATOR_CLEAR);
	c->paint ();
	c->set_operator (Cairo::OPERATOR_OVER);
	c = Cairo::Context::create (_surface[1]);
	c->set_operator (Cairo::OPERATOR_CLEAR);
	c->paint ();
	c->set_operator (Cairo::OPERATOR_OVER);
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
		// TODO compare to current data, emit changed signals
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

		// TODO copy settings to generic/abstract interface
		// TODO .. compare & emit changes
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
		_surface[d]->flush ();
		const unsigned char* img = _surface[d]->get_data ();
		int stride = _surface[d]->get_stride ();
		for (int l = 0; l < 8; ++l) {
			buf[3] = 8 * l;
			buf[5] = 0x20;
			buf[7] = 0x08;

			int y0 = l * 8;
			for (int p = 0; p < 256; ++p) {
				uint8_t v = 0;
				int y = y0 + p / 32;
				for (int b = 0; b < 8; ++b) {
					int x = (p % 32) * 8 + b;
					int off = y * stride + x * 4 /* RGBA32 */;
					//printf ("%d %d -> %d\n", x, y, off);
					if (img[off + 1] > 0x7f) {
						v |= 1 << (7 - b);
					}
				}
				buf[9 + p] = v;
			}
			hid_write (handle, buf, 265);
		}
	}
}
