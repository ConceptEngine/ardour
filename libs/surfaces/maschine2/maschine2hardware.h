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

#ifndef _ardour_surfaces_maschine2hardware_h_
#define _ardour_surfaces_maschine2hardware_h_

namespace ArdourSurface {

/** Abstraction for various variants:
 *  - NI Maschine Mikro
 *  - NI Maschine
 *  - NI Maschine Studio
 */

class Maschine2Hardware
{
	public:
		Maschine2Hardware () {}
		virtual ~Maschine2Hardware () {}

		virtual void clear () {}
		virtual void read (hid_device*) = 0;
		virtual void write (hid_device*) = 0;
};

} /* namespace */
#endif /* _ardour_surfaces_maschine2_h_*/
