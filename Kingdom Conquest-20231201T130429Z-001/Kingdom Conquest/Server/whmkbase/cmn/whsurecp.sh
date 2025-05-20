#!/bin/sh

####################################################################
## Copyright (C) PixelGame, http://www.pixelgame.net
## All rights reserved.
## File         : whsurecp.sh
## Creator      : Wei Hua (魏华) weihua@pixelgame.net
## Comment      : 拷贝文件，确保目标目录存在。
##              : 参数和cp类似。不过如果目标以'/'结尾则说明目标是一个目录，否则是文件（即需要改名的拷贝）。
## CreationDate : 2006-04-06
## ChangeLog    :
####################################################################

# 判断目标是否是文件，获得目标目录
# 下面替换的意思是把从行尾向左的第一个'/'号之间的东西全部替换为空
DSTDIR=`echo $2 | sed -e 's/\/[^\/]*$//'`
# 判断目标目录是否存在，不存在就创建之
if [ ! -d $DSTDIR ]
then
	mkdir -p $DSTDIR
fi
# 拷贝文件
echo surecopying $1 to $2 ...
cp -f $1 $2
