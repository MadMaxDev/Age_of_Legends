#!/bin/sh

####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whsurecp.sh
## Creator      : Wei Hua (κ��) weihua@pixelgame.net
## Comment      : �����ļ���ȷ��Ŀ��Ŀ¼���ڡ�
##              : ������cp���ơ��������Ŀ����'/'��β��˵��Ŀ����һ��Ŀ¼���������ļ�������Ҫ�����Ŀ�������
## CreationDate : 2006-04-06
## ChangeLog    :
####################################################################

# �ж�Ŀ���Ƿ����ļ������Ŀ��Ŀ¼
# �����滻����˼�ǰѴ���β����ĵ�һ��'/'��֮��Ķ���ȫ���滻Ϊ��
DSTDIR=`echo $2 | sed -e 's/\/[^\/]*$//'`
# �ж�Ŀ��Ŀ¼�Ƿ���ڣ������ھʹ���֮
if [ ! -d $DSTDIR ]
then
	mkdir -p $DSTDIR
fi
# �����ļ�
echo surecopying $1 to $2 ...
cp -f $1 $2
