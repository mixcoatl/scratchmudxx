ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src

# Files not picked up via _SOURCES / noinst_HEADERS / Automake defaults.
# Listing the data/ directory includes whatever default game data lives there
# at dist time (today just README; add new default files under data/).
#
# For bin/, list bin/README only — never EXTRA_DIST = bin — so built
# binaries (scratch, .libs, .dirstamp) stay out of the tarball.
EXTRA_DIST = \
	autorun \
	bootstrap \
	bin/README \
	data \
	log/README \
	m4/README

# Belt-and-suspenders: strip any build outputs that might appear under bin/.
dist-hook:
	rm -f $(distdir)/bin/scratch $(distdir)/bin/scratch$(EXEEXT)
	rm -f $(distdir)/bin/.dirstamp
	rm -rf $(distdir)/bin/.libs
