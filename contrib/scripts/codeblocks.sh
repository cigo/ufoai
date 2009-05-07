#!/bin/bash

#######################################################
# functions
#######################################################

download()
{
	if [ -e "${DOWNLOAD_DIR}/$3" ]
	then
		return 0
	else
		mkdir -p ${DOWNLOAD_DIR}
		pushd ${DOWNLOAD_DIR}
		${WGET} $1$2 -O $3
		popd
		return $?
	fi
}

start_downloads()
{
	# using an older version here - as windres is buggy in higher versions. Latest tested version is 2.19.1
	download http://downloads.sourceforge.net/mingw/ binutils-2.16.91-20060119-1.tar.gz binutils.tar.gz
	#download http://downloads.sourceforge.net/mingw/ binutils-2.19.1-mingw32-bin.tar.gz binutils.tar.gz
	download http://downloads.sourceforge.net/mingw/ mingwrt-3.15.2-mingw32-dev.tar.gz mingwrt.tar.gz
	download http://downloads.sourceforge.net/mingw/ w32api-3.13-mingw32-dev.tar.gz w32api.tar.gz
	download http://downloads.sourceforge.net/mingw/ gdb-6.8-mingw-3.tar.bz2 gdb.tar.bz2
	download http://downloads.sourceforge.net/mingw/ mingw32-make-3.81-20080326-3.tar.gz make.tar.gz

	download http://downloads.sourceforge.net/mingw/ bash-3.1-MSYS-1.0.11-1.tar.bz2 msys-bash.tar.bz2
	download http://downloads.sourceforge.net/mingw/ bzip2-1.0.3-MSYS-1.0.11-1.tar.bz2 msys-bzip2.tar.bz2
	download http://downloads.sourceforge.net/mingw/ tar-1.19.90-MSYS-1.0.11-2-bin.tar.gz msys-tar.tar.gz
	download http://downloads.sourceforge.net/mingw/ coreutils-5.97-MSYS-1.0.11-snapshot.tar.bz2 msys-coreutils.tar.bz2
	download http://downloads.sourceforge.net/mingw/ findutils-4.3.0-MSYS-1.0.11-3-bin.tar.gz msys-findutils.tar.gz
	download http://downloads.sourceforge.net/mingw/ MSYS-1.0.11-20090120-dll.tar.gz msys.tar.gz
	download http://downloads.sourceforge.net/mingw/ msysCORE-1.0.11-20080826.tar.gz msys-core.tar.gz
	download http://downloads.sourceforge.net/mingw/ autoconf-4-1-bin.tar.bz2 msys-autoconf.tar.bz2 
	    
	download http://downloads.sourceforge.net/tdm-gcc/ gcc-4.4.0-tdm-1-core-2.tar.gz gcc.tar.gz
	download http://downloads.sourceforge.net/tdm-gcc/ gcc-4.4.0-tdm-1-g++-2.tar.gz g++.tar.gz

	download http://downloads.sourceforge.net/gnuwin32/ freetype-2.3.6-lib.zip freetype.zip
	download http://downloads.sourceforge.net/gnuwin32/ gettext-0.14.4-bin.zip gettext.zip
	download http://downloads.sourceforge.net/gnuwin32/ jpeg-6b-4-lib.zip libjpeg.zip
	download http://downloads.sourceforge.net/gnuwin32/ libiconv-1.9.2-1-lib.zip libiconv.zip
	download http://downloads.sourceforge.net/gnuwin32/ libiconv-1.9.2-1-bin.zip libiconv-bin.zip
	download http://downloads.sourceforge.net/gnuwin32/ libintl-0.14.4-lib.zip libintl.zip
	download http://downloads.sourceforge.net/gnuwin32/ libpng-1.2.35-lib.zip libpng.zip
	download http://downloads.sourceforge.net/gnuwin32/ libxml2-2.4.12-1-lib.zip libxml2.zip
	download http://downloads.sourceforge.net/gnuwin32/ tiff-3.8.2-1-lib.zip libtiff.zip
	download http://downloads.sourceforge.net/gnuwin32/ wget-1.11.4-1-bin.zip wget.zip
	download http://downloads.sourceforge.net/gnuwin32/ zlib-1.2.3-lib.zip zlib.zip
	download http://downloads.sourceforge.net/gnuwin32/ unzip-5.51-1-bin.zip unzip.zip
	download http://downloads.sourceforge.net/gnuwin32/ openssl-0.9.8h-1-bin.zip openssl.zip
	
	download http://downloads.sourceforge.net/sevenzip/ 7za465.zip 7zip.zip

	download http://curl.de-mirror.de/download/ libcurl-${CURL_VERSION}-win32-nossl.zip libcurl.zip

	download http://www.libsdl.org/release/ SDL-devel-${SDL_VERSION}-mingw32.tar.gz sdl.tar.gz
	download http://www.libsdl.org/projects/SDL_ttf/release/ SDL_ttf-devel-${SDL_TTF_VERSION}-VC8.zip sdl_ttf.zip
	download http://www.libsdl.org/projects/SDL_mixer/release/ SDL_mixer-devel-${SDL_MIXER_VERSION}-VC8.zip sdl_mixer.zip
	download http://www.libsdl.org/projects/SDL_image/release/ SDL_image-devel-${SDL_IMAGE_VERSION}-VC8.zip sdl_image.zip

	download http://oss.netfarm.it/mplayer/pkgs/ libvorbis-mingw32-1.2.0-gcc45.tar.bz2 libvorbis.tar.bz2
	download http://oss.netfarm.it/mplayer/pkgs/ libtheora-mingw32-1.1alpha1-gcc45.tar.bz2 libtheora.tar.bz2
	download http://oss.netfarm.it/mplayer/pkgs/ xvidcore-mingw32-1.2.1-gcc45.tar.bz2 xvidcore.tar.bz2
	download http://oss.netfarm.it/mplayer/pkgs/ libogg-mingw32-1.1.3-gcc45.tar.bz2 libogg.tar.bz2
	
#	download http://downloads.xiph.org/releases/ogg/ libogg-1.1.3.tar.gz libogg.tar.gz
#	download http://downloads.xiph.org/releases/ogg/ libvorbis-1.2.0.tar.gz libvorbis.tar.gz

	download http://download.berlios.de/codeblocks/ wxmsw28u_gcc_cb_wx2810.7z codeblocks_gcc.7z
	download http://download.berlios.de/codeblocks/ mingwm10_gcc421.7z codeblocks_mingw.7z
	download http://download.berlios.de/codeblocks/ CB_20090413_rev5535_win32.7z codeblocks.7z

	download http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.18/ glib-dev_2.18.4-1_win32.zip glib-dev.zip
	download http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.14/ gtk+-dev_2.14.7-1_win32.zip gtk+-dev.zip
	download http://ftp.gnome.org/pub/gnome/binaries/win32/pango/1.22/ pango-dev_1.22.4-1_win32.zip pango-dev.zip
	download http://ftp.gnome.org/pub/gnome/binaries/win32/atk/1.24/ atk-dev_1.24.0-1_win32.zip atk-dev.zip
	download http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/ cairo-dev_1.8.6-1_win32.zip cairo-dev.zip

	download http://mattn.ninex.info/download gtkglext-1.2.tar.bz2 gtkglext-dev.tar.bz2

	download http://subversion.tigris.org/files/documents/15/45600/ svn-win32-1.6.1.zip svn.zip

	download http://www.libsdl.org/extras/win32/common/ directx-devel.tar.gz directx.tar.gz
}

