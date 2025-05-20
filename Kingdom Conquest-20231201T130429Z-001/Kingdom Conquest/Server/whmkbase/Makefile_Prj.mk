####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : Makefile_Prj.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 针对工程的通用make文件
## CreationDate : 2006-04-04
## ChangeLog    : 2006-06-16 增加判断如果SRCFILE非空则不枚举所有源文件了
##              : 2007-01-17 修正了直接指定SRCFILE会导致temp目录不能通过dirs目标创建的问题
##              : 2007-04-02 增加了MYDEPLIB_ABS变量，可以用于直接写需要的库的绝对路径。
##              :            这样写的好处是可以把库也写入依赖关系
##              : 2007-05-17 增加Prj-EXT.mk，为了在不同的平台上可能有一些不同的配置，但是有不想在svn上统一修改
####################################################################

####################################################################
# 这些是虚目标
####################################################################
.PHONY: dummy_help all dep obj cleanobj cleanint cleanout clean veryclean verycleanint install dirint dirout dirs

dummy_help:
# 打印help
	@echo Please input a specific target to make, like: clean dep all install ...

# 本系统相关的定义
include $(PMKBASE)/cfg/cmn.mk
# 公共路径
include $(PMKBASE)/cfg/paths.mk
# 自己需要的附加路径
SEARCHINCFILE	:= paths.mk
include $(PMKBASE)/cmn/whsearch_inc_file_$(SEARCHINCLVL).mk

# 当前工程自己相关的东西（这个必须在前面的系统include文件之后include，因为里面可能用到前面的定义）
# 里面有各个相关目录，如：SRCDIR、IntDir、OutDir等
include $(PRJ_FILE)

# 可能所有功能都需要附加的东西，比如一个统一的系统库
SEARCHINCFILE   := Prj-EXT.mk
include $(PMKBASE)/cmn/whsearch_inc_file_$(SEARCHINCLVL).mk

