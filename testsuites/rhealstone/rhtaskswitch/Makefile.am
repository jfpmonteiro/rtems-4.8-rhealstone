MANAGERS = all

rtems_tests_PROGRAMS = rhtaskswitch
rhtaskswitch_SOURCES  = taskswitch.c

dist_rtems_tests_DATA = rhtaskswitch.adoc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS = -I$(top_srcdir)/../support/include
AM_CPPFLAGS += -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/../tmitrontests/include

LINK_OBJS = $(rhtaskswitch_OBJECTS) $(rhtaskswitch_LDADD)
LINK_LIBS = $(rhtaskswitch_LDLIBS)

rhtaskswitch$(EXEEXT): $(rhtaskswitch_OBJECTS) $(rhtaskswitch_DEPENDENCIES)
	@rm -f rhtaskswitch$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
