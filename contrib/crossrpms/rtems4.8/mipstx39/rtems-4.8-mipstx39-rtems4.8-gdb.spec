#
# Please send bugfixes or comments to
# 	http://www.rtems.org/bugzilla
#

%define _prefix			/opt/rtems-4.8
%define _infodir		%{_prefix}/info
%define _mandir			%{_prefix}/man

%ifos cygwin cygwin32 mingw mingw32
%define _exeext .exe
%else
%define _exeext %{nil}
%endif

%define gdb_version 6.5
%define gdb_rpmvers %{expand:%(echo 6.5 | tr - _)} 

Name:		rtems-4.8-mipstx39-rtems4.8-gdb
Summary:	Gdb for target mipstx39-rtems4.8
Group:		Development/Tools
Version:	%{gdb_rpmvers}
Release:	1%{?dist}
License:	GPL/LGPL
URL: 		http://sources.redhat.com/gdb
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%if "%{gdb_version}" >= "6.6"
BuildRequires:	expat-devel
%endif
# Required for building the infos
BuildRequires:	/sbin/install-info
BuildRequires:	texinfo >= 4.2
%if "mipstx39-rtems4.8" == "sparc-rtems4.8"
BuildRequires:	libtermcap-devel
%endif
BuildRequires:	ncurses-devel

Requires:	rtems-4.8-gdb-common

Source0:	ftp://ftp.gnu.org/pub/gnu/gdb/gdb-%{gdb_version}.tar.bz2
%{?_without_sources:NoSource:	0}
%if "%{gdb_version}" == "6.5"
Patch0:		gdb-6.5-rtems-20060713.diff
%endif
%if "%{gdb_version}" == "6.6"
Patch0:		gdb-6.6-rtems4.8-20070218.diff
%endif

%description
GDB for target mipstx39-rtems4.8
%prep
%setup -q -c -T -n %{name}-%{version}

%setup -q -D -T -n %{name}-%{version} -a0
cd gdb-%{gdb_version}
%{?PATCH0:%patch0 -p1}
cd ..

%build
  mkdir -p build
  cd build
  CFLAGS="$RPM_OPT_FLAGS" \
  ../gdb-%{gdb_version}/configure \
    --build=%_build --host=%_host \
    --target=mipstx39-rtems4.8 \
    --verbose --disable-nls \
    --without-included-gettext \
    --disable-win32-registry \
    --disable-werror \
    --enable-sim \
    --prefix=%{_prefix} --bindir=%{_bindir} \
    --includedir=%{_includedir} --libdir=%{_libdir} \
    --mandir=%{_mandir} --infodir=%{_infodir}

  make all
  make info
  cd ..

%install
  rm -rf $RPM_BUILD_ROOT

  cd build
%if "%{gdb_version}" >= "6.3"
  make DESTDIR=$RPM_BUILD_ROOT install
%else
  make prefix=$RPM_BUILD_ROOT%{_prefix} \
    bindir=$RPM_BUILD_ROOT%{_bindir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    libdir=$RPM_BUILD_ROOT%{_libdir} \
    infodir=$RPM_BUILD_ROOT%{_infodir} \
    mandir=$RPM_BUILD_ROOT%{_mandir} \
    install

  make prefix=$RPM_BUILD_ROOT%{_prefix} \
    bindir=$RPM_BUILD_ROOT%{_bindir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    libdir=$RPM_BUILD_ROOT%{_libdir} \
    infodir=$RPM_BUILD_ROOT%{_infodir} \
    mandir=$RPM_BUILD_ROOT%{_mandir} \
    install-info
%endif

  rm -f $RPM_BUILD_ROOT%{_infodir}/dir
  touch $RPM_BUILD_ROOT%{_infodir}/dir

# These come from other packages
  rm -rf $RPM_BUILD_ROOT%{_infodir}/bfd*
  rm -rf $RPM_BUILD_ROOT%{_infodir}/configure*
  rm -rf $RPM_BUILD_ROOT%{_infodir}/standards*

# We don't ship host files
  rm -f ${RPM_BUILD_ROOT}%{_libdir}/libiberty*

%if "%{gdb_version}" >= "6.4"
# host library, installed to a bogus directory
  rm -f ${RPM_BUILD_ROOT}%{_libdir}/libmipstx39-rtems4.8-sim.a
%endif

  cd ..

# Extract %%__os_install_post into os_install_post~
cat << \EOF > os_install_post~
%__os_install_post
EOF

# Generate customized brp-*scripts
cat os_install_post~ | while read a x y; do
case $a in
# Prevent brp-strip* from trying to handle foreign binaries
*/brp-strip*)
  b=$(basename $a)
  sed -e 's,find $RPM_BUILD_ROOT,find $RPM_BUILD_ROOT%_bindir $RPM_BUILD_ROOT%_libexecdir,' $a > $b
  chmod a+x $b
  ;;
# Fix up brp-compress to handle %%_prefix != /usr
*/brp-compress*)
  b=$(basename $a)
  sed -e 's,\./usr/,.%{_prefix}/,g' < $a > $b
  chmod a+x $b
  ;;
