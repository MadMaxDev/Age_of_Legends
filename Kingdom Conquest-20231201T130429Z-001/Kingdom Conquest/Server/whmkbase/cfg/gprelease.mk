# 是带gprof的模式
GPUSED					:= 1
# 编译模式名
ConfigurationName		:= gprelease
# 用于体现该编译模式的文件标记
# (对于debug版来说，比如目标文件后缀就会变成._do)
CfgFileTag				:= _p
# 编译选项
CCFlag					:= -O2 -Wall -pg -static -D_DEBUG -Wno-strict-aliasing
#CCFlag					:= -finput-charset=gb2312 -O2 -Wall -pg -static -D_DEBUG -Wno-strict-aliasing
# 连接选项
LNFlag					:= -lpthread
