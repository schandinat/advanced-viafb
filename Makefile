#
# Makefile for the VIA framebuffer driver (for Linux Kernel 2.6)
#

obj-$(CONFIG_FB_VIA) += viafb.o

viafb-y	:=viafbdev.o hw.o via_i2c.o dvi.o lcd.o ioctl.o accel.o vt1636.o global.o tblDPASetting.o viamode.o tbl1636.o via_hw.o
