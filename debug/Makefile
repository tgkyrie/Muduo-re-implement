
ALLOBJS=$(wildcard obj/*.o)
BINOBJS=$(BIN:%=obj/%.o)
OBJS=$(filter-out $(BINOBJS),$(ALLOBJS))
ALLBIN=$(addprefix $(BIN_DIR)/,$(BIN))
# $(info "$(ALLBIN)")
all:$(ALLBIN)
	@echo "finish compile"
$(BIN_DIR)/%:obj/%.o $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAG)

