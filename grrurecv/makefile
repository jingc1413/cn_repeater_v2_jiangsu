#
# GRRU�������ű�
#
include $(HOME)/make.dir/makefile.std

all:grrurecv

grrurecv:grrurecv.o cJSON.o
	$(CCLINK) -L$(HOME)/lib -lmobile2g -lhiredis

