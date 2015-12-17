SRC_CC = init.cc plugin.cc

vpath %.cc $(REP_DIR)/src/lib/libc_comm
CC_OPT = -w
LIBS +=  libc libc-resolv libc-isc libc-nameser libc-net libc-rpc  
INC_DIR += $(REP_DIR)/src/server/comm_server/include
