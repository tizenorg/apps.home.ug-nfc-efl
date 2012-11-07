%define _optdir	/opt
%define _ugdir	%{_optdir}/ug


Name:       ug-nfc-efl
Summary:    UI gadget about the nfc
Version:    0.0.2
Release:    21
Group:      TO_BE/FILLED_IN
License:    Samsung Proprietary License
Source0:    %{name}-%{version}.tar.gz
Source1:    libug-setting-nfc-efl.install.in
Source2:    libug-share-nfc-efl.install.in
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(status)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-nfc)
BuildRequires:  pkgconfig(capi-content-mime-type)

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

%package -n ug-share-nfc-efl
Summary:  ug for nfc share
Group:    Development/Libraries
Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/vconftool
Requires: %{name} = %{version}-%{release}

%description -n ug-share-nfc-efl
ug for nfc share


%build
mkdir cmake_tmp
cd cmake_tmp
cmake .. -DCMAKE_INSTALL_PREFIX=%{_ugdir}

make %{?jobs:-j%jobs}


%install
cd cmake_tmp
rm -rf %{buildroot}
%make_install

%post


%post -n ug-share-nfc-efl
vconftool set -t int -f db/private/ug-nfc-efl/last_file_number 0 -u 5000

%postun

%files
%manifest ug-nfc-efl.manifest
%defattr(-,root,root,-)
/opt/ug/lib/libug-setting-nfc-efl*
/opt/ug/res/locale/*/LC_MESSAGES/ug-setting-nfc-efl*
/opt/ug/res/icons/*

%files -n ug-share-nfc-efl
%defattr(-,root,root,-)
/opt/ug/lib/libug-share-nfc-efl*
/opt/ug/res/edje/*
/opt/ug/res/images/*
/opt/ug/res/locale/*/LC_MESSAGES/ug-share-nfc-efl*
