# ISWHAT��ȡֵ�������£�LIB EXE DLL
ISWHAT		:= LIB
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR	:= ./src
# ��temp�ж�һ���ϲ�Ŀ¼
UPPERDIR	:= $(notdir $(patsubst %/,%,$(dir $(CURDIR))))
IntDir		:= ../../temp/$(ConfigurationName)/$(UPPERDIR)/$(ProjectName)
OutDir		:= ../../lib
