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
set(PLUGIN_VERSION "1.0.0")
set(PLUGIN_HEX_VERSION "0x0100")
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

install(FILES greeting.conf
        DESTINATION ${INSTALL_SYSCONFDIR}/${PLUGIN_NAME}/)
install(FILES greeting.sql
        DESTINATION ${INSTALL_SHAREDIR}/${PLUGIN_NAME}/)

include(CPack)
