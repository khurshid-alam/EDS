NULL =

module_LTLIBRARIES = module-google-backend.la

module_google_backend_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"module-google-backend\" \
	$(E_BACKEND_CFLAGS) \
	$(E_DATA_SERVER_CFLAGS) \
	$(GDATA_CFLAGS) \
	$(NULL)

module_google_backend_la_SOURCES = \
	module-google-backend.c \
	e-gdata-oauth2-authorizer.c \
	e-gdata-oauth2-authorizer.h \
	$(NULL)

module_google_backend_la_LIBADD = \
	$(top_builddir)/libebackend/libebackend-1.2.la \
	$(top_builddir)/libedataserver/libedataserver-1.2.la \
	$(top_builddir)/camel/libcamel-1.2.la \
	$(E_BACKEND_LIBS) \
	$(E_DATA_SERVER_LIBS) \
	$(GDATA_LIBS) \
	$(NULL)

module_google_backend_la_LDFLAGS = \
	-module -avoid-version $(NO_UNDEFINED) \
	$(NULL)

-include $(top_srcdir)/git.mk
