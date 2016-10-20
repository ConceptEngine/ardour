#include "maschine2hardware.h"

#include <cairomm/context.h>
#include <pangomm/layout.h>

namespace ArdourSurface {

class Maschine2Mk2 : public Maschine2Hardware
{
	public:
		Maschine2Mk2 ();
		void clear ();
		void read (hid_device*);
		void write (hid_device*);

	private:
		struct machine_mk2_input {
			unsigned int top_0           : 1; // 0
			unsigned int top_1           : 1;
			unsigned int top_2           : 1;
			unsigned int top_3           : 1;
			unsigned int top_4           : 1;
			unsigned int top_5           : 1;
			unsigned int top_6           : 1;
			unsigned int top_7           : 1;
			unsigned int top_control     : 1; // 8
			unsigned int top_step        : 1;
			unsigned int top_browse      : 1;
			unsigned int top_sampling    : 1;
			unsigned int top_left        : 1;
			unsigned int top_right       : 1;
			unsigned int top_all         : 1;
			unsigned int top_auto        : 1;
			unsigned int mst_volume      : 1; // 16
			unsigned int mst_swing       : 1;
			unsigned int mst_tempo       : 1;
			unsigned int mst_left        : 1;
			unsigned int mst_right       : 1;
			unsigned int mst_enter       : 1;
			unsigned int mst_note_repeat : 1;
			unsigned int mst_wheel       : 1;
			unsigned int groups_a        : 1; // 24
			unsigned int groups_b        : 1;
			unsigned int groups_c        : 1;
			unsigned int groups_d        : 1;
			unsigned int groups_e        : 1;
			unsigned int groups_f        : 1;
			unsigned int groups_g        : 1;
			unsigned int groups_h        : 1;
			unsigned int trs_restart     : 1; // 32
			unsigned int trs_left        : 1;
			unsigned int trs_right       : 1;
			unsigned int trs_grid        : 1;
			unsigned int trs_play        : 1;
			unsigned int trs_rec         : 1;
			unsigned int trs_erase       : 1;
			unsigned int trs_shift       : 1;
			unsigned int pads_scene      : 1; // 40
			unsigned int pads_pattern    : 1;
			unsigned int pads_mode       : 1;
			unsigned int pads_navigate   : 1;
			unsigned int pads_duplicate  : 1;
			unsigned int pads_select     : 1;
			unsigned int pads_solo       : 1;
			unsigned int pads_mute       : 1;
			uint8_t reserved             : 8; // 48
			uint8_t mst_wheel_pos        : 8; // 56
			uint16_t top_knobs[8];            // 64 ... 191
		} ctrl_in;

		uint16_t pad[16];

		Cairo::RefPtr<Cairo::ImageSurface> _surface[2];
		Glib::RefPtr<Pango::Layout> _layout[2];
};
} /* namespace */
