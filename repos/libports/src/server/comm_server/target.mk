TARGET = comm_server
SRC_CC = main.cc
LIBS   = base lwip libc
CC_OPT = -w


INC_DIR += $(REP_DIR)/src/lib/lwip/include
INC_DIR += $(REP_DIR)/src/server/comm_server/include
  
