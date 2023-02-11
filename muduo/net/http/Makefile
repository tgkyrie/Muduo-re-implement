SUBDIRS =$(shell ls -l| grep ^d | awk '{print $$9}')

CUR_SOURCE=${wildcard *.cc}
CUR_OBJS=${patsubst %.cc, %.o, $(CUR_SOURCE)}

all:$(SUBDIRS) $(CUR_OBJS)
$(SUBDIRS):ECHO
	make -C $@
$(CUR_OBJS):%.o:%.cc
	$(CXX) $(CXXFLAG) -c $^ -o $(OBJS_DIR)/$@ -I ${INC}
ECHO:
	@echo $(SUBDIRS)