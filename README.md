# mod_auth_grid

mod_auth_grid is an Apache httpd module that provides secondary grid-mapfile authentication. It was designed for use with a primary authentication module (e.g. mod_auth_kerb) or with mod_ssl using the `SSLUserName` directive.

The module has two parts:
 1. A dummy authentication function for use when mod_ssl has already set the `user` field of the request record to the DN of the user's certificate. It does nothing except make Apache thinks an authentication module has been run - this is necessary for running authorisation modules. It is activated using the `AuthType Grid` directive.
 2. An authorisation function that changes the value of the `user` field in the Apache request record by looking it up in a grid-mapfile. It runs as the first authentication module so that other modules will use the new value of `user`. It is activated with the `AuthGridMapfile` directive, which takes the location of the grid-mapfile as its argument.


## Installing

To install the module:

```sh
cd src
make
sudo make install
```


## Apache Configuration

To load the module, add the line `LoadModule auth_grid_module modules/mod_auth_grid.so` to an appropriate Apache configuration file (probably `httpd.conf`).

Add an `AuthGridMapfile` directive plus the location of the grid-mapfile to the appropriate `<Location>` or `<Directory>` section to activate the module.

If using mod_ssl with the `SSLUserName` directive, add `AuthType Grid` to the `<Location>` or `<Directory>` section - the hook that handles the grid-mapfile lookup will not be called unless there is a valid `AuthType` directive.


## Documentation

The following can be found in the `doc` directory:
 - [CONFIG.md](doc/CONFIG.md): How to configure Apache to use this module, and the directives for the Apache config files implemented by this module.
 - [RPM.md](doc/RPM.md): A guide to making an RPM for the module.
