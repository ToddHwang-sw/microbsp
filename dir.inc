
## Libraries
_LIBDIR_=\
	zlib       \
	pcre       \
	lzo        \
	liblzma    \
	bzip2      \
	lz4        \
	zstd       \
	gmp        \
	mpfr       \
	mpc        \
	isl        \
	libffi     \
	libnl      \
	ncurses    \
	readline   \
	libcap     \
	util-linux \
	e2fsprogs  \
	libparted  \
	openssl    \
	libssh2    \
	nettle     \
	flex       \
	lsjson     \
	pcre2      \
	libxml     \
	libxslt    \
	expat      \
	libpam     \
	libpcap    \
	libev      \
	gnulib     \
	libtasn1   \
	libtool    \
	libattr
export _LIBDIR_

## Applications 
_APPDIR_=\
	busybox   \
	bash      \
	vim       \
	glibc
export _APPDIR_

# External disk 
_EXTDIR_=\
	cpu                        \
	btrfs                      \
	binutils                   \
	libbind                    \
	fdisk                      \
	iperf                      \
	ncftp                      \
	ntpclient                  \
	sqlite                     \
	iconv                      \
	autoconf                   \
	automake                   \
	proxy-libintl              \
	m4                         \
	make                       \
	gettext                    \
	glib                       \
	pkg-config                 \
	module-tools               \
	procps                     \
	strace                     \
	openssh                    \
	gcc                        \
	gdb                        \
	gdbm                       \
	perl                       \
	python                     \
	pciaccess                  \
	drm                        \
	gobject-introspection      \
	libidn                     \
	gnutls                     \
	curl                       \
	git                        \
	xconfig                    \
	http/fastcgi               \
	http/lighttpd              \
	tcpdump                    \
	json                       \
	jq                         \
	texinfo                    \
	help2man                   \
	mtdev                      \
	ethtool                    \
	linuxptp                   \
	pciutils                   \
	i2ctools                   \
	spitools                   \
	gpio                       \
	lpps
export _EXTDIR_

##
##
## UI-related Materials - excluded from SUBDIR set. 
##
##
_UIDIR_=\
	libjpeg                \
	libpng	               \
	freetype2              \
	fontconfig             \
	libgpg-error           \
	libgcrypt              \
	libdbus	               \
	sndfile	               \
	X11/xorg-macros        \
	X11/xorg-doctools      \
	X11/xorg-x11proto      \
	X11/xorg-xtrans        \
	X11/xorg-kbproto       \
	X11/libXau             \
	X11/xorg-xcbproto      \
	X11/xorg-libxcb        \
	X11/xorg-proto         \
	X11/libX11             \
	X11/libICE             \
	X11/xorg-libsm         \
	X11/libXt              \
	X11/libXext            \
	X11/libXmu             \
	X11/libXpm             \
	X11/libXaw             \
	X11/libXrender         \
	X11/libXfixes          \
	X11/libXinput          \
	X11/libXdamage         \
	X11/libXcursor         \
	X11/xorg-libxtst       \
	X11/xshmfence          \
	X11/libXxf86vm         \
	X11/libXrandr          \
	libseccomp             \
	systemd                \
	libevdev               \
	wayland-host           \
	wayland                \
	wayland-protocols      \
	xkbcommon              \
	pixman                 \
	mesa                   \
	cairo                  \
	fribidi                \
	harfbuzz               \
	gdk-pixbuf             \
	pango                  \
	epoxy                  \
	graphene               \
	gtk                    \
	libinput               \
	libva                  \
	weston                 \
	libalsa                \
	libSDL                 \
	ffmpeg                 \
	pygobject              \
	gstreamer/base         \
	gstreamer/plugin-base  \
	gstreamer/plugin-good  \
	gstreamer/plugin-bad   \
	gstreamer/plugin-ugly  \
	gstreamer/libav        \
	gstreamer/python       \
	gstreamer/vaapi        \
	microwindows	       \
	pulseaudio             \
	qt
export _UIDIR_
