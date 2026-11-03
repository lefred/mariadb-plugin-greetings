# Copyright (c) 2026 lefred (Frederic Descamps)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335 USA

cmake_minimum_required(VERSION 3.12)

project(greeting)

set(PLUGIN_NAME "greeting")
set(PLUGIN_AUTHOR "lefred")
set(PLUGIN_VERSION "1.0.1")
set(PLUGIN_HEX_VERSION "0x0101")
set(PLUGIN_DESCRIPTION "Sample plugin whose GREETING() function returns the message set in its config file")

#
# New MariaDB external plugin infrastructure (Foundry).
#
find_package(mariadb-plugin REQUIRED)

MARIADB_ADD_PLUGIN(${PLUGIN_NAME} greeting.cc
        MODULE_ONLY
        RECOMPILE_FOR_EMBEDDED
        AUTHOR "${PLUGIN_AUTHOR}"
        VERSION "${PLUGIN_VERSION}"
        DESCRIPTION "${PLUGIN_DESCRIPTION}"
)

# Where Marina's installer accepts a plugin's own files for any build, even one
# below verified: <etc>/mariadb/<name>/ (INSTALL_SYSCONFDIR is /etc for RPM and
# DEB, etc in a tarball) and share/mariadb/<name>/ (INSTALL_SHAREDIR is plain
# share in every layout, so it is written out).
install(FILES greeting.conf
        DESTINATION ${INSTALL_SYSCONFDIR}/mariadb/${PLUGIN_NAME}/)
install(FILES greeting.sql
        DESTINATION share/mariadb/${PLUGIN_NAME}/)

include(CPack)
