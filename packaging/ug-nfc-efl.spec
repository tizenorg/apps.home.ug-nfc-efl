%define _usrdir	/usr
%define _ugdir	%{_usrdir}/ug


Name:       ug-nfc-efl
Summary:    UI gadget about the nfc
Version:    0.1.0
Release:    0
Group:      TO_BE/FILLED_IN
License:    Flora Software License
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(efl-assist)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-nfc)
BuildRequires:  pkgconfig(capi-content-mime-type)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(notification)

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools

Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/vconftool
Requires(postun): /sbin/ldconfig

%description
UI gadget about the nfc

%prep
%setup -q

%package devel
Summary:  ug for nfc setting
Group:    Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
ug for nfc setting

%build
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
mkdir cmake_tmp
cd cmake_tmp
cmake .. -DCMAKE_INSTALL_PREFIX=%{_ugdir}

make %{?jobs:-j%jobs}

%install
cd cmake_tmp
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
cp -af %{_builddir}/%{name}-%{version}/LICENSE %{buildroot}/usr/share/license/

%post
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client /usr/ug/bin/setting-nfc-efl

vconftool set -t bool -f db/private/ug-nfc-setting-efl/first_time_secure_storage_popup_show 1 -u 5000 -s system::vconf_network

%postun

%files
%manifest ug-nfc-efl.manifest
%defattr(-,root,root,-)
/usr/ug/lib/libug-setting-nfc-efl*
/usr/ug/res/edje/ug-setting-nfc-efl/*.edj
/usr/ug/res/locale/*/LC_MESSAGES/ug-setting-nfc-efl*
/usr/ug/res/icons/*
/usr/share/license/LICENSE
/usr/share/packages/ug-setting-nfc-efl.xml
