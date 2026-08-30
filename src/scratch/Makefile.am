AM_CPPFLAGS = -I$(top_srcdir)/src/include $(LUA_INCLUDE) $(BOOST_CPPFLAGS)
AM_CXXFLAGS = $(WARN_CXXFLAGS)
AM_LDFLAGS = $(WARN_LDFLAGS) $(BOOST_LDFLAGS)

bin_PROGRAMS = $(top_builddir)/bin/scratch
__top_builddir__bin_scratch_LDFLAGS = -rdynamic
__top_builddir__bin_scratch_LDADD = \
	$(BOOST_ASIO_LIB) \
	$(BOOST_CHRONO_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(LUA_LIB)
__top_builddir__bin_scratch_SOURCES = \
	action.cpp \
	color.cpp \
	color_bindings.cpp \
	command.cpp \
	command_bindings.cpp \
	config.cpp \
	config_bindings.cpp \
	data.cpp \
	descriptor.cpp \
	descriptor_bindings.cpp \
	editor.cpp \
	editor_bindings.cpp \
	editor_verbs.cpp \
	game.cpp \
	game_action.cpp \
	game_bindings.cpp \
	gender.cpp \
	gender_bindings.cpp \
	instance.cpp \
	instance_bindings.cpp \
	logger.cpp \
	lua.cpp \
	main.cpp \
	menu.cpp \
	parser.cpp \
	parser_bindings.cpp \
	protocol_telnet.cpp \
	random.cpp \
	server.cpp \
	social.cpp \
	state.cpp \
	state_bindings.cpp \
	string.cpp \
	thing.cpp \
	trust.cpp \
	trust_bindings.cpp \
	user.cpp \
	user_bindings.cpp \
	world.cpp \
	world_bindings.cpp

noinst_HEADERS = \
	../include/scratch/action.hpp \
	../include/scratch/color.hpp \
	../include/scratch/color_bindings.hpp \
	../include/scratch/command.hpp \
	../include/scratch/command_bindings.hpp \
	../include/scratch/config.hpp \
	../include/scratch/config_bindings.hpp \
	../include/scratch/data.hpp \
	../include/scratch/descriptor.hpp \
	../include/scratch/descriptor_bindings.hpp \
	../include/scratch/editor.hpp \
	../include/scratch/editor_bindings.hpp \
	../include/scratch/editor_verbs.hpp \
	../include/scratch/game.hpp \
	../include/scratch/game_bindings.hpp \
	../include/scratch/gender.hpp \
	../include/scratch/gender_bindings.hpp \
	../include/scratch/instance.hpp \
	../include/scratch/instance_bindings.hpp \
	../include/scratch/logger.hpp \
	../include/scratch/lua.hpp \
	../include/scratch/lua_bindings.hpp \
	../include/scratch/menu.hpp \
	../include/scratch/parser.hpp \
	../include/scratch/parser_bindings.hpp \
	../include/scratch/protocol.hpp \
	../include/scratch/protocol_telnet.hpp \
	../include/scratch/random.hpp \
	../include/scratch/repository.hpp \
	../include/scratch/scratch.hpp \
	../include/scratch/server.hpp \
	../include/scratch/social.hpp \
	../include/scratch/state.hpp \
	../include/scratch/state_bindings.hpp \
	../include/scratch/storage.hpp \
	../include/scratch/storage_file.hpp \
	../include/scratch/storage_file_multi.hpp \
	../include/scratch/storage_null.hpp \
	../include/scratch/string.hpp \
	../include/scratch/thing.hpp \
	../include/scratch/trust.hpp \
	../include/scratch/trust_bindings.hpp \
	../include/scratch/user.hpp \
	../include/scratch/user_bindings.hpp \
	../include/scratch/world.hpp \
	../include/scratch/world_bindings.hpp
