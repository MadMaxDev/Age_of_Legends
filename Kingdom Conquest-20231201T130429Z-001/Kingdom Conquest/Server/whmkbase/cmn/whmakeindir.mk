####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whmakeindir.mk
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : ���$PRJS�е�ÿ��Ŀ¼����ĳ��make
## CreationDate : 2006-04-04
## ChangeLog    : 
####################################################################

# 
$(TARGET):
# ÿ��dir�ж�makeһ�¡����������ǰ����ӡһЩ˵������ע�⣺ע��һ��Ҫ������ͷд������ᱻ��Ϊ��ָ����ӡ������
	@for dir in $(PRJS); do		\
		echo "** IN DIR:$$dir, make $@ with Makefile_$(ConfigurationName).mk $@ **";	\
		$(MAKE) -C $$dir -f $(PMKBASE)/Makefile_$(ConfigurationName).mk $@;		\
		echo;					\
	done

