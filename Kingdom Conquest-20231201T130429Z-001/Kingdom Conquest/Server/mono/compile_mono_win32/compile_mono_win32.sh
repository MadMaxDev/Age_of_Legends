1.依据dep/mono.def生成mono.lib文件
命令行:
	lib /nologo /def:mono.def /out:mono.lib /machine:x86
注意:
	lib.exe在Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin文件夹下
	同时需要将Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\mspdb100.dll拷入该文件夹下,生成lib文件之后将mspdb100.dll删除

2.将dll拷入目标文件夹
	即将XCombatSvr,lib拷入PRJ\bin\Debug\TTY文件夹