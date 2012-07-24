%define _optdir	/opt
%define _ugdir	%{_optdir}/ug


Name:       ug-nfc-efl
Summary:    UI gadget about the nfc
Version:    0.0.1
Release:    1
Group:      TO_BE/FILLED_IN
License:    Flora Software License
Source0:    %{name}-%{version}.tar.gz
Source1:    libug-setting-nfc-efl.install.in
Source2:    libug-share-nfc-efl.install.in
Source1001: packaging/ug-nfc-efl.manifest 
BuildRequires: cmake
BuildRequires: edje-tools
BuildRequires: gettext-tools
BuildRequires: pkgconfig(nfc-common-lib)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(contacts-service)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(ui-gadget)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(bluetooth-api)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(syspopup-caller)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(capi-network-nfc)
BuildRequires: pkgconfig(nfc)
BuildRequires: pkgconfig(utilX)
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
Requires(post):   /sbin/ldconfig
Requires(post): /usr/bin/vconftool
Requires: %{name} = %{version}-%{release}

%description -n ug-share-nfc-efl
ug for nfc share


%build
cp %{SOURCE1001} .
mkdir cmake_tmp
cd cmake_tmp
cmake .. -DCMAKE_INSTALL_PREFIX=%{_ugdir}

make %{?jobs:-j%jobs}


%install
cd cmake_tmp
rm -rf %{buildroot}
%make_install

%post
vconftool set -t bool db/nfc/enable 0 -u 5000


%post -n ug-share-nfc-efl
vconftool set -t int db/nfc/last_file_number 0 -u 5000

%postun

%files
%manifest ug-nfc-efl.manifest
%defattr(-,root,root,-)
/opt/ug/lib/libug-setting-nfc-efl*
/opt/ug/res/locale/*/LC_MESSAGES/ug-setting-nfc-efl*
/opt/ug/res/icons/*

%files -n ug-share-nfc-efl
%manifest ug-nfc-efl.manifest
%defattr(-,root,root,-)
/opt/ug/lib/libug-share-nfc-efl*
/opt/ug/res/edje/*
/opt/ug/res/images/*
/opt/ug/res/locale/*/LC_MESSAGES/ug-share-nfc-efl*
