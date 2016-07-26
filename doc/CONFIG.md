# Apache Configuration

To load the module, add the line `LoadModule auth_grid_module modules/mod_auth_grid.so` to an appropriate Apache configuration file (probably `httpd.conf`).

Add an `AuthGridMapfile` directive plus the location of the grid-mapfile to the appropriate `<Location>` or `<Directory>` section to activate the module.

If using mod_ssl with the `SSLUserName` directive, add `AuthType Grid` to the `<Location>` or `<Directory>` section - the hook that handles the grid-mapfile lookup will not be called unless there is a valid `AuthType` directive.


## Directives

These are the directives implemented by mod_auth_grid for the Apache configuration files. They go inside `<Location>` or `<Directory>` sections.

#### AuthType Grid
Enable the dummy authentication function. For use with mod_ssl and the `SSLUserName` directive.

#### AuthGridMapfile
Location of the grid-mapfile. This directive must be specified to enable the module.

#### AuthGridUserEnvIn
Name of variable in the subprocess environment table that holds the user's DN, rather than the `user` field in the request record. For use with mod_authn_myproxy, which stores the DN in the `X509_USER_DN` variable.  This can't be used with the mod_ssl variables because they are added to the environment after this module is run.

#### AuthGridUserEnvOut
Name of environment variable to write user's DN into (i.e. the old value of `user` unless `AuthGridUserEnvIn` is used). For when the old value is still wanted for writing to the log.


## Example - X.509

```ApacheConf
# The follwing is an example configuration using X.509 authentication via
# mod_ssl:

LoadModule auth_grid_module modules/mod_auth_grid.so

SSLVerifyClient require
SSLVerifyDepth  10

SSLOptions +StdEnvVars
SSLUserName SSL_CLIENT_S_DN

<Location /castor>
	Require valid-user
	AuthType Grid
	AuthGridMapfile /etc/grid-security/grid-mapfile
</Location>
```


## Example - Myproxy

```ApacheConf
# The follwing is an example configuration using myproxy authentication via
# mod_authn_myproxy. Note that this uses HTTP basic (plaintext) authentication
# so SSL *MUST* be used:

LoadModule authn_myproxy_module modules/mod_authn_myproxy.so
LoadModule auth_grid_module     modules/mod_auth_grid.so

MyProxyCacheDir /var/cache/mod_authn_myproxy

<Location /castor>
        Require valid-user
        AuthType Basic
        AuthBasicProvider myproxy 
        AuthName "Castor WebDAV - Myproxy"
        MyProxyServer myproxy.gridpp.rl.ac.uk
        AuthGridMapfile /etc/grid-security/grid-mapfile
        AuthGridUserEnvIn X509_USER_DN
</Location>
```
