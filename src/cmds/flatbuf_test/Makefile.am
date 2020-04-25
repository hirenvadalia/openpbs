bin_PROGRAMS = \
	flattest

common_cflags = \
	-I$(top_srcdir)/src/cmds/flatbuf_test \
	-I$(top_srcdir)/src/include \
	@libz_inc@ \
	@flatcc_inc@

common_libs = \
	$(top_builddir)/src/lib/Libpbs/.libs/libpbs.a \
	$(top_builddir)/src/lib/Libtpp/libtpp.a \
	$(top_builddir)/src/lib/Liblog/liblog.a \
	$(top_builddir)/src/lib/Libutil/libutil.a \
	$(top_builddir)/src/lib/Libnet/libnet.a \
	$(top_builddir)/src/lib/Libsec/libsec.a \
	-lpthread \
	@socket_lib@ \
	@libz_lib@ \
	@flatcc_lib@

flattest_CPPFLAGS = ${common_cflags}
flattest_LDADD = ${common_libs}
flattest_SOURCES = flattest.c
