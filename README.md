# mariadb-plugin-greeting

A sample MariaDB plugin that ships more than a library: a config file and a
SQL script. It shows what a Foundry build can carry, and where Marina
installs each part.

- `GREETING()` returns the message set in the config file, `greeting.conf`.
  The plugin reads the file on every call, so an edit applies at once.
- `greeting.sql` creates the stored procedure `greeting.say_hello()`, which
  calls `GREETING()`.

## Files

| File | tar.gz (server basedir) | rpm / deb |
|------|-------------------------|-----------|
| `greeting.so` | `lib/plugin/` | the server's plugin directory |
| `greeting.conf` | `etc/mariadb/greeting/` | `/etc/mariadb/greeting/` |
| `greeting.sql` | `share/mariadb/greeting/` | `/usr/share/mariadb/greeting/` |

`etc/mariadb/<name>/` and `share/mariadb/<name>/` are where Marina's
installer lets a build of any trust level write. A community build of this
plugin installs completely. Marina treats `greeting.conf` as a config file:
an update keeps your edited copy and writes the new one as
`greeting.conf.marinanew`. An uninstall keeps an edited copy as
`greeting.conf.marinasave`. The rpm marks it `%config(noreplace)`, and the
deb lists it as a conffile.

## Config file

```ini
# Blank lines and lines starting with # are ignored.
message = Hello from the greeting sample plugin!
```

The value can be in double quotes, to keep leading or trailing spaces.
Other `key = value` lines are ignored.

When the file cannot be read, has no `message`, or has a line that is not
`key = value`, `GREETING()` returns NULL with a warning that says why.

The file is `greeting_config_file`, a read-only option:

```ini
[mariadb]
greeting_config_file = /path/to/greeting.conf
```

By default it is `<etc>/mariadb/greeting/greeting.conf`. `<etc>` is
`<basedir>/etc` when the plugin directory is `<basedir>/lib/plugin` (or
`lib64/plugin`), as in a tarball install, and `/etc` otherwise. That is the
same rule Marina's installer uses for a build's `etc/` files.

## Use

```sql
INSTALL SONAME 'greeting';
SELECT GREETING();
```

```sh
mariadb < /usr/share/mariadb/greeting/greeting.sql
```

```sql
CALL greeting.say_hello();
```

The procedure is `SQL SECURITY INVOKER`. `GREETING()` cannot be used in a
stored or indexed generated column, because its value changes with the
file.

## Build

With the Foundry's CMake package (`MariaDB-devel` or `libmariadb-dev`
installed, or `CMAKE_PREFIX_PATH` pointing at a server install):

```sh
cmake -S . -B build              # add -DRPM=1 or -DDEB=1 for packages
cmake --build build --target package
```

`.github/workflows/build-foundry.yml` builds the deb, rpm and tar.gz
packages with `lefred/mariadb-plugin-ci`, runs the `greeting` MTR suite, and
publishes them on a `v*` tag.

The plugin can also be built in a server source tree, at `plugin/greeting`.

## Tests

```sh
./mtr --suite=greeting
```

`script.test` runs `greeting.sql`. It finds the script in these sources, or
where a package installed it. Without the script, the test is skipped.
