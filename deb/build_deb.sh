#!/bin/bash
#
# build_deb.sh:
# Script for deb packaging dvbtee
#
# Originally submitted by: Pasha Mesh <pasha.mesh@gmail.com>
#
# Tested on Ubuntu 14.04 & 15.04
#

PATH='/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

current_dir="$(dirname "$(readlink -f "$0")")"
compiled_base_dir="$(readlink -f "${current_dir}/../")"

pkg_install_dir='/usr'

err() {
  echo -e "[$(date +'%Y-%m-%dT%H:%M:%S%z')]: \e[32m${@}\e[0m" >&2
}

main() {
  build_dir_base=${current_dir}/build
  build_dir=${build_dir_base}/${pkg_install_dir}
  build_dir_ldconf=${build_dir_base}/etc/ld.so.conf.d

  # Run sources build
  cd "${compiled_base_dir}" && autoreconf -i && ./configure --prefix="${build_dir}" && make clean && make && make install
  if [ "$?" -ne "0" ]; then
    err "Build failed"
    exit 1
  fi
  cd "${current_dir}" || exit
  # end Run sources build

  #Check if fpm package
  which fpm > /dev/null
  if [ "$?" -ne "0" ]; then
    err "fpm package is not installed."
    echo "Please use the following commands to install it:"
    echo -e "\e[32msudo apt-get install ruby-dev gcc\e[0m"
    echo -e "\e[32msudo gem install fpm\e[0m"
    exit 1;
  fi
  # end Check if fpm package

  # Prepare file for deb packaging
  mkdir -p "${build_dir_ldconf}"

  echo "${pkg_install_dir}/lib" > "${build_dir_ldconf}/libdvbtee.conf"
  # end Prepare file for deb packaging

  pkg_name="dvbtee"
  pkg_description="MPEG2 transport stream parser & digital television service info aggregation library"
  pkg_maintainer="mkrufky@linuxtv.org"
  pkg_vendor="mkrufky@linuxtv.org"
  pkg_url="git://github.com/mkrufky/libdvbtee.git"
  pkg_iteration=$(git rev-list HEAD --count)
  pkg_version=$(git describe --abbrev=0 --tags)

  fpm -f -s dir -C build -t deb --description "${pkg_description}" \
    --vendor "${pkg_vendor}" \
    --maintainer "${pkg_maintainer}" --url "${pkg_url}" --name "${pkg_name}" \
    --version "${pkg_version/v/}" --package "." \
    --after-install "${current_dir}/scripts/_after-install.sh" \
    --after-remove "${current_dir}/scripts/_after-remove.sh" \
    --depends "libhdhomerun1 >= 20140121-1" \
    --depends "libdvbpsi9" \
    --iteration "${pkg_iteration}"

  rm -rf "${build_dir_base}"
}

main
