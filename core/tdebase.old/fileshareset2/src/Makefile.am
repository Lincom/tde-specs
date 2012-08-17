fileshareset_SOURCES = fileshareset.cpp fssConfig.cpp fssConfigNfs.cpp \
				  fssShare.cpp fssShareNfs.cpp \
				  fssShareSmb.cpp fssConfigSmb.cpp
bin_PROGRAMS = fileshareset

install-exec-hook:
	ln -s fileshareset $(DESTDIR)$(bindir)/filesharelist
