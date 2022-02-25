include $(HOME)/make.dir/makefile.std

all:applserv gprsserv 

applserv:appl_redis.o cJSON.o appl_util.o appl_qryandset.o appl_alarm.o appl_gprs.o appl_td.o appl_process.o applserver.o  
	$(CCLINK) -L$(HOME)/lib -lmobile2g -L$(HOME)/lib -lcgprotcl -lhiredis

gprsserv:appl_redis.o cJSON.o appl_util.o appl_qryandset.o appl_alarm.o appl_gprs.o appl_td.o appl_process.o gprsserver.o  
	$(CCLINK) -L$(HOME)/lib -lmobile2g -L$(HOME)/lib -lcgprotcl -lhiredis
	
snmpserv:appl_util.o appl_qryandset.o appl_alarm.o appl_gprs.o appl_td.o appl_process.o snmpserver.o  
	$(CCLINK) -L$(HOME)/lib -lmobile2g -L$(HOME)/lib -lcgprotcl

hbserv:appl_redis.o cJSON.o appl_util.o appl_qryandset.o appl_alarm.o appl_gprs.o appl_td.o appl_process.o hbserv.o
	$(CCLINK) -L$(HOME)/lib -lev -lm -lmobile2g -L$(HOME)/lib -lcgprotcl -lhiredis -lpthread
