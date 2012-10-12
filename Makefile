SER_DIR := $(shell pwd)/libdvbtee_server
LIB_DIR := $(shell pwd)/libdvbtee
APP_DIR := $(shell pwd)/dvbtee
AP1_DIR := $(shell pwd)/server_example
AP2_DIR := $(shell pwd)/parser_example

all:

install:
	$(MAKE) -C $(LIB_DIR) install
	$(MAKE) -C $(SER_DIR) install
	$(MAKE) -C $(APP_DIR) install
	$(MAKE) -C $(AP1_DIR) install
	$(MAKE) -C $(AP2_DIR) install

%::
	$(MAKE) -C $(LIB_DIR) $(MAKECMDGOALS)
	$(MAKE) -C $(SER_DIR) $(MAKECMDGOALS)
	$(MAKE) -C $(APP_DIR) $(MAKECMDGOALS)
	$(MAKE) -C $(AP1_DIR) $(MAKECMDGOALS)
	$(MAKE) -C $(AP2_DIR) $(MAKECMDGOALS)
