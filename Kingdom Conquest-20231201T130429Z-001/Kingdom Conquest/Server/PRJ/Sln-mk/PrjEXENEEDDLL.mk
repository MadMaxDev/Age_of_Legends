# ISWHAT��ȡֵ�������£�LIB EXE DLL
ISWHAT		:= EXE
EXENEEDDLL	:= 1
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR		:= ./src
# ��temp��outĿ¼�ж�һ���ϲ�Ŀ¼
UPPERDIR	:= $(notdir $(patsubst %/,%,$(dir $(CURDIR))))
IntDir		:= ../../temp/$(ConfigurationName)/$(UPPERDIR)/$(ProjectName)
OutDir		:= ../../bin/$(ConfigurationName)/$(UPPERDIR)/$(ProjectName)
