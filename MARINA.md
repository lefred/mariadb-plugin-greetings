# INSTALL

Using the traditional method:

```sql 
INSTALL SONAME 'greeting';
```

Using Marina:

```sql
SELECT marina_install('greering@community');
```

The plugin provides a script to install a stored procedure:

```sh
mariadb < /usr/share/mariadb/greeting/greeting.sql
```


## Usage

```sql
SELECT GREETING();
```

Calling the store procedure:

```sql
CALL greeting.say_hello();
```