extract_codeblocks()
{
	${UN7ZIP} x -y -o${CODEBLOCKS_DIR} ${DOWNLOAD_DIR}/codeblocks_mingw.7z
	${UN7ZIP} x -y -o${CODEBLOCKS_DIR} ${DOWNLOAD_DIR}/codeblocks_gcc.7z
	${UN7ZIP} x -y -o${CODEBLOCKS_DIR} ${DOWNLOAD_DIR}/codeblocks.7z
}

extract_mingw()
{
	${TAR} -xzf ${DOWNLOAD_DIR}/binutils.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/mingwrt.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/w32api.tar.gz -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/gdb.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/gcc.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/g++.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/make.tar.gz -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/msys-bash.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/msys-bzip2.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/msys-tar.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/msys-findutils.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/msys.tar.gz -C ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/msys-core.tar.gz -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/msys-autoconf.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/msys-coreutils.tar.bz2 -C ${MINGW_DIR}
	cp -R ${MINGW_DIR}/coreutils*/* ${MINGW_DIR}
	rm -rf ${MINGW_DIR}/coreutils*
}

extract_libs()
{
	${UNZIP} -o ${DOWNLOAD_DIR}/zlib.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libjpeg.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libpng.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libiconv-bin.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libiconv.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libintl.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/freetype.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libtiff.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/gettext.zip -d ${MINGW_DIR}
	${TAR} -xzf ${DOWNLOAD_DIR}/directx.tar.gz -C ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/libxml2.zip -d ${MINGW_DIR}
	cp -R ${MINGW_DIR}/include/libxml2/* ${MINGW_DIR}/include
	rm -r ${MINGW_DIR}/include/libxml2
}

extract_libcurl()
{
	mkdir ${TEMP_DIR}/tmp
	${UNZIP} -o ${DOWNLOAD_DIR}/libcurl.zip -d ${TEMP_DIR}/tmp
	cp ${TEMP_DIR}/tmp/libcurl-${CURL_VERSION}/* -R ${MINGW_DIR}
	rm -rf ${TEMP_DIR}/tmp
}

extract_sdl()
{
	mkdir ${TEMP_DIR}/tmp
	${UNZIP} -o ${DOWNLOAD_DIR}/sdl_mixer.zip -d ${TEMP_DIR}/tmp
	${UNZIP} -o ${DOWNLOAD_DIR}/sdl_ttf.zip -d ${TEMP_DIR}/tmp
	${UNZIP} -o ${DOWNLOAD_DIR}/sdl_image.zip -d ${TEMP_DIR}/tmp
	${TAR} -xzf ${DOWNLOAD_DIR}/sdl.tar.gz -C ${TEMP_DIR}/tmp
	pushd ${TEMP_DIR}/tmp/SDL-${SDL_VERSION}
	make install-sdl prefix=${MINGW_DIR}
	popd
	cp ${TEMP_DIR}/tmp/SDL_mixer-${SDL_MIXER_VERSION}/include/* ${MINGW_DIR}/include/SDL
	cp ${TEMP_DIR}/tmp/SDL_mixer-${SDL_MIXER_VERSION}/lib/* ${MINGW_DIR}/lib
	cp ${TEMP_DIR}/tmp/SDL_ttf-${SDL_TTF_VERSION}/include/* ${MINGW_DIR}/include/SDL
	cp ${TEMP_DIR}/tmp/SDL_ttf-${SDL_TTF_VERSION}/lib/* ${MINGW_DIR}/lib
	cp ${TEMP_DIR}/tmp/SDL_image-${SDL_IMAGE_VERSION}/include/* ${MINGW_DIR}/include/SDL
	cp ${TEMP_DIR}/tmp/SDL_image-${SDL_IMAGE_VERSION}/lib/* ${MINGW_DIR}/lib
	rm -rf ${TEMP_DIR}/tmp
}

extract_tools()
{
	mkdir ${TEMP_DIR}/tmp
	${UNZIP} -o ${DOWNLOAD_DIR}/wget.zip d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/unzip.zip d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/openssl.zip d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/7zip.zip d ${MINGW_DIR}/tmp
	${UNZIP} -o ${DOWNLOAD_DIR}/svn.zip d ${MINGW_DIR}/tmp
	#some parts of openssl are also included in the svn package
	cp -R ${TEMP_DIR}/tmp/7za.exe ${MINGW_DIR}/bin/7z.exe
	cp -R ${TEMP_DIR}/tmp/7za.exe ${MINGW_DIR}/bin/7za.exe
	cp -R ${TEMP_DIR}/tmp/svn-win32*/* ${MINGW_DIR}
	rm -rf ${TEMP_DIR}/tmp
}

