CXX = g++
CXXFLAG = -O2 

# SUBDIRS = $(shell ls -l |grep ^d | awk '{if($$9!="debug") print $$9}')
SUBDIRS = muduo debug
DEBUG = $(shell ls -l |grep ^d | awk '{if($$9=="debug") print $$9}')
ROOT_DIR=$(shell pwd)
INC=$(ROOT_DIR)

BIN=Exception_test Atomic_unittest Timestamp_unittest Condition_test Thread_test BlockingQueue_test ThreadPool_test TimerQueue_unittest EchoServer_unittest HttpServer_test
# BIN=TimerQueue_unittest
BIN_DIR=$(ROOT_DIR)/debug/bin
OBJS_DIR=$(ROOT_DIR)/debug/obj

CUR_SOURCE = ${wildcard *.cc}
CUR_OBJS=${patsubst,%.cc,%.o,$(CUR_SOURCE)}

export CXX BIN BIN_DIR OBJS_DIR ROOT_DIR INC CXXFLAG

all:$(SUBDIRS) $(CUR_OBJS) DEBUG

$(SUBDIRS):ECHO
	make -C $@
DEBUG:ECHO
	make -C debug
ECHO:
	@echo $(SUBDIRS)
$(CUR_OBJS):%.o:%.cc
	$(CXX) $(CXXFLAG) -c $^ -o $(ROOT_DIR)/$(OBJS_DIR)/$@ -I $(INC)

clean:
	rm $(OBJS_DIR)/*.o
	rm -rf $(BIN_DIR)/*