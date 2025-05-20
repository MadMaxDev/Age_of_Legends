#!/bin/sh

# 安装monodevelop

# 新的yum源
wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm
rpm -Uvh epel-release-6-8.noarch.rpm

# 依赖项
yum -y install glib2-devel.i686
yum -y install libpng-devel.i686
yum -y install giflib-devel.i686 
yum -y install libtiff-devel.i686 
yum -y install libexif-devel.i686 
yum -y install libX11-devel.i686 
yum -y install fontconfig-devel.i686 
yum -y install gettext.i686

# 下载
wget http://download.mono-project.com/sources/mono/mono-3.0.0.tar.bz2

# 安装
export CFLAGS="-m32 -march=i386 -mtune=i386"
tar xvf mono-3.0.0.tar.bz2
cd mono-3.0.0
linux32 ./configure --prefix=/opt/mono
make
make install

# 设置环境变量
echo 'export PATH=$PATH:$HOME/bin:/opt/mono/bin' >> /etc/profile
# Linux环境变量名,该环境变量主要用于指定查找共享库(动态链接库)时除了默认路径之外的其他路径.(该路径在默认路径之前查找)
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/mono/lib' >> /etc/profile
# 重新导入
source /etc/profile
