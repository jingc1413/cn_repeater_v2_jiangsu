#
# 定时服务池编译脚本
#
include $(HOME)/make.dir/makefile.std

all:timeserv houralarm dayalarm timetask

timeserv:timeserver.o alarm_util.o cJSON.o
	$(CCLINK) -lhiredis

houralarm:houralarm.o
	$(CCLINK)
	
repeatinfo:repeatinfo.o
	$(CCLINK)
systemalarm:systemalarm.o
	$(CCLINK)
	
timetask:timetask.o
	$(CCLINK)

dayalarm:dayalarm.o alarm_util.o
	$(CCLINK)

dbtest:dbtest.o cJSON.o
	$(CCLINK) -lhiredis