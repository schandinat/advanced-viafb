/*
 * Copyright 1998-2008 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2008 S3 Graphics, Inc. All Rights Reserved.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTIES OR REPRESENTATIONS; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.See the GNU General Public License
 * for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "global.h"

/* Somehow, the M1 engine has the registers in slightly different
 * locations than previous 2D acceleration engines */
static u_int8_t via_m1_eng_reg[] = {
	[VIA_REG_GECMD]		= VIA_REG_GECMD_M1,
	[VIA_REG_GEMODE]	= VIA_REG_GEMODE_M1,
	[VIA_REG_SRCPOS]	= VIA_REG_SRCPOS_M1,
	[VIA_REG_DSTPOS]	= VIA_REG_DSTPOS_M1,
	[VIA_REG_DIMENSION]	= VIA_REG_DIMENSION_M1,
	[VIA_REG_PATADDR]	= VIA_REG_PATADDR_M1,
	[VIA_REG_FGCOLOR]	= VIA_REG_FGCOLOR_M1,
	[VIA_REG_BGCOLOR]	= VIA_REG_BGCOLOR_M1,
	[VIA_REG_CLIPTL]	= VIA_REG_CLIPTL_M1,
	[VIA_REG_CLIPBR]	= VIA_REG_CLIPBR_M1,
	[VIA_REG_OFFSET]	= VIA_REG_OFFSET_M1,
	[VIA_REG_KEYCONTROL]	= VIA_REG_KEYCONTROL_M1,
	[VIA_REG_SRCBASE]	= VIA_REG_SRCBASE_M1,
	[VIA_REG_DSTBASE]	= VIA_REG_DSTBASE_M1,
	[VIA_REG_PITCH]		= VIA_REG_PITCH_M1,
	[VIA_REG_MONOPAT0]	= VIA_REG_MONOPAT0_M1,
	[VIA_REG_MONOPAT1]	= VIA_REG_MONOPAT1_M1,
};

void viafb_2d_writel(u_int32_t val, u_int32_t reg)
{
	if (viaparinfo->chip_info->twod_engine == VIA_2D_ENG_M1 &&
	    reg < ARRAY_SIZE(via_m1_eng_reg))
		reg = via_m1_eng_reg[reg];

	writel(val, viaparinfo->io_virt + reg);
}

void viafb_init_accel(void)
{
	viaparinfo->fbmem_free -= CURSOR_SIZE;
	viaparinfo->cursor_start = viaparinfo->fbmem_free;
	viaparinfo->fbmem_used += CURSOR_SIZE;

	/* Reverse 8*1024 memory space for cursor image */
	viaparinfo->fbmem_free -= (CURSOR_SIZE + VQ_SIZE);
	viaparinfo->VQ_start = viaparinfo->fbmem_free;
	viaparinfo->VQ_end = viaparinfo->VQ_start + VQ_SIZE - 1;
	viaparinfo->fbmem_used += (CURSOR_SIZE + VQ_SIZE); }

