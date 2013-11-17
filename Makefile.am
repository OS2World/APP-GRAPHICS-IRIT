SUBDIRS = \
	grap_lib	\
	prsr_lib	\
	cagd_lib	\
	bool_lib	\
	geom_lib	\
	mdl_lib		\
	misc_lib	\
	mvar_lib	\
	symb_lib	\
	rndr_lib	\
	trim_lib	\
	triv_lib	\
	trng_lib	\
	user_lib	\
	xtra_lib	\
	ext_lib	 	\
	irit	 	\
	aisoshad	\
	filters		\
	grapdrvs	\
	ihidden	 	\
	illustrt	\
	irender	 	\
	poly3d-h	\
	include



EXTRA_DIST = scripts

install-data-local:
	mkdir $(prefix)/scripts
	cp -f $(srcdir)/scripts/* $(prefix)/scripts
	chmod -R a+r $(prefix)/scripts
	echo "export IRIT_PATH=$(bindir)" >> /etc/profile.d/irit.sh
	echo -e "export PATH=\$$PATH:$(bindir)" >> /etc/profile.d/irit.sh
	echo "export LD_LIBRARY_PATH=$(libdir)" >> /etc/profile.d/irit.sh
	echo "export IRIT_DISPLAY=\"x11drvs -s-\"" >> /etc/profile.d/irit.sh
	echo "export C_INCLUDE_PATH=$(includedir)" >> /etc/profile.d/irit.sh
	echo "export CPLUS_INCLUDE_PATH=$(includedir)" >> /etc/profile.d/irit.sh
	chmod a+x /etc/profile.d/irit.sh
	. /etc/profile

uninstall-local:
	rm -f /etc/profile.d/irit.sh
	rm -rf $(prefix)/scripts

