APP_DESCRIPTOR_MODULE_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

PYTHON := python3

.PHONY: INSERT_CRC
POST_MAKE_ALL_RULE_HOOK: $(BUILDDIR)/$(PROJECT)-crc.bin
$(BUILDDIR)/$(PROJECT)-crc.bin: $(BUILDDIR)/$(PROJECT).bin
	$(PYTHON) $(APP_DESCRIPTOR_MODULE_DIR)/crc_binary.py $< $@
