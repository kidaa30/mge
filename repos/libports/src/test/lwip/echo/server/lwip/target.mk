TARGET   = test-echo_server_or
LIBS     = base libc lwip config_args libc_lwip_nic_dhcp
SRC_C   = server.c



CC_OPT += -DLWIP_NATIVE

INC_DIR += $(REP_DIR)/src/lib/lwip/include

