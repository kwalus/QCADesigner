docdir = $(datadir)/doc/QCADesigner

# doc_DATA = manual

# EXTRA_DIST = $(doc_DATA)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	for Nix in `find manual | grep -v CVS`; do \
	[ -d $$Nix ] && \
	{ $(mkinstalldirs) $(DESTDIR)$(docdir)/$$Nix ; } ; \
	[ -f $$Nix ] && \
	{ $(INSTALL) -m 644 $$Nix $(DESTDIR)$(docdir)/$$Nix ; } ; \
	done
	chown -Rf root:root $(DESTDIR)$(docdir)/manual ; exit 0

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/manual
