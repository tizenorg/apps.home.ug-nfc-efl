%define _usrdir	/usr
%define _ugdir	%{_usrdir}/ug

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

Name:       ug-nfc-efl
Summary:    UI gadget about the nfc
Version:    0.1.0
Release:    0
Group:      TO_BE/FILLED_IN
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(efl-extension)
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
mkdir -p %{buildroot}/etc/config/nfc/
cp -af %{_builddir}/%{name}-%{version}/LICENSE %{buildroot}/usr/share/license/

%post
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client /usr/ug/bin/setting-nfc-efl

%postun

%files
%defattr(-,root,root,-)
/usr/ug/lib/libug-setting-nfc-efl*
/usr/ug/res/edje/ug-setting-nfc-efl/*.edj
/usr/ug/res/locale/*/LC_MESSAGES/ug-setting-nfc-efl*
/usr/ug/res/icons/*
/etc/config/nfc/*
/usr/share/license/LICENSE