esac
done

sed -e 's,^[ ]*/usr/lib/rpm.*/brp-strip,./brp-strip,' \
  -e 's,^[ ]*/usr/lib/rpm.*/brp-compress,./brp-compress,' \
< os_install_post~ > os_install_post 
%define __os_install_post . ./os_install_post

%clean
  rm -rf $RPM_BUILD_ROOT

# ==============================================================
# mipstx39-rtems4.8-gdb
# ==============================================================
# %package -n rtems-4.8-mipstx39-rtems4.8-gdb
# Summary:      rtems gdb for mipstx39-rtems4.8
# Group: Development/Tools
# %if %build_infos
# Requires: rtems-4.8-gdb-common
# %endif

%description -n rtems-4.8-mipstx39-rtems4.8-gdb

GNU gdb targetting mipstx39-rtems4.8.

%files -n rtems-4.8-mipstx39-rtems4.8-gdb
%defattr(-,root,root)
%dir %{_mandir}
%dir %{_mandir}/man1
%{_mandir}/man1/mipstx39-rtems4.8-*.1*

%dir %{_bindir}
%{_bindir}/mipstx39-rtems4.8-*

# ==============================================================
# rtems-4.8-gdb-common
# ==============================================================
%package -n rtems-4.8-gdb-common
Summary:      Base package for RTEMS gdbs
Group: Development/Tools
Requires(post):		/sbin/install-info
Requires(preun):	/sbin/install-info

Provides:	rtems-4.8-rtems4.7-base-gdb = %{gdb_version}-%{release}
Obsoletes:	rtems-4.8-rtems4.7-base-gdb < %{gdb_version}-%{release}
Provides:	rtems-4.8-rtems-base-gdb = %{gdb_version}-%{release}
Obsoletes:	rtems-4.8-rtems-base-gdb < %{gdb_version}-%{release}

%description -n rtems-4.8-gdb-common

GDB files shared by all targets.

%post -n rtems-4.8-gdb-common
  /sbin/install-info --info-dir=%{_infodir} %{_infodir}/gdb.info.gz || :
%if "%{gdb_version}" < "6.3"
  /sbin/install-info --info-dir=%{_infodir} %{_infodir}/mmalloc.info.gz || :
%endif
%if "%{gdb_version}" >= "5.0"
  /sbin/install-info --info-dir=%{_infodir} %{_infodir}/gdbint.info.gz || :
  /sbin/install-info --info-dir=%{_infodir} %{_infodir}/stabs.info.gz || :
%endif
%if "{gdb_version}" >= "6.0"
  /sbin/install-info --info-dir=%{_infodir} %{_infodir}/annotate.info.gz || :
%endif

%preun -n rtems-4.8-gdb-common
if [ $1 -eq 0 ]; then
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/gdb.info.gz || :
%if "%{gdb_version}" < "6.3"
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/mmalloc.info.gz || :
%endif
%if "%{gdb_version}" >= "5.0"
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/gdbint.info.gz || :
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/stabs.info.gz || :
%endif
%if "{gdb_version}" >= "6.0"
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/annotate.info.gz || :
%endif
fi

%files -n rtems-4.8-gdb-common
%defattr(-,root,root)
%dir %{_infodir}
%ghost %{_infodir}/dir
%{_infodir}/gdb.info*

# FIXME: When had mmalloc.info been removed?
%if "%{gdb_version}" < "6.3"
%{_infodir}/mmalloc.info*
%endif
# FIXME: When had gdbint and stabs been introduced?
%if "%{gdb_version}" >= "5.0"
%{_infodir}/gdbint.info*
%{_infodir}/stabs.info*
%endif

%if "{gdb_version}" >= "6.0"
%{_infodir}/annotate.info*
%endif
