Name: connman-ui
Summary: A full-featured GTK based trayicon UI for ConnMan
Group: System Environment/Networking
Version: 0.3
License: GPL
URL: https://github.com/tbursztyka/connman-ui
Release: b1
Source0: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
#BuildArchitectures: i686
BuildRequires: automake, autoconf, intltool, libtool, glib2-devel, gtk3-devel
#BuildRequires: libmesagl-devel

%description
Description: %{summary}

%prep
%setup -q

%build
CMAKE_INSTALL_PREFIX=/usr cmake .
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p %{buildroot}%{_datadir}/applications
cp -a connman-ui.desktop %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}/etc/xdg/autostart
#cp -a connman-ui.desktop %{buildroot}/etc/xdg/autostart
ln -s %{_datadir}/applications/connman-ui.desktop %{buildroot}/etc/xdg/autostart

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/connman-ui-gtk
%{_datadir}/connman_ui_gtk/*
%{_datadir}/applications/connman-ui.desktop
/etc/xdg/autostart/connman-ui.desktop


%changelog
* Thu Dec 25 2014 Yuichiro Nakada <berry@berry-lab.net>
- Create for Berry Linux
