BUILD_DIR := $(shell pwd)/dvbtee

all:

install:
	$(MAKE) -C $(BUILD_DIR) install

%::
	$(MAKE) -C $(BUILD_DIR) $(MAKECMDGOALS)
