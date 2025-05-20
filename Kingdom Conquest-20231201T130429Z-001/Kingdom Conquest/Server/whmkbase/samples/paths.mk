# 这个文件放在工程目录下或者其上的某及目录，工程make从其目录向上开始找，找到哪个就包含哪个
# 公共inc目录
# 注意不能用~代替用户根目录，这个路径gcc不认。
WHTEMPVAR	:= 3RD PRJ/ENGINE PRJ/GAME PRJ/TTY
WHTEMPVAR	:= $(addprefix -I$(HOME)/WORK/,$(WHTEMPVAR))
INCPATH		:= $(INCPATH) $(WHTEMPVAR)
# 公共lib目录
WHTEMPVAR	:= 3RD/lib PRJ/lib
WHTEMPVAR	:= $(addprefix -L$(HOME)/WORK/,$(WHTEMPVAR))
LIBPATH		:= $(LIBPATH) $(WHTEMPVAR)

# 安装根目录
INSTALLROOT	:= /ITC/GameSvr0

