/*
 * Copyright 2009 Florian Tobias Schandinat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTIES OR REPRESENTATIONS; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef __VIA_HW_H__
#define __VIA_HW_H__


void	viafb_SetPrimaryDisplayAddress( u32 addr );
void	viafb_SetSecondaryDisplayAddress( u32 addr );
void	viafb_SetPrimaryDisplayLine( u32 length );
void	viafb_SetSecondaryDisplayLine( u32 length );
void	viafb_SetPrimaryDisplayColor( u32 bpp );
void	viafb_SetSecondaryDisplayColor( u32 bpp );
void	viafb_SetPrimaryDisplayTiming(
	u32 xres,
	u32 yres,
	u32 left_margin,
	u32 right_margin,
	u32 upper_margin,
	u32 lower_margin,
	u32 hsync_len,
	u32 vsync_len );
void	viafb_SetSecondaryDisplayTiming(
	u32 xres,
	u32 yres,
	u32 left_margin,
	u32 right_margin,
	u32 upper_margin,
	u32 lower_margin,
	u32 hsync_len,
	u32 vsync_len );


#endif /* __VIA_HW_H__ */
