/*
 * Copyright (C) 2016 Robin Gareus <robin@gareus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _ardour_surfaces_maschine2_h_
#define _ardour_surfaces_maschine2_h_

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <hidapi.h>

#define ABSTRACT_UI_EXPORTS
#include "pbd/abstract_ui.h"
#include "ardour/types.h"
#include "control_protocol/control_protocol.h"

#include "maschine2hardware.h"

namespace ArdourSurface {

class Maschine2Exception : public std::exception
{
	public:
		Maschine2Exception (const std::string& msg) : _msg( msg ) { }
		virtual ~Maschine2Exception() throw () {}
		const char* what() const throw () { return _msg.c_str(); }
	private:
		std::string _msg;
};

struct Maschine2Request : public BaseUI::BaseRequestObject {
  public:
	Maschine2Request () {}
	~Maschine2Request() {}
};

class Maschine2: public ARDOUR::ControlProtocol, public AbstractUI<Maschine2Request>
{
	public:
		Maschine2 (ARDOUR::Session&);
		~Maschine2 ();

		static void* request_factory (uint32_t);

#if 0
		bool has_editor () const { return false; }
		void* get_gui () const;
		void  tear_down_gui ();
#endif

		int set_active (bool yn);
		XMLNode& get_state();
		int set_state (const XMLNode & node, int version);

	private:
		void do_request (Maschine2Request*);

		int start ();
		int stop ();

		void thread_init ();
		void run_event_loop ();
		void stop_event_loop ();

		sigc::connection read_connection;
		sigc::connection write_connection;

		bool dev_write ();
		bool dev_read ();

		hid_device* handle;
		Maschine2Hardware* hw;
};

} /* namespace */
#endif /* _ardour_surfaces_maschine2_h_*/
