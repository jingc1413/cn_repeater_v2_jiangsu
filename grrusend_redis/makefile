#
# 定时服务池编译脚本
#
include $(HOME)/make.dir/makefile.std

all:grrusend

#grrusend:grrusend.o errorlog_r.o debuglog_r.o threadpool.o
grrusend:grrusend.o errorlog_r.o debuglog_r.o cJSON.o 
	$(CCLINK) -lhiredis -lpthread

