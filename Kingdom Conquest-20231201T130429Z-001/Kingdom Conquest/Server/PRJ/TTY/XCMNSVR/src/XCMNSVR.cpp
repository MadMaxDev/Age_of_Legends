// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : 无
// File         : XCMNSVR.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 通用服务器框架
// CreationDate : 2005-09-20
// ChangeLog    :
//                2005-09-20 定版为V0.0001。从XGMS2.cpp继承过来。
//                2005-11-15 升级为V0.0002。增加UDPEvent以迅速知道程序被关闭了。
//                2005-12-02 升级为V0.0003。转移到VC2003.net。
//                2006-04-07 升级为V0.0004。在打不开配置文件的时候报错，而不是打印帮助。
//                2006-08-22 升级为V0.0005。增加了文件管理器。
//                2007-03-02 升级为V0.0006。增加了windows下的调用栈打印。
//                2007-05-18 升级为V0.0007。把epoll编入主程序，因为我现在编译的so不能自动吧epoll代码连入。
//                2007-06-12 升级为V0.0008。增了了pid文件的相关设置。并可以通过-stop停止服务器、-status获得服务器运行状态。
//                2007-06-13 修正了任何时候正常启动都报Old pid file deleted的问题。
//                2007-07-19 升级为V0.0009。增加在-stop的时候等待进程结束再退出。
//                2007-08-21 升级为V0.0010。把文件查找顺序顺序改为优先读目录再读取包中数据。
//                2007-08-31 增加了-kill参数。
//                2007-12-10 改为调用库中的函数

#include <PNGS/inc/XCMNSVR_Lib.h>

int	main(int argc, char *argv[])
{
	return	n_pngs::main_dll(argc, argv);
}
