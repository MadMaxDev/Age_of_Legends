# ISWHAT��ȡֵ�������£�LIB EXE DLL
ISWHAT		:= LIB
CURDIR		:= $(shell pwd)
ProjectName	:= $(notdir $(CURDIR))
SRCDIR		:= .
IntDir		:= ../temp/$(ConfigurationName)/$(ProjectName)
OutDir		:= ../lib
# ��-Wallȥ��(��Ϊcryptlib��warning̫����)
CCFlag		:= $(filter-out -Wall,$(CCFlag)) -msse2
#CCFlag		:= $(filter-out -Wall,$(CCFlag))

