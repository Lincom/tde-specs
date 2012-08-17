#
# spec file for package lame (Version 3.89beta)
# 
# Copyright  (c)  2001  SuSE GmbH  Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
# 
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  autoconf automake
# usedforbuild    aaa_base aaa_dir autoconf automake base bash bindutil binutils bison bzip compress cpio cpp cracklib cyrus-sasl db devs diffutils e2fsprogs file fileutils findutils flex gawk gcc gdbm gdbm-devel gettext glibc glibc-devel gpm gppshare grep groff gzip kbd less libtool libz m4 make man mktemp modutils ncurses ncurses-devel net-tools netcfg pam pam-devel patch perl ps rcs readline rpm sendmail sh-utils shadow strace syslogd sysvinit texinfo textutils timezone unzip util-linux vim

# Commandline:  -c --enable-mp3rtp -c --enable-mp3x -c --mandir=/usr/share/man/
Name:         lame
Copyright:    GPL
Group:        Applications/Multimedia
Summary:      Free MP3 encoder
Version:      3.93.1
Release:      0
Source0:      lame-%{version}.tar.gz
BuildRoot:    /var/tmp/%{name}-buildroot

%description
free mp3 encoder

uses patented algorithm by Fraunhofer.

Authors:
--------
    Mike Cheng
    Mark Taylor

SuSE series: suse

%prep
%setup -n lame-%{version}

%build
CXXFLAGS="$CXXFLAGS $RPM_OPT_FLAGS -DNDEBUG " ./configure \
  --prefix=/usr \
  --enable-mp3rtp \
  --enable-mp3x \
  --mandir=/usr/share/man/ \
  
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files
%doc TODO Makefile.MSVC USAGE LICENSE README PRESETS.draft HACKING README.DJGPP README.B32 INSTALL.configure INSTALL README.WINGTK ChangeLog Makefile.DJGPP DEFINES COPYING STYLEGUIDE 
%dir /usr/include/lame
%dir /usr/share/doc/lame
%dir /usr/share/doc/lame/html
/usr/bin/lame
/usr/bin/mp3rtp
/usr/include/lame/lame.h
/usr/lib/libmp3lame.so*
/usr/lib/libmp3lame.la
/usr/lib/libmp3lame.a
/usr/share/doc/lame/html/presets.html
/usr/share/doc/lame/html/basic.html
/usr/share/doc/lame/html/contributors.html
/usr/share/doc/lame/html/examples.html
/usr/share/doc/lame/html/history.html
/usr/share/doc/lame/html/id3.html
/usr/share/doc/lame/html/index.html
/usr/share/doc/lame/html/lame.css
/usr/share/doc/lame/html/modes.html
/usr/share/doc/lame/html/node6.html
/usr/share/doc/lame/html/switchs.html
/%{_mandir}/man1/lame.1.gz