####################################################################
# 通过源文件生成dep文件列表
####################################################################
# 如果在这之前定义了SRCFILE的话就不用扫描所有的源文件了（这个一般用于测试性的程序）
# 但是还是需要把临时文件目录先定义好（否则不能用mkxxx dirs创建临时目录了）
INTDIRS			:= $(IntDir)
ifeq ($(SRCFILE),)
ifneq ($(MULTISRCDIR),)
SRCFILE			:= $(foreach sfd,$(MULTISRCDIR),$(wildcard $(sfd)/*.$(CPPEXT)))
OBJFILE_TO_DEL	:= $(patsubst $(SRCDIR)/%,$(IntDir)/%,$(foreach sfd,$(MULTISRCDIR),$(sfd)/*.o$(CfgFileTag)))
DEPFILE_TO_DEL	:= $(patsubst $(SRCDIR)/%,$(IntDir)/%,$(foreach sfd,$(MULTISRCDIR),$(sfd)/*.d$(CfgFileTag)))
INTDIRS			:= $(patsubst $(SRCDIR)/%,$(IntDir)/%,$(foreach sfd,$(MULTISRCDIR),$(sfd)))
else
SRCFILE			:= $(wildcard $(SRCDIR)/*.$(CPPEXT))
OBJFILE_TO_DEL	:= $(IntDir)/*.o$(CfgFileTag)
DEPFILE_TO_DEL	:= $(IntDir)/*.d$(CfgFileTag)
endif
endif
OBJFILE			:= $(patsubst $(SRCDIR)/%.$(CPPEXT),$(IntDir)/%.o$(CfgFileTag),$(SRCFILE))
DEPFILE			:= $(patsubst $(SRCDIR)/%.$(CPPEXT),$(IntDir)/%.d$(CfgFileTag),$(SRCFILE))
S_OBJFILE		:= $(patsubst $(SRCDIR)/%.$(CPPEXT),$(IntDir)/%.s.o$(CfgFileTag),$(SRCFILE))
S_DEPFILE		:= $(patsubst $(SRCDIR)/%.$(CPPEXT),$(IntDir)/%.s.d$(CfgFileTag),$(SRCFILE))

ifeq ($(ISWHAT),LIB)
TargetOutFile	:= $(OutDir)/lib$(ProjectName)$(CfgFileTag)
else
TargetOutFile	:= $(OutDir)/$(ProjectName)$(CfgFileTag)
endif

####################################################################
# 连接输出结果
####################################################################
ifeq ($(ISWHAT),LIB)
# 是LIB的输出
CMNDEP			:= 1
S_DEP			:= 1
ALLDEPFILE		:= $(DEPFILE) $(S_DEPFILE)
# 如果有gprof就不能需要编译带DLL信息的模块
ifneq ($(GPUSED),)
all: $(TargetOutFile).a
else
all: $(TargetOutFile).a $(TargetOutFile).s.a
endif

$(TargetOutFile).a: $(OBJFILE)
	@echo ==== MAKING lib FOR common use ... ====
	$(AR) $(ARFlag) $@ $(OBJFILE)
	$(RANLIB) $@

$(TargetOutFile).s.a: $(S_OBJFILE)
	@echo ==== MAKING lib FOR DLL use ... ====
	$(AR) $(ARFlag) $@ $(S_OBJFILE)
	$(RANLIB) $@

else
# 给MYDEPLIB加上适当后缀（MYDEPLIB和MYSYSLIB应该是在Prj.mk中定义的）
ifneq ($(MYDEPLIB_ABS),)
# 手动填写了绝对库名字。加上后缀即可
ifeq ($(ISWHAT),EXE)
MYDEPLIB_ABS:= $(addsuffix $(CfgFileTag).a,$(MYDEPLIB_ABS))
else
ifeq ($(ISWHAT),DLL)
MYDEPLIB_ABS:= $(addsuffix $(CfgFileTag).s.a,$(MYDEPLIB_ABS))
endif
endif
endif
ifneq ($(MYDEPLIB),)
MYDEPLIB	:= $(addsuffix $(CfgFileTag),$(MYDEPLIB))
MYDEPLIB	:= $(addprefix -l,$(MYDEPLIB))
endif
ifeq ($(ISWHAT),EXE)
CMNDEP			:= 1
S_DEP			:= 0
ALLDEPFILE		:= $(DEPFILE)
# 是可执行程序的输出
# 连接
all: $(TargetOutFile)

# 对于需要载入DLL的程序，就不能用gprof的方式来编译（否则是不能编译通过的，连接时会报各种dll相关的函数没有定义）
ifneq ($(EXENEEDDLL),)
ifeq ($(GPUSED),)
$(TargetOutFile): $(OBJFILE) $(MYDEPLIB_ABS)
	@echo ==== making EXE file ... ====
	$(CC) $(PMKDEFINEs) $(CCFlag) -rdynamic -ldl -o $@ $(OBJFILE) $(MYDEPLIB_ABS) $(MYDEPLIB) $(MYSYSLIB) $(LNFlag) $(LIBPATH)
else
$(TargetOutFile):
	@echo ==== This EXE file can not be made with gprof. Skipped ...  ====
endif
else
$(TargetOutFile): $(OBJFILE) $(MYDEPLIB_ABS)
	@echo ==== making EXE file ... ====
	$(CC) $(PMKDEFINEs) $(CCFlag) -o $@ $(OBJFILE) $(MYDEPLIB_ABS) $(MYDEPLIB) $(MYSYSLIB) $(LNFlag) $(LIBPATH)
endif

else
ifeq ($(ISWHAT),DLL)
CMNDEP			:= 0
S_DEP			:= 1
ALLDEPFILE		:= $(S_DEPFILE)
# 是DLL的输出
# 连接
all: $(TargetOutFile).so

# 如果有gprof就不能编译DLL
ifeq ($(GPUSED),)
$(TargetOutFile).so: $(S_OBJFILE) $(MYDEPLIB_ABS)
	@echo ==== making DLL file ... ====
	$(CC) $(PMKDEFINEs) $(CCFlag) -o $@ $(S_OBJFILE) $(MYDEPLIB_ABS) $(MYDEPLIB) $(MYSYSLIB) $(LNFlag) $(SOLNFlag) $(LIBPATH)
else
$(TargetOutFile).so:
	@echo ==== This DLL file can not be made with gprof. Skipped ...  ====
endif

else
	@echo !!!! make type is $$ISWHAT=$ISWHAT, NOT GOOD !!!!

endif
endif
endif

dep: $(ALLDEPFILE)
	@echo ==== dep make Done! ^_^ ====

cleanobj:
# 移除所有目标文件
	$(RM) -f $(OBJFILE_TO_DEL)

cleanint: cleanobj
# 移除所有临时文件(目标文件、依赖文件)
	$(RM) -f $(DEPFILE_TO_DEL)

cleanout:
ifeq ($(ISWHAT),LIB)
# 库
	$(RM) -f $(TargetOutFile).*
else
ifeq ($(ISWHAT),EXE)
# EXE文件
	$(RM) -f $(TargetOutFile)
else
# DLL或其他
	$(RM) -f $(TargetOutFile).*
endif
endif

clean: cleanint cleanout
	@echo ==== clean now ====

verycleanint:
	$(RM) -rf $(IntDir)
	@echo ==== int dir is very clean now ====

veryclean: verycleanint
	@echo ==== I am is very clean now ====

#
# 各个目录
#
# 临时目录
dirint:
ifneq ($(INTDIRS),)
ifeq ($(wildcard $(INTDIRS)),)
	$(MKDIR) -p $(INTDIRS)
endif
endif

# 结果输出目录
dirout:
ifneq ($(OutDir),)
ifeq ($(wildcard $(OutDir)),)
	$(MKDIR) -p $(OutDir)
endif
endif

# 所有目录
dirs: dirint dirout

# 各个dep文件的依赖关系和生成(依赖dirint是用来确保临时目录是存在的)
# 注意这里.d文件只依赖于.$(CPPEXT)文件，所以如果有.h文件的include内容发生了改变，请自行重新make dep
# 如果是LIB或EXE才需要普通dep
# 如果是LIB或DLL才需要普通s.dep

ifeq ($(CMNDEP),1)
$(IntDir)/%.d$(CfgFileTag): $(SRCDIR)/%.$(CPPEXT)
	$(CC) $(PMKDEFINEs) $(DEPFlag) "$(patsubst %.d$(CfgFileTag),%.o$(CfgFileTag),$@)" -MF $@ $< $(INCPATH)
	@echo "	$(CC) $(PMKDEFINEs) $(CCFlag) -c -o $(patsubst %.d$(CfgFileTag),%.o$(CfgFileTag),$@) $< $(INCPATH)" >> $@
endif

ifeq ($(S_DEP),1)
$(IntDir)/%.s.d$(CfgFileTag): $(SRCDIR)/%.$(CPPEXT)
	$(CC) $(PMKDEFINEs) $(DEPFlag) "$(patsubst %.s.d$(CfgFileTag),%.s.o$(CfgFileTag),$@)" -MF $@ $< $(INCPATH)
	@echo "	$(CC) $(PMKDEFINEs) $(CCFlag) $(SOCCFlag) -c -o $(patsubst %.s.d$(CfgFileTag),%.s.o$(CfgFileTag),$@) $< $(INCPATH)" >> $@
endif

####################################################################
# 包含所有存在的dep文件，没有不报错
####################################################################
GOOD_DEPFILE := $(wildcard $(DEPFILE))
ifneq ($(GOOD_DEPFILE),)
include $(GOOD_DEPFILE)
endif

GOOD_S_DEPFILE := $(wildcard $(S_DEPFILE))
ifneq ($(GOOD_S_DEPFILE),)
include $(GOOD_S_DEPFILE)
endif

