####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whmakeindir.mk
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 针对$PRJS中的每个目录进行某种make
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

# 
$(TARGET):
# 每个dir中都make一下。另外在输出前后多打印一些说明。（注意：注释一定要顶着行头写，否则会被人为是指令。会打印出来）
	@for dir in $(PRJS); do		\
		echo "** IN DIR:$$dir, make $@ with Makefile_$(ConfigurationName).mk $@ **";	\
		$(MAKE) -C $$dir -f $(PMKBASE)/Makefile_$(ConfigurationName).mk $@;		\
		echo;					\
	done

