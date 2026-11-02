/* Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */

/*
  A sample plugin: GREETING() returns the message set in its config file,
  greeting.conf, read on every call. The package also ships greeting.sql,
  which creates a stored procedure calling GREETING().

  The config file is greeting_config_file, by default
  <etc>/mariadb/greeting/greeting.conf, where <etc> is /etc, or
  <basedir>/etc for a server installed from a tarball, whose plugin
  directory is <basedir>/lib/plugin. That is where Marina installs the
  etc/ files of a build.
*/

#define MYSQL_SERVER

#include <my_global.h>
#include <my_sys.h>
#include <sql_class.h>
#include <sql_plugin.h>
#include <mysql/plugin_function.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

const size_t MAX_CONFIG_BYTES= 64 * 1024;

char *config_file;
char default_config_file[FN_REFLEN];

MYSQL_SYSVAR_STR(config_file, config_file,
                 PLUGIN_VAR_READONLY | PLUGIN_VAR_RQCMDARG,
                 "The file GREETING() reads its message from. Default: "
                 "<etc>/mariadb/greeting/greeting.conf, where <etc> is "
                 "<basedir>/etc when the plugin directory is "
                 "<basedir>/lib/plugin, and /etc otherwise",
                 nullptr, nullptr, nullptr);

struct st_mysql_sys_var *greeting_variables[]= {
  MYSQL_SYSVAR(config_file),
  nullptr
};

/* Where Marina's installer puts the etc/ files of a build. */
std::string default_etc_dir()
{
  char resolved[PATH_MAX];
  std::string plugin_dir= realpath(opt_plugin_dir, resolved) ? resolved
                                                             : opt_plugin_dir;
  while (plugin_dir.size() > 1 && plugin_dir.back() == '/')
    plugin_dir.pop_back();
  std::string basedir;
  for (const char *lib : {"/lib/", "/lib64/"})
  {
    size_t at= (plugin_dir + "/").rfind(lib);
    if (at != std::string::npos && at > 0 &&
        (basedir.empty() || at > basedir.size()))
      basedir= plugin_dir.substr(0, at);
  }
  for (const char *lib : {"/lib/plugin", "/lib64/plugin"})
    if (!basedir.empty() && plugin_dir == basedir + lib)
      return basedir + "/etc";
  return "/etc";
}

int greeting_init(void *)
{
  if (!config_file || !*config_file)
  {
    std::string path= default_etc_dir() + "/mariadb/greeting/greeting.conf";
    strmake(default_config_file, path.c_str(), sizeof(default_config_file) - 1);
    config_file= default_config_file;
  }
  return 0;
}

std::string trim(const std::string &text)
{
  const char *space= " \t\r";
  size_t begin= text.find_first_not_of(space);
  if (begin == std::string::npos)
    return "";
  return text.substr(begin, text.find_last_not_of(space) - begin + 1);
}

/*
  Reads `message = TEXT` from the config file: blank lines and lines
  starting with # are ignored, and TEXT may be in double quotes.
*/
bool read_message(std::string *message, std::string *error)
{
  FILE *file= fopen(config_file, "r");
  if (!file)
  {
    *error= std::string("cannot open ") + config_file + ": " + strerror(errno);
    return false;
  }
  std::string text;
  char buffer[4096];
  size_t n;
  while ((n= fread(buffer, 1, sizeof(buffer), file)) > 0 &&
         text.size() <= MAX_CONFIG_BYTES)
    text.append(buffer, n);
  bool failed= ferror(file);
  fclose(file);
  if (failed)
  {
    *error= std::string("cannot read ") + config_file;
    return false;
  }
  if (text.size() > MAX_CONFIG_BYTES)
  {
    *error= std::string(config_file) + " is larger than 64 KiB";
    return false;
  }

  size_t start= 0;
  unsigned line_number= 0;
  while (start < text.size())
  {
    size_t end= text.find('\n', start);
    if (end == std::string::npos)
      end= text.size();
    std::string line= trim(text.substr(start, end - start));
    start= end + 1;
    ++line_number;
    if (line.empty() || line[0] == '#')
      continue;
    size_t equals= line.find('=');
    if (equals == std::string::npos)
    {
      *error= std::string(config_file) + ":" + std::to_string(line_number) +
              ": expected key = value";
      return false;
    }
    if (trim(line.substr(0, equals)) != "message")
      continue;
    std::string value= trim(line.substr(equals + 1));
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
      value= value.substr(1, value.size() - 2);
    *message= value;
    return true;
  }
  *error= std::string(config_file) + " sets no message";
  return false;
}

class Create_func_greeting : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override;
};

class Item_func_greeting : public Item_str_func
{
  using Self= Item_func_greeting;

public:
  Item_func_greeting(THD *thd) : Item_str_func(thd) {}

  bool fix_length_and_dec(THD *) override
  {
    set_maybe_null();
    collation.set(&my_charset_utf8mb4_general_ci, DERIVATION_COERCIBLE,
                  MY_REPERTOIRE_UNICODE30);
    max_length= MAX_CONFIG_BYTES * collation.collation->mbmaxlen;
    return false;
  }

  String *val_str(String *output) override
  {
    std::string message, error;
    if (!read_message(&message, &error))
    {
      push_warning_printf(current_thd, Sql_condition::WARN_LEVEL_WARN,
                          ER_UNKNOWN_ERROR, "GREETING(): %s", error.c_str());
      return null_value= true, nullptr;
    }
    if (output->copy(message.data(), message.size(),
                     &my_charset_utf8mb4_general_ci))
      return null_value= true, nullptr;
    null_value= false;
    return output;
  }

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {STRING_WITH_LEN("greeting")};
    return name;
  }

  /* The message changes with the file: not in stored or indexed columns. */
  bool check_vcol_func_processor(void *arg) override
  {
    return mark_unsupported_function(func_name(), "()", arg,
                                     VCOL_NON_DETERMINISTIC);
  }

  Item *shallow_copy(THD *thd) const override
  { return get_item_copy<Self>(thd, this); }

  static Plugin_function *plugin_descriptor()
  {
    static Create_func_greeting creator;
    static Plugin_function descriptor(&creator);
    return &descriptor;
  }
};

Item *Create_func_greeting::create_builder(THD *thd)
{
  return new (thd->mem_root) Item_func_greeting(thd);
}

} // namespace

maria_declare_plugin(greeting)
{
  MariaDB_FUNCTION_PLUGIN,
  Item_func_greeting::plugin_descriptor(),
  "greeting",
  PLUGIN_AUTHOR,
  PLUGIN_DESCRIPTION,
  PLUGIN_LICENSE_GPL,
  greeting_init,
  nullptr,
  PLUGIN_HEX_VERSION,
  nullptr,
  greeting_variables,
  PLUGIN_VERSION,
  MariaDB_PLUGIN_MATURITY_GAMMA
}
maria_declare_plugin_end;
