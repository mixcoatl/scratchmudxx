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
	descriptor.cpp \
	game.cpp \
	logger.cpp \
	main.cpp \
	random.cpp \
	server.cpp \
	string.cpp

noinst_HEADERS = \
	../include/scratch/data.hpp \
	../include/scratch/descriptor.hpp \
	../include/scratch/game.hpp \
	../include/scratch/logger.hpp \
	../include/scratch/random.hpp \
	../include/scratch/repository.hpp \
	../include/scratch/scratch.hpp \
	../include/scratch/server.hpp \
	../include/scratch/storage.hpp \
	../include/scratch/storage_file.hpp \
	../include/scratch/storage_file_multi.hpp \
	../include/scratch/storage_null.hpp \
	../include/scratch/string.hpp
