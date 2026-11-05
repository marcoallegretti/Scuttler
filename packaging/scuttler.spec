#
# spec file for package scuttler
#
# SPDX-FileCopyrightText: 2026 Marco Allegretti
# SPDX-License-Identifier: EUPL-1.2
#

Name:           scuttler
Version:        0.1.0
Release:        0
Summary:        Qt WebEngine shell for the Agama installer web UI
License:        BSD-3-Clause AND EUPL-1.2
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake >= 3.21
BuildRequires:  gcc-c++
BuildRequires:  pam-devel
BuildRequires:  qt6-declarative-tools
BuildRequires:  systemd-rpm-macros
BuildRequires:  sysuser-tools
BuildRequires:  cmake(Qt6Core) >= 6.11
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6QmlTools)
BuildRequires:  cmake(Qt6Quick)
BuildRequires:  cmake(Qt6Test)
BuildRequires:  cmake(Qt6WebEngineQuick)
Requires:       qt6-declarative-imports
Requires:       qt6-wayland
Requires:       qt6-webengine-imports

%description
Scuttler shows the web user interface of the Agama installer in a
Qt WebEngine view, logging in with the installer token.

%package kiosk
Summary:        Agama installer web UI session
License:        EUPL-1.2
BuildArch:      noarch
Requires:       %{name} = %{version}-%{release}
Requires:       agama
Requires:       breeze6-cursors
Requires:       kwin6
# QTBUG-147154
Requires:       xwayland
%sysusers_requires
%{?systemd_requires}

%description kiosk
Runs Scuttler as an unprivileged user in a KWin Wayland session on the
first virtual terminal, as the display manager of an Agama installer image.
KWin follows the keyboard layout that Agama sets through localed.

%prep
%autosetup

%build
%sysusers_generate_pre session/scuttler.sysusers scuttler scuttler.conf
%cmake
%cmake_build

%install
%cmake_install
install -Dm0644 session/scuttler.service %{buildroot}%{_unitdir}/scuttler.service
install -Dm0644 session/scuttler.pam %{buildroot}%{_pam_vendordir}/scuttler
install -Dm0644 session/scuttler.sysusers %{buildroot}%{_sysusersdir}/scuttler.conf
install -Dm0644 session/kdeglobals %{buildroot}%{_datadir}/scuttler/xdg/kdeglobals

%check
%ctest

%pre kiosk -f scuttler.pre
%service_add_pre scuttler.service

%post kiosk
%service_add_post scuttler.service

%preun kiosk
%service_del_preun scuttler.service

%postun kiosk
%service_del_postun_without_restart scuttler.service

%files
%license LICENSES/EUPL-1.2.txt LICENSES/BSD-3-Clause.txt
%doc README.md
%{_bindir}/scuttler

%files kiosk
%{_unitdir}/scuttler.service
%{_pam_vendordir}/scuttler
%{_sysusersdir}/scuttler.conf
%{_datadir}/scuttler

%changelog
