# 是带gprof的模式
GPUSED					:= 1
# 编译模式名
ConfigurationName		:= gpdebug
# 用于体现该编译模式的文件标记
# (对于debug版来说，比如目标文件后缀就会变成._do)
CfgFileTag				:= _pd
# 编译选项
CCFlag					:= -Wall -g -pg -static -D_DEBUG
#CCFlag					:= -finput-charset=gb2312 -Wall -g -pg -static -D_DEBUG
# 连接选项
LNFlag					:= -lpthread
