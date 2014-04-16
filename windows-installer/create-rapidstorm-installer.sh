#!/bin/bash

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 major_version minor_version debian_version"
    exit 1
fi

set -e

declare -r REPOSITORY=https://idefix.biozentrum.uni-wuerzburg.de/debian
declare -r DIST=wheezy
declare -r VERSION=$3
declare -r MARKETING_VERSION="$1.$2"
declare -r MAJOR_VERSION="$1"

while read ARCH HOST_TYPE MARKETING_ARCH COMPILER_PACKAGE; do
    TEMPDIR="tmp-${ARCH}"
    APT_DIR="$(pwd)/${TEMPDIR}/apt-tree"
    STAGE_DIR="${TEMPDIR}/stage"
    APT_CONFIG="${TEMPDIR}/apt.conf"

    mkdir -p "${TEMPDIR}"

    cat > ${APT_CONFIG} <<EOF
APT::Architecture "${ARCH}";
APT::Architectures { "mingw-i686"; "mingw-amd64"; "amd64"; }
Dir::State      "${APT_DIR}/var/lib/apt";
Dir::Cache      "${APT_DIR}/var/cache/apt";
Dir::Etc        "${APT_DIR}/etc/apt";
Dir::Log        "${APT_DIR}/";
Dir::Parts      "${APT_DIR}/etc/apt/apt.conf.d";
Dir::State::status "${APT_DIR}/var/lib/dpkg/status";
APT::Get::Assume-Yes    "true";
EOF

    mkdir --parents ${APT_DIR}/etc/apt/preferences.d ${APT_DIR}/etc/apt/apt.conf.d
    cat > ${APT_DIR}/etc/apt/sources.list <<EOF
deb [arch=mingw-i686,mingw-amd64] ${REPOSITORY} ${DIST} main non-free contrib mingw
# We need the main Debian repository for the gcc-mingw-w64-* package
deb [arch=amd64] http://ftp.de.debian.org/debian/ ${DIST} main
EOF

    mkdir --parents ${APT_DIR}/var/{cache/apt/archives,lib/{apt/{lists,mirrors},dpkg}}
    touch ${APT_DIR}/var/lib/dpkg/status
    APT_CONFIG=${APT_CONFIG} apt-get -o Debug::NoLocking=yes update

    APT_CONFIG=${APT_CONFIG} apt-get --download-only --allow-unauthenticated install rapidstorm=${VERSION} rapidstorm-doc=${VERSION}

    pushd ${TEMPDIR}
    APT_CONFIG=${APT_CONFIG} apt-get download --allow-unauthenticated ${COMPILER_PACKAGE}:amd64
    popd

    mkdir -p "${STAGE_DIR}"

    # We need only the dll's that are packaged into the gcc compiler.
    # Extract them first and delete the rest.
    dpkg -x ${TEMPDIR}/${COMPILER_PACKAGE}_*_amd64.deb ${STAGE_DIR}
    find ${STAGE_DIR} -type f -not -name '*.dll' -delete

    # Now extract all the rest over it.
    find "${APT_DIR}/var/cache/apt" -name '*.deb' -exec dpkg -x '{}' ${STAGE_DIR} ';'
    find ${STAGE_DIR} -iname '*.exe' -or -iname '*.dll' -exec ${HOST_TYPE}-strip '{}' ';'
    find ${STAGE_DIR}/{bin,usr/lib} -iname '*.dll' -exec mv -n '{}' ${STAGE_DIR}/usr/bin ';'
    find ${STAGE_DIR} -name '*.pdf.gz' -type f -exec gunzip '{}' ';'

    {
        sed -e "s/[@]VERSION[@]/${MARKETING_VERSION}/g" \
            -e "s/[@]MAJOR_VERSION[@]/${MAJOR_VERSION}/g" \
            -e "s:[@]srcdir[@]:Z\:$(dirname $0):g" \
            -e "s:[@]README[@]:Z\:$(pwd)/${STAGE_DIR}/usr/share/doc/rapidstorm/README:g" \
            < $(dirname $0)/rapidstorm-setup.iss.in;

        cat $(dirname $0)/${ARCH}.iss

        echo "[Files]"; echo;
        pushd ${STAGE_DIR}/usr >/dev/null
        find -type f -printf "Source: \"Z:$(pwd)/%p\"; DestDir: \"{app}/%h/\"; Flags: ignoreversion\n" | sed -e 's:/\./:/:g'
        popd >/dev/null
    } > ${TEMPDIR}/setup.iss

    wine "C:/Program Files/Inno Setup 5/ISCC.exe" ${TEMPDIR}/setup.iss
    mv ${TEMPDIR}/Output/setup.exe rapidstorm-${MARKETING_VERSION}-${MARKETING_ARCH}.exe
    rm -rf ${TEMPDIR}
done <<EOF
mingw-i686 i686-w64-mingw32 win32 gcc-mingw-w64-i686
mingw-amd64 x86_64-w64-mingw32 win64 gcc-mingw-w64-x86-64
EOF
