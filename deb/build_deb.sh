#!/bin/bash
#
# Script for deb packaging dvbtee
#
# Tested on Ubuntu 14.04
#
# Author: Pasha Mesh <pasha.mesh@gmail.com>
#

PATH='/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games'

current_dir="$(dirname "$(readlink -f "$0")")"
compiled_base_dir="$(readlink -f "${current_dir}/../")"

pkg_install_dir='/opt/dvbtee'

err() {
  echo -e "[$(date +'%Y-%m-%dT%H:%M:%S%z')]: \e[32m${@}\e[0m" >&2
}

main() {
  # Run sources build
  cd "${compiled_base_dir}" && ./build.sh
  if [ "$?" -ne "0" ]; then
    err "Build is failed"
    exit 1
  fi
  cd "${current_dir}" || exit
  # end Run sources build

  #Check if fpm package
  which fpm > /dev/null
  if [ "$?" -ne "0" ]; then
    err "fpm package not installed."
    echo "Please use next commands to install it:"
    echo -e "\e[32msudo apt-get install ruby-dev gcc\e[0m"
    echo -e "\e[32msudo gem install fpm\e[0m"
    exit 1;
  fi
  # end Check if fpm package

  # Prepare file for deb packaging
  build_dir_base=${current_dir}/build
  build_dir=${build_dir_base}/${pkg_install_dir}
  build_dir_ldconf=${build_dir_base}/etc/ld.so.conf.d

  rm -rf "${build_dir_base}"

  mkdir -p "${build_dir}"/{bin,lib}
  mkdir -p "${build_dir_ldconf}"

  echo "${pkg_install_dir}/lib" > "${build_dir_ldconf}/libdvbtee.conf"

  rsync -a "${compiled_base_dir}/usr/" "${build_dir}"
  rsync -a "${compiled_base_dir}/dvbtee/dvbtee" "${build_dir}/bin"
  rsync -a "${compiled_base_dir}/libdvbtee/libdvbtee.so"* \
    "${compiled_base_dir}/libdvbtee_server/libdvbtee_server.so"* \
    "${build_dir}/lib"
  # end Prepare file for deb packaging

  pkg_name="dvbtee"
  pkg_description="Stream parser and service information aggregator library "
  pkg_description+="for MPEG2 transport streams"
  pkg_maintainer="pasha.mesh@gmail.com"
  pkg_vendor="mkrufky@linuxtv.org"
  pkg_url=$(git config --get remote.origin.url)
  pkg_iteration=$(git rev-list HEAD --count)
  pkg_version=$(git describe --abbrev=0 --tags)

  fpm -f -s dir -C build -t deb --description "${pkg_description}" \
    --vendor "${pkg_vendor}" \
    --maintainer "${pkg_maintainer}" --url "${pkg_url}" --name "${pkg_name}" \
    --version "${pkg_version/v/}" --package "." \
    --after-install "${current_dir}/scripts/_after-install.sh" \
    --after-remove "${current_dir}/scripts/_after-remove.sh" \
    --depends "libhdhomerun1 >= 20140121-1" \
    --iteration "${pkg_iteration}"
}

main