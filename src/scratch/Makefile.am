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
	data.cpp \
	game.cpp \
	logger.cpp \
	main.cpp \
	random.cpp \
	string.cpp

noinst_HEADERS = \
	../include/scratch/data.hpp \
	../include/scratch/game.hpp \
	../include/scratch/logger.hpp \
	../include/scratch/random.hpp \
	../include/scratch/scratch.hpp \
	../include/scratch/string.hpp
