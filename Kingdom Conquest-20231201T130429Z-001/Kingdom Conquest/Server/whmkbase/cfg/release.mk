# 编译模式名
ConfigurationName		:= release
# 用于体现该编译模式的文件标记
# (对于debug版来说，比如目标文件后缀就会变成._do，而release版就是.o)
CfgFileTag				:=
# 编译选项
CCFlag					:= -Wall -O2 -g -D_DEBUG -Wno-strict-aliasing
#CCFlag					:= -finput-charset=gb2312 -Wall -O2 -DNDEBUG -Wno-strict-aliasing
# 连接选项
LNFlag					:= -lpthread
