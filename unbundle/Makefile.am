ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign 

BUNDLE:=$(PACKAGE_TARNAME)_$(PACKAGE_VERSION)_bin.tgz

install-exec-local : unbundled-stamp

unbundled-stamp : $(BUNDLE)
	$(TAR) xfz $(BUNDLE) -C $(DESTDIR)$(PREFIX) && touch $@
