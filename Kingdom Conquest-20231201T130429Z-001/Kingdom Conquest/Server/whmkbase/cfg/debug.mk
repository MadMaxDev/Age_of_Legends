# 编译模式名
ConfigurationName		:= debug
# 用于体现该编译模式的文件标记
# (对于debug版来说，比如目标文件后缀就会变成._do)
CfgFileTag				:= _d
# 编译选项
CCFlag					:= -Wall -g -D_DEBUG -Wno-unused-variable -Wno-sign-compare
#CCFlag					:= -finput-charset=gb2312 -Wall -g -D_DEBUG
# 连接选项
LNFlag					:= -lpthread
