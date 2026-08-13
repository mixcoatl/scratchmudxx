AM_CPPFLAGS = -I$(top_srcdir)/src/include $(BOOST_CPPFLAGS)
AM_CXXFLAGS = $(WARN_CXXFLAGS)
AM_LDFLAGS = $(WARN_LDFLAGS) $(BOOST_LDFLAGS)

bin_PROGRAMS = $(top_builddir)/bin/scratch
__top_builddir__bin_scratch_LDFLAGS = -rdynamic
__top_builddir__bin_scratch_LDADD = \
	$(BOOST_ASIO_LIB) \
	$(BOOST_CHRONO_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB)
__top_builddir__bin_scratch_SOURCES = \
	color.cpp \
	config.cpp \
	data.cpp \
	descriptor.cpp \
	game.cpp \
	logger.cpp \
	main.cpp \
	protocol_telnet.cpp \
	random.cpp \
	server.cpp \
	string.cpp

noinst_HEADERS = \
	../include/scratch/color.hpp \
	../include/scratch/config.hpp \
	../include/scratch/data.hpp \
	../include/scratch/descriptor.hpp \
	../include/scratch/game.hpp \
	../include/scratch/logger.hpp \
	../include/scratch/protocol.hpp \
	../include/scratch/protocol_telnet.hpp \
	../include/scratch/random.hpp \
	../include/scratch/scratch.hpp \
	../include/scratch/server.hpp \
	../include/scratch/string.hpp
