# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_H6_V200) += allwinner-h6-v200.dtb

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS += -a 0x8

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb *.dtbo *_HS
