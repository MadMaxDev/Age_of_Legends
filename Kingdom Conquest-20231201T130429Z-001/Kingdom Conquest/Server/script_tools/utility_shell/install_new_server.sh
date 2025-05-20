#!/bin/sh
yum -y install mysql
yum -y install mysql-server
yum -y install httpd
yum -y install php
yum -y install php-mysql
yum -y install vim-enhanced

# 在32位系统上运行及编译32位程序
yum -y groupinstall "Development tools"  
yum -y groupinstall "Compatibility libraries"

yum -y install glibc-devel.i686
yum -y install libstdc++-devel.i686
yum -y install ncurses-devel.i686
yum -y install ncurses-devel
yum -y install zlib-devel.i686

ln -s /lib/libz.so.1.2.3 /usr/lib/libz.so

iptables -A INPUT -i eth0 -p tcp --dport 3306 -j DROP

# 开机开启防火墙
echo "iptables -A INPUT -i eth0 -p tcp --dport 3306 -j DROP" >> /etc/rc.d/rc.local
