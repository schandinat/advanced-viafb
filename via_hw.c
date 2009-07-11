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


#include <asm/io.h>
#include <linux/module.h>
#include "share.h"


inline static
u8	read_reg( u16 io_port, u8 index )
{
	outb( index, io_port );
	return inb( io_port + 1 );
}

inline static
void	write_reg( u8 index, u16 io_port, u8 data )
{
	outb( index, io_port );
	outb( data, io_port + 1 );
	return;
}

inline static
void	write_reg_mask( u8 index, u16 io_port, u8 data, u8 mask )
{
	u8	tmp;

	outb( index, io_port );
	tmp = inb( io_port + 1 );
	tmp &= ~mask;
	tmp |= data&mask;
	outb( tmp, io_port + 1 );
	return;
}

void	viafb_SetPrimaryDisplayAddress( u32 addr )
{
	printk( KERN_INFO "viafb_SetPrimaryDisplayAddress( 0x%08X )\n", addr );
	write_reg( 0x0D, VIACR, addr&0xFF );
	write_reg( 0x0C, VIACR, (addr>>8)&0xFF );
	write_reg( 0x34, VIACR, (addr>>16)&0xFF );
	write_reg_mask( 0x48, VIACR, (addr>>24)&0x1F, 0x1F );
	return;
}

void	viafb_SetSecondaryDisplayAddress( u32 addr )
{
	printk( KERN_INFO "viafb_SetSecondaryDisplayAddress( 0x%08X )\n",
		addr );
	/* secondary display supports only quadword aligned memory */
	write_reg_mask( 0x62, VIACR, (addr>>2)&0xFE, 0xFE );
	write_reg( 0x63, VIACR, (addr>>10)&0xFF );
	write_reg( 0x64, VIACR, (addr>>18)&0xFF );
	write_reg_mask( 0xA3, VIACR, (addr>>26)&0x07, 0x07 );
	return;
}

void	viafb_SetPrimaryDisplayLine( u32 length )
{
	printk( KERN_INFO "viafb_SetPrimaryDisplayLine( %d )\n", length );
	/* spec does not say that first adapter skips 3 bits but old 
	   code did it and seems to be reasonable in analogy to 2nd adapter */
	length = length>>3;
	write_reg( 0x13, VIACR, (length)&0xFF );
	write_reg_mask( 0x35, VIACR, (length>>(8 - 5))&0xE0, 0xE0 );
	return;
}

void	viafb_SetSecondaryDisplayLine( u32 length )
{
	printk( KERN_INFO "viafb_SetSecondaryDisplayLine( %d )\n", length );
	length = length>>3;
	write_reg( 0x66, VIACR, (length)&0xFF );
	write_reg_mask( 0x67, VIACR, (length>>8)&0x03, 0x03 );
	write_reg_mask( 0x71, VIACR, (length>>(10 - 7))&0x80, 0x80 );
	return;
}

void	viafb_SetPrimaryDisplayColor( u32 bpp )
{
	u8	csval, mask = 0x0C;

	printk( KERN_INFO "viafb_SetPrimaryDisplayColor( %d )\n", bpp );
	switch (bpp)
	{
		case 8:
			csval = 0x00;
			break;
		case 15:
			csval = 0x01;
			mask |= 0x10;
			break;
		case 16:
			csval = 0x05;
			mask |= 0x10;
			break;
		case 32: /* compatiblity only */
		case 24:
			csval = 0x03;
			break;
		case 30:
			csval = 0x02;
			break;
		default:
			printk( KERN_ALERT "Invalid color depth!\n" );
			return;
	}

	write_reg_mask( 0x15, VIASR, csval<<2, mask );
	return;
}

void	viafb_SetSecondaryDisplayColor( u32 bpp )
{
	u8	csval, mask = 0xC0;

	printk( KERN_INFO "viafb_SetSecondaryDisplayColor( %d )\n", bpp );
	switch (bpp)
	{
		case 8:
			csval = 0x00;
			break;
		case 16:
			csval = 0x01;
			break;
		case 32: /* compatiblity only */
		case 24:
			csval = 0x03;
			break;
		case 30:
			csval = 0x02;
			break;
		default:
			printk( KERN_ALERT "Invalid color depth!\n" );
			return;
	}

	write_reg_mask( 0x67, VIACR, csval<<6, mask );
	return;
}

