ifneq ($(FD_HAS_ZSTD),)

ifneq ($(ROCKSDB),)

ifneq (,$(wildcard $(ROCKSDB)/librocksdb.a))
CFLAGS += -I$(ROCKSDB)/include -DFD_HAS_ROCKSDB=1
LDFLAGS += -lstdc++ $(ROCKSDB)/librocksdb.a -lpthread -lbz2 -lz -ldl -lm
FD_HAS_ROCKSDB:=1
endif

else

CFLAGS += -DFD_HAS_ROCKSDB=1
LDFLAGS += $(shell pkg-config --libs rocksdb) -lbz2
FD_HAS_ROCKSDB:=1

endif

endif
