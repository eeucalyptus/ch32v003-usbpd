TARGET:=usbpd_fw

ADDITIONAL_C_FILES:=pd_phy.S pd_phy.c pd_crc.c

CH32V003FUN:=ch32v003fun/ch32v003fun
MINICHLINK:=ch32v003fun/minichlink
include ch32v003fun/ch32v003fun/ch32v003fun.mk

all : build_fw

build_fw: $(ADDITIONAL_C_FILES) build

flash : cv_flash
clean : cv_clean