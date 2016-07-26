Name: mod_auth_grid
Version: 0.1
Release: 1
License: Unreleased
Group: Applications/Internet
Requires: httpd
BuildRequires: httpd-devel
Summary: Apache httpd module for secondary grid-mapfile authentication
Source: mod_auth_grid-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-root

%description
Apache httpd module for secondary grid-mapfile authentication.
For use with a primary authentication module (e.g. mod_auth_kerb) or with mod_ssl using the `SSLUserName' directive.

%prep
%setup

%build
cd src
make

%install
cd src
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/lib64/httpd/modules/mod_auth_grid.so