void viafb_init_2d_engine(void)
{
	u32 dwVQStartAddr, dwVQEndAddr;
	u32 dwVQLen, dwVQStartL, dwVQEndL, dwVQStartEndH;
	int i, highest_reg;

	/* init 2D engine regs to reset 2D engine */
	switch (viaparinfo->chip_info->twod_engine) {
	case VIA_2D_ENG_M1:
		highest_reg = 0x5c;
		break;
	default:
		highest_reg = 0x40;
		break;
	}
	for (i = 0; i <= highest_reg; i+= 4)
		writel(0x0, viaparinfo->io_virt + i);

	/* Init AGP and VQ regs */
	switch (viaparinfo->chip_info->name) {
	case UNICHROME_K8M890:
	case UNICHROME_P4M900:
	case UNICHROME_VX800:
	case UNICHROME_VX855:
		writel(0x00100000, viaparinfo->io_virt + VIA_REG_CR_TRANSET);
		writel(0x680A0000, viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
		writel(0x02000000, viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
		break;

	default:
		writel(0x00100000, viaparinfo->io_virt + VIA_REG_TRANSET);
		writel(0x00000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x00333004, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x60000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x61000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x62000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x63000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x64000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		writel(0x7D000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);

		writel(0xFE020000, viaparinfo->io_virt + VIA_REG_TRANSET);
		writel(0x00000000, viaparinfo->io_virt + VIA_REG_TRANSPACE);
		break;
	}
	if (viaparinfo->VQ_start != 0) {
		/* Enable VQ */
		dwVQStartAddr = viaparinfo->VQ_start;
		dwVQEndAddr = viaparinfo->VQ_end;

		dwVQStartL = 0x50000000 | (dwVQStartAddr & 0xFFFFFF);
		dwVQEndL = 0x51000000 | (dwVQEndAddr & 0xFFFFFF);
		dwVQStartEndH = 0x52000000 |
			((dwVQStartAddr & 0xFF000000) >> 24) |
			((dwVQEndAddr & 0xFF000000) >> 16);
		dwVQLen = 0x53000000 | (VQ_SIZE >> 3);
		switch (viaparinfo->chip_info->name) {
		case UNICHROME_K8M890:
		case UNICHROME_P4M900:
		case UNICHROME_VX800:
		case UNICHROME_VX855:
			dwVQStartL |= 0x20000000;
			dwVQEndL |= 0x20000000;
			dwVQStartEndH |= 0x20000000;
			dwVQLen |= 0x20000000;
			break;
		default:
			break;
		}

		switch (viaparinfo->chip_info->name) {
		case UNICHROME_K8M890:
		case UNICHROME_P4M900:
		case UNICHROME_VX800:
		case UNICHROME_VX855:
			writel(0x00100000,
				viaparinfo->io_virt + VIA_REG_CR_TRANSET);
			writel(dwVQStartEndH,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			writel(dwVQStartL,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			writel(dwVQEndL,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			writel(dwVQLen,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			writel(0x74301001,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			writel(0x00000000,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			break;
		default:
			writel(0x00FE0000,
				viaparinfo->io_virt + VIA_REG_TRANSET);
			writel(0x080003FE,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x0A00027C,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x0B000260,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x0C000274,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x0D000264,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x0E000000,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x0F000020,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x1000027E,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x110002FE,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x200F0060,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);

			writel(0x00000006,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x40008C0F,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x44000000,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x45080C04,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x46800408,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);

			writel(dwVQStartEndH,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(dwVQStartL,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(dwVQEndL,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(dwVQLen,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			break;
		}
	} else {
		/* Disable VQ */
		switch (viaparinfo->chip_info->name) {
		case UNICHROME_K8M890:
		case UNICHROME_P4M900:
		case UNICHROME_VX800:
		case UNICHROME_VX855:
			writel(0x00100000,
				viaparinfo->io_virt + VIA_REG_CR_TRANSET);
			writel(0x74301000,
				viaparinfo->io_virt + VIA_REG_CR_TRANSPACE);
			break;
		default:
			writel(0x00FE0000,
				viaparinfo->io_virt + VIA_REG_TRANSET);
			writel(0x00000004,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x40008C0F,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x44000000,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x45080C04,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			writel(0x46800408,
				viaparinfo->io_virt + VIA_REG_TRANSPACE);
			break;
		}
	}

	viafb_set_2d_mode(viafbinfo);
}

/* Set the mode-specific parameters for the 2D acceleration, such as
 * BPP, source and destination base, as well as pitch */
void viafb_set_2d_mode(struct fb_info *info)
{
	u32 dwGEMode, pitch, pitch_reg, base;

	/* Set BPP */
	dwGEMode = readl(viaparinfo->io_virt + 0x04) & 0xFFFFFCFF;
	switch (viaparinfo->bpp) {
	case 16:
		dwGEMode |= VIA_GEM_16bpp;
		break;
	case 32:
		dwGEMode |= VIA_GEM_32bpp;
		break;
	default:
		dwGEMode |= VIA_GEM_8bpp;
		break;
	}
	viafb_2d_writel(dwGEMode, VIA_REG_GEMODE);

	/* Set source and destination base */
	base = ((void *)info->screen_base - viafb_FB_MM);
	viafb_2d_writel(base >> 3, VIA_REG_SRCBASE);
	viafb_2d_writel(base >> 3, VIA_REG_DSTBASE);

	/* Set source and destination pitch (128bit aligned) */
	pitch = (viaparinfo->hres * viaparinfo->bpp >> 3) >> 3;
	pitch_reg = pitch | (pitch << 16);
	if (viaparinfo->chip_info->twod_engine != VIA_2D_ENG_M1)
		pitch_reg |= VIA_PITCH_ENABLE;
	viafb_2d_writel(pitch_reg, VIA_REG_PITCH);
}

void viafb_hw_cursor_init(void)
{
	/* Set Cursor Image Base Address */
	writel(viaparinfo->cursor_start,
		viaparinfo->io_virt + VIA_REG_CURSOR_MODE);
	writel(0x0, viaparinfo->io_virt + VIA_REG_CURSOR_POS);
	writel(0x0, viaparinfo->io_virt + VIA_REG_CURSOR_ORG);
	writel(0x0, viaparinfo->io_virt + VIA_REG_CURSOR_BG);
	writel(0x0, viaparinfo->io_virt + VIA_REG_CURSOR_FG);
}

void viafb_show_hw_cursor(struct fb_info *info, int Status)
{
	u32 temp;
	u32 iga_path = ((struct viafb_par *)(info->par))->iga_path;

	temp = readl(viaparinfo->io_virt + VIA_REG_CURSOR_MODE);
	switch (Status) {
	case HW_Cursor_ON:
		temp |= 0x1;
		break;
	case HW_Cursor_OFF:
		temp &= 0xFFFFFFFE;
		break;
	}
	switch (iga_path) {
	case IGA2:
		temp |= 0x80000000;
		break;
	case IGA1:
	default:
		temp &= 0x7FFFFFFF;
	}
	writel(temp, viaparinfo->io_virt + VIA_REG_CURSOR_MODE);
}

int viafb_wait_engine_idle(void)
{
	int loop = 0;
	u_int32_t status_mask;

	switch (viaparinfo->chip_info->twod_engine) {
	case VIA_2D_ENG_H5:
	case VIA_2D_ENG_M1:
		status_mask = VIA_CMD_RGTR_BUSY_M1 | VIA_2D_ENG_BUSY_M1 |
			      VIA_3D_ENG_BUSY_M1;
		break;
	default:
		while (!(readl(viaparinfo->io_virt + VIA_REG_STATUS) &
				VIA_VR_QUEUE_BUSY) && (loop < MAXLOOP)) {
			loop++;
			cpu_relax();
		}
		status_mask = VIA_CMD_RGTR_BUSY | VIA_2D_ENG_BUSY |
			      VIA_3D_ENG_BUSY;
		break;
	}

	while ((readl(viaparinfo->io_virt + VIA_REG_STATUS) & status_mask) &&
		    (loop < MAXLOOP)) {
		loop++;
		cpu_relax();
	}

	return loop >= MAXLOOP;
}
