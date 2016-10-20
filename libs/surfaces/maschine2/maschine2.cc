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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "pbd/compose.h"
#include "pbd/error.h"
#include "pbd/i18n.h"

#include "ardour/session.h"

#include "pbd/abstract_ui.cc" // instantiate template

#include "maschine2.h"
#include "hw_mk2.h"

using namespace ARDOUR;
using namespace PBD;
using namespace ArdourSurface;

Maschine2::Maschine2 (ARDOUR::Session& s)
	: ControlProtocol (s, string (X_("Native Instruments Maschine Controller MK2")))
	, AbstractUI<Maschine2Request> (name())
	, handle (0)
	, hw (0)
{
	if (hid_init()) {
		throw Maschine2Exception ("HIDAPI initialization failed");
	}
	run_event_loop ();
}

Maschine2::~Maschine2 ()
{
	stop ();
	hid_exit ();
	delete hw;
}

void*
Maschine2::request_factory (uint32_t num_requests)
{
	return request_buffer_factory (num_requests);
}

void
Maschine2::do_request (Maschine2Request* req)
{
	if (req->type == CallSlot) {
		call_slot (MISSING_INVALIDATOR, req->the_slot);
	} else if (req->type == Quit) {
		stop ();
	}
}

int
Maschine2::set_active (bool yn)
{
	if (yn == active()) {
		return 0;
	}

	if (yn) {
		if (start ()) {
			return -1;
		}
	} else {
		if (stop ()) {
			return -1;
		}
	}

	ControlProtocol::set_active (yn);
	return 0;
}

XMLNode&
Maschine2::get_state()
{
	XMLNode& node (ControlProtocol::get_state());
	return node;
}

int
Maschine2::set_state (const XMLNode & node, int version)
{
	if (ControlProtocol::set_state (node, version)) {
		return -1;
	}
	return 0;
}

int
Maschine2::start ()
{
	// 17cc:???? Mikro
	// 17cc:1140 Maschine
	// 17cc:1300 Studio
	handle = hid_open (0x17cc, 0x1140, NULL); // Maschine

	if (!handle) {
		error << _("Cannot find or connect to Maschine2");
		return -1;
	}

	hw = new Maschine2Mk2 ();

	Glib::RefPtr<Glib::TimeoutSource> write_timeout = Glib::TimeoutSource::create (40);
	write_connection = write_timeout->connect (sigc::mem_fun (*this, &Maschine2::dev_write));
	write_timeout->attach (main_loop()->get_context());

	Glib::RefPtr<Glib::TimeoutSource> read_timeout = Glib::TimeoutSource::create (1);
	read_connection = read_timeout->connect (sigc::mem_fun (*this, &Maschine2::dev_read));
	read_timeout->attach (main_loop()->get_context());

	return 0;
}

int
Maschine2::stop ()
{
	read_connection.disconnect ();
	write_connection.disconnect ();

	if (handle && hw) {
		hw->clear ();
		hw->write (handle);
	}

	hid_close (handle);
	handle = 0;

	stop_event_loop ();
	return 0;
}

void
Maschine2::thread_init ()
{
	pthread_set_name (event_loop_name().c_str());
	ARDOUR::SessionEvent::create_per_thread_pool (event_loop_name(), 128);
	PBD::notify_event_loops_about_thread_creation (pthread_self(), event_loop_name(), 128);

	struct sched_param rtparam;
	memset (&rtparam, 0, sizeof (rtparam));
	rtparam.sched_priority = 9; /* XXX should be relative to audio (JACK) thread */
	if (pthread_setschedparam (pthread_self(), SCHED_FIFO, &rtparam) != 0) {
		// do we care? not particularly.
	}
}

void
Maschine2::run_event_loop ()
{
	BaseUI::run ();
}

void
Maschine2::stop_event_loop ()
{
	BaseUI::quit ();
}

bool
Maschine2::dev_read ()
{
	hw->read (handle);
	return true;
}

bool
Maschine2::dev_write ()
{
	hw->write (handle);
	return true;
}
