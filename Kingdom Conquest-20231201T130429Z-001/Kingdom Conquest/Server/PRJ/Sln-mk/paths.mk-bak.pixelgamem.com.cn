# 这个文件放在工程目录下或者其上的某及目录，工程make从其目录向上开始找，找到哪个就包含哪个
# 公共inc目录
# 注意不能用~代替用户根目录，这个路径gcc不认。
VER_EXT		:= #_cur
WORKDIR		:= src$(VER_EXT)
PRJDIR		:= PRJ
#MYSQLLIBPATH:= /usr/local/mysql/lib
MYSQLLIBPATH:= /usr/local/mysql_x32/lib
WHTEMPVAR	:= 3RD 3RD/freetype221 $(PRJDIR)/ENGINE $(PRJDIR)/GAME $(PRJDIR)/TTY
WHTEMPVAR	:= $(addprefix -I$(HOME)/$(WORKDIR)/,$(WHTEMPVAR))
#INCPATH		:= -I/usr/local/mysql/include $(INCPATH) $(WHTEMPVAR)
INCPATH		:= -I/usr/local/mysql_x32/include $(INCPATH) $(WHTEMPVAR)
# 公共lib目录
WHTEMPVAR	:= 3RD/lib $(PRJDIR)/lib 
WHTEMPVAR	:= $(addprefix -L$(HOME)/$(WORKDIR)/,$(WHTEMPVAR))
LIBPATH		:= $(LIBPATH) $(WHTEMPVAR)

# 安装根目录
#INSTALLROOT	:= /ITC/GameSvr$(VER_EXT)/bin
#INSTALLROOT	:= /ITC_91tst/GameSvr$(VER_EXT)/bin
# 因为系统已经定义了HOME，而且之前又已经链接了bin目录，所以可以同意写成下面这样
INSTALLROOT	:= $(HOME)/bin