extract_gtk()
{
	${UNZIP} -o ${DOWNLOAD_DIR}/glib-dev.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/gtk+-dev.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/pango-dev.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/atk-dev.zip -d ${MINGW_DIR}
	${UNZIP} -o ${DOWNLOAD_DIR}/cairo-dev.zip -d ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/gtkglext-dev.tar.gz -C ${MINGW_DIR}

	for i in $(echo "glib-2.0 gail-1.0 gtk-2.0 pango-1.0 atk-1.0 cairo gtkglext-1.0 gtkglextmm-1.2"); do
		cp -R ${MINGW_DIR}/include/${i}/* ${MINGW_DIR}/include
		rm -r ${MINGW_DIR}/include/${i}
		cp -R ${MINGW_DIR}/lib/${i}/include/* ${MINGW_DIR}/include
		rm -r ${MINGW_DIR}/lib/${i}
	done
}

extract_ogg()
{
	${TAR} -xjf ${DOWNLOAD_DIR}/libogg.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/xvidcore.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/libtheora.tar.bz2 -C ${MINGW_DIR}
	${TAR} -xjf ${DOWNLOAD_DIR}/libvorbis.tar.bz2 -C ${MINGW_DIR}
}

#######################################################
# variables
#######################################################

# Some packages are using the version information at several places
# because we have to copy some files around
SDL_VERSION=1.2.13
SDL_MIXER_VERSION=1.2.8
SDL_TTF_VERSION=2.0.9
SDL_IMAGE_VERSION=1.2.6
CURL_VERSION=7.16.4

TEMP_DIR=$(pwd)/tmp_codeblocks
TARGET_DIR=${TEMP_DIR}/codeblocks
DOWNLOAD_DIR=${TEMP_DIR}/downloads
MINGW_DIR=${TARGET_DIR}/MinGW
CODEBLOCKS_DIR=${TARGET_DIR}
ARCHIVE_NAME=codeblocks.zip
UN7ZIP=$(which 7z)
WGET=$(which wget)
UNZIP=$(which unzip)
TAR=$(which tar)

if [ ! -x "${UN7ZIP}" ]; then
	echo "you need 7zip installed to run this script"
	exit 1;
fi

if [ ! -x "${WGET}" ]; then
	echo "you need wget installed to run this script"
	exit 1;
fi

if [ ! -x "${UNZIP}" ]; then
	echo "you need unzip installed to run this script"
	exit 1;
fi

if [ ! -x "${TAR}" ]; then
	echo "you need tar installed to run this script"
	exit 1;
fi

#######################################################
# don't change anything below here
#######################################################

create()
{
	mkdir -p ${TEMP_DIR}
	rm -rf ${TARGET_DIR}
	mkdir -p ${TARGET_DIR}
	mkdir -p ${CODEBLOCKS_DIR}
	mkdir -p ${MINGW_DIR}

	start_downloads

	extract_codeblocks

	extract_mingw

	extract_libs

	extract_sdl

	extract_libcurl

	extract_tools

	extract_gtk

	extract_ogg

	${UN7ZIP} a -tzip -mx=9 ${ARCHIVE_NAME} ${CODEBLOCKS_DIR}

	if [ ! -e "${ARCHIVE_NAME}" ]; then
		echo "Failed to create ${ARCHIVE_NAME}"
		exit 1;
	fi
	echo "finished creating ${ARCHIVE_NAME} in $(pwd)"
}

ACTION=${1:-help}

if [ "$ACTION" == "create" ]; then
	create
elif [ "$ACTION" == "help" ]; then
	echo "Valid parameters are: help, create and clean"
	echo " - create  : creates the ${ARCHIVE_NAME} with the latest versions"
	echo "             of the needed programs and libs"
	echo " - clean   : removes the downloads and the temp dirs - but not"
	echo "             the created archive file"
	echo " - help    : prints this help message"
	echo " - upload  : uploads the created archive - ./ssh/config must be"
	echo "             set up for this to work"
elif [ "$ACTION" == "upload" ]; then
	if [ ! -e "${ARCHIVE_NAME}" ]; then
		echo "${ARCHIVE_NAME} doesn't exist. Hit any key to create it."
		read
		create
	fi
	scp ${ARCHIVE_NAME} ufo:~/public_html/download
elif [ "$ACTION" == "clean" ]; then
	rm -rf ${TEMP_DIR}
	echo "clean finished"
else
	echo "unknown action given"
fi
