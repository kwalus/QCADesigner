Summary: A QCA design and simulation tool
Name: QCADesigner
Version: 1.4.1
Release: 1
Copyright: GPL
Group: Development/Tools
Source: http://qcadesigner.ca/QCADesigner/releases/1.4.0/QCADesigner-1.4.0-src.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot
Vendor: Konrad Walus <walus@atips.ca> University of Calgary
Packager: Gabriel Schulhof <schulhof@atips.ca> University of Calgary
Requires: gtk2 >= 2.0

%description
QCADesigner allows you to create QCA-based circuit designs whose function
you can then simulate with the various built-in simulation engines. It is
a full-featured design tool, and it comes with three different simulation 
engines.

%prep
%setup -q

%build
CFLAGS="%optflags" CXXFLAGS="%optflags" ./autogen.sh --prefix=/usr
make

%install
make DESTDIR=%buildroot install
[ "/" != %buildroot ] && rm -rf %buildroot/usr/share/doc

%clean
[ "/" != $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog CurrentVerChangeLog INSTALL README TODO
%doc docs/manual

/usr/bin/QCADesigner
/usr/share

%changelog
* Sat Oct 11 2003 Gabriel Schulhof <schulhof@atips.ca> 
- added RPM support
