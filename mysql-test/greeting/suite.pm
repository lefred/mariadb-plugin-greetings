package My::Suite::Greeting;

@ISA = qw(My::Suite);

return "No GREETING plugin" unless $ENV{GREETING_SO};

return "Not run for embedded server" if $::opt_embedded_server;

push @::global_suppressions,
  (
    qr/Plugin 'greeting' is of maturity level gamma while the server is/,
  );

# greeting.sql: in the plugin's sources, two levels above this suite, or
# where the package installed it.
use File::Basename;
my $suitedir = dirname(__FILE__);
($ENV{GREETING_SQL}) = grep { -f $_ } (
  "$suitedir/../../greeting.sql",
  "$::basedir/share/mariadb/greeting/greeting.sql",
  "/usr/share/mariadb/greeting/greeting.sql");
$ENV{GREETING_SQL} //= '';

sub is_default { 1 }

bless { };