void	viafb_SetPrimaryDisplayTiming(
	u32 xres,
	u32 yres,
	u32 left_margin,
	u32 right_margin,
	u32 upper_margin,
	u32 lower_margin,
	u32 hsync_len,
	u32 vsync_len )
{
	u16	htotal = (left_margin + right_margin + xres + hsync_len) / 8
			- 5,
		hend = xres / 8 - 1,
		hblank_start = xres / 8 - 1,
		hblank_end = (left_margin + right_margin + xres + hsync_len)
			/ 8 - 1,
		hretrace_start = (right_margin + xres) / 8,
		hretrace_end = (right_margin + xres + hsync_len) / 8,
		vtotal = (upper_margin + lower_margin + yres + vsync_len) - 2,
		vend = yres - 1,
		vblank_start = yres - 1,
		vblank_end = (upper_margin + lower_margin + yres + vsync_len)
			- 1,
		vretrace_start = (lower_margin + yres) - 1,
		vretrace_end = (lower_margin + yres + vsync_len)  - 1;

	printk( KERN_INFO
		"viafb_SetPrimaryDisplayTiming( %d %d %d %d %d %d %d %d )\n",
		xres, yres, left_margin, right_margin, upper_margin,
		lower_margin, hsync_len, vsync_len );

	/* unlock timing registers */
	write_reg_mask( 0x11, VIACR, 0, 0x80 );

	write_reg( 0x00, VIACR, (htotal)&0xFF );
	write_reg( 0x01, VIACR, (hend)&0xFF );
	write_reg( 0x02, VIACR, (hblank_start)&0xFF );
	write_reg_mask( 0x03, VIACR, (hblank_end)&0x1F, 0x1F );
	write_reg( 0x04, VIACR, (hretrace_start)&0xFF );
	write_reg_mask( 0x05,
		VIACR, ((hretrace_end)&0x1F) 
		|((hblank_end<<(7 - 5))&0x80), 0x9F );
	write_reg( 0x06, VIACR, (vtotal)&0xFF );
	write_reg_mask( 0x07,
		VIACR, ((vtotal>>8)&0x01) 
		|((vend>>(8 - 1))&0x02) 
		|((vretrace_start>>(8 - 2))&0x04) 
		|((vblank_start>>(8 - 3))&0x08) 
		|((vtotal>>(9 - 5))&0x20) 
		|((vend>>(9 - 6))&0x40) 
		|((vretrace_start>>(9 - 7))&0x80), 0xEF );
	write_reg_mask( 0x09, VIACR, (vblank_start>>(9 - 5))&0x20, 0x20 );
	write_reg( 0x10, VIACR, (vretrace_start)&0xFF );
	write_reg_mask( 0x11, VIACR, (vretrace_end)&0x0F, 0xF );
	write_reg( 0x12, VIACR, (vend)&0xFF );
	write_reg( 0x15, VIACR, (vblank_start)&0xFF );
	write_reg( 0x16, VIACR, (vblank_end)&0xFF );
	write_reg_mask( 0x33, VIACR,
		((hretrace_start>>(8 - 4))&0x10) 
		|((hblank_end>>(6 - 5))&0x20), 0x30 );
	write_reg_mask( 0x35, VIACR,
		((vtotal>>10)&0x01) 
		|((vretrace_start>>(10 - 1))&0x02)
		|((vend>>(10 - 2))&0x04) 
		|((vblank_start>>(10 - 3))&0x08), 0x0F );
	write_reg_mask( 0x36, VIACR, (htotal>>(8-3))&0x08, 0x08 );

	/* lock timing registers */
	write_reg_mask( 0x11, VIACR, 0x80, 0x80 );	
	return;
}

void	viafb_SetSecondaryDisplayTiming(
	u32 xres,
	u32 yres,
	u32 left_margin,
	u32 right_margin,
	u32 upper_margin,
	u32 lower_margin,
	u32 hsync_len,
	u32 vsync_len )
{
	u16	htotal = (left_margin + right_margin + xres + hsync_len) - 1,
		hend = xres - 1,
		hblank_start = xres - 1,
		hblank_end = (left_margin + right_margin + xres + hsync_len)
			- 1,
		hretrace_start = (right_margin + xres) - 1,
		hretrace_end = (right_margin + xres + hsync_len) - 1,
		vtotal = (upper_margin + lower_margin + yres + vsync_len) - 1,
		vend = yres - 1,
		vblank_start = yres - 1,
		vblank_end = (upper_margin + lower_margin + yres + vsync_len)
			- 1,
		vretrace_start = (lower_margin + yres) - 1,
		vretrace_end = (lower_margin + yres + vsync_len) - 1;

	printk( KERN_INFO
		"viafb_SetSecondaryDisplayTiming( %d %d %d %d %d %d %d %d )\n",
		xres, yres, left_margin, right_margin, upper_margin,
		lower_margin, hsync_len, vsync_len );
	write_reg( 0x50, VIACR, (htotal)&0xFF );
	write_reg( 0x51, VIACR, (hend)&0xFF );
	write_reg( 0x52, VIACR, (hblank_start)&0xFF );
	write_reg( 0x53, VIACR, (hblank_end)&0xFF );
	write_reg( 0x54, VIACR,
		((hblank_start>>8)&0x07)
		|((hblank_end>>(8 - 3))&0x38) 
		|((hretrace_start>>(8 - 6))&0xC0) );
	write_reg_mask( 0x55, VIACR,
		((htotal>>8)&0x0F)
		|((hend>>(8 - 4))&0x70), 0x7F );
	write_reg( 0x56, VIACR, (hretrace_start)&0xFF );
	write_reg( 0x57, VIACR, (hretrace_end)&0xFF );
	write_reg( 0x58, VIACR, (vtotal)&0xFF );
	write_reg( 0x59, VIACR, (vend)&0xFF );
	write_reg( 0x5A, VIACR, (vblank_start)&0xFF );
	write_reg( 0x5B, VIACR, (vblank_end)&0xFF );
	write_reg( 0x5C, VIACR,
		((vblank_start>>8)&0x07)
		|((vblank_end>>(8 - 3))&0x38)
		|((hretrace_end>>(8 - 6))&0x40)
		|((hretrace_start>>(10 - 7))&0x80) );
	write_reg( 0x5D, VIACR,
		((vtotal>>8)&0x07)
		|((vend>>(8 - 3))&0x38)
		|((hblank_end>>(11 - 6))&0x40)
		|((hretrace_start>>(11 - 7))&0x80) );
	write_reg( 0x5E, VIACR, (vretrace_start)&0xFF );
	write_reg( 0x5F, VIACR,
		((vretrace_start>>(8 - 5))&0xE0)
		|((vretrace_end)&0x1F) );	
	return;
}
