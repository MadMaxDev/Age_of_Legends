####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : Makefile_Prj.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : ��Թ��̵�ͨ��make�ļ�
## CreationDate : 2006-04-04
## ChangeLog    : 2006-06-16 �����ж����SRCFILE�ǿ���ö������Դ�ļ���
##              : 2007-01-17 ������ֱ��ָ��SRCFILE�ᵼ��tempĿ¼����ͨ��dirsĿ�괴��������
##              : 2007-04-02 ������MYDEPLIB_ABS��������������ֱ��д��Ҫ�Ŀ�ľ���·����
##              :            ����д�ĺô��ǿ��԰ѿ�Ҳд��������ϵ
##              : 2007-05-17 ����Prj-EXT.mk��Ϊ���ڲ�ͬ��ƽ̨�Ͽ�����һЩ��ͬ�����ã������в�����svn��ͳһ�޸�
####################################################################

####################################################################
# ��Щ����Ŀ��
####################################################################
.PHONY: dummy_help all dep obj cleanobj cleanint cleanout clean veryclean verycleanint install dirint dirout dirs

dummy_help:
# ��ӡhelp
	@echo Please input a specific target to make, like: clean dep all install ...

# ��ϵͳ��صĶ���
include $(PMKBASE)/cfg/cmn.mk
# ����·��
include $(PMKBASE)/cfg/paths.mk
# �Լ���Ҫ�ĸ���·��
SEARCHINCFILE	:= paths.mk
include $(PMKBASE)/cmn/whsearch_inc_file_$(SEARCHINCLVL).mk

# ��ǰ�����Լ���صĶ��������������ǰ���ϵͳinclude�ļ�֮��include����Ϊ��������õ�ǰ��Ķ��壩
# �����и������Ŀ¼���磺SRCDIR��IntDir��OutDir��
include $(PRJ_FILE)

# �������й��ܶ���Ҫ���ӵĶ���������һ��ͳһ��ϵͳ��
SEARCHINCFILE   := Prj-EXT.mk
include $(PMKBASE)/cmn/whsearch_inc_file_$(SEARCHINCLVL).mk

####################################################################
# ͨ��Դ�ļ�����dep�ļ��б�
####################################################################
# �������֮ǰ������SRCFILE�Ļ��Ͳ���ɨ�����е�Դ�ļ��ˣ����һ�����ڲ����Եĳ���
# ���ǻ�����Ҫ����ʱ�ļ�Ŀ¼�ȶ���ã���������mkxxx dirs������ʱĿ¼�ˣ�
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
# ����������
####################################################################
ifeq ($(ISWHAT),LIB)
# ��LIB�����
CMNDEP			:= 1
S_DEP			:= 1
ALLDEPFILE		:= $(DEPFILE) $(S_DEPFILE)
# �����gprof�Ͳ�����Ҫ�����DLL��Ϣ��ģ��
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
# ��MYDEPLIB�����ʵ���׺��MYDEPLIB��MYSYSLIBӦ������Prj.mk�ж���ģ�
ifneq ($(MYDEPLIB_ABS),)
# �ֶ���д�˾��Կ����֡����Ϻ�׺����
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
# �ǿ�ִ�г�������
# ����
all: $(TargetOutFile)

# ������Ҫ����DLL�ĳ��򣬾Ͳ�����gprof�ķ�ʽ�����루�����ǲ��ܱ���ͨ���ģ�����ʱ�ᱨ����dll��صĺ���û�ж��壩
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
# ��DLL�����
# ����
all: $(TargetOutFile).so

# �����gprof�Ͳ��ܱ���DLL
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
# �Ƴ�����Ŀ���ļ�
	$(RM) -f $(OBJFILE_TO_DEL)

cleanint: cleanobj
# �Ƴ�������ʱ�ļ�(Ŀ���ļ��������ļ�)
	$(RM) -f $(DEPFILE_TO_DEL)

cleanout:
ifeq ($(ISWHAT),LIB)
# ��
	$(RM) -f $(TargetOutFile).*
else
ifeq ($(ISWHAT),EXE)
# EXE�ļ�
	$(RM) -f $(TargetOutFile)
else
# DLL������
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
# ����Ŀ¼
#
# ��ʱĿ¼
dirint:
ifneq ($(INTDIRS),)
ifeq ($(wildcard $(INTDIRS)),)
	$(MKDIR) -p $(INTDIRS)
endif
endif

# ������Ŀ¼
dirout:
ifneq ($(OutDir),)
ifeq ($(wildcard $(OutDir)),)
	$(MKDIR) -p $(OutDir)
endif
endif

# ����Ŀ¼
dirs: dirint dirout

# ����dep�ļ���������ϵ������(����dirint������ȷ����ʱĿ¼�Ǵ��ڵ�)
# ע������.d�ļ�ֻ������.$(CPPEXT)�ļ������������.h�ļ���include���ݷ����˸ı䣬����������make dep
# �����LIB��EXE����Ҫ��ͨdep
# �����LIB��DLL����Ҫ��ͨs.dep

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
# �������д��ڵ�dep�ļ���û�в�����
####################################################################
GOOD_DEPFILE := $(wildcard $(DEPFILE))
ifneq ($(GOOD_DEPFILE),)
include $(GOOD_DEPFILE)
endif

GOOD_S_DEPFILE := $(wildcard $(S_DEPFILE))
ifneq ($(GOOD_S_DEPFILE),)
include $(GOOD_S_DEPFILE)
endif

