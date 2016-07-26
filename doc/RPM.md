# Creating An RPM

These instructions are inteded for use on 64-bit RedHat Enterprise Linux 5/6 and their derivatives (e.g. Scientific Linux).


## Prerequisits

  - Install the `http-devel`, `gcc`, `make` & `rpmbuild` packages.
  - Create a copy of this repository in a directory named `mod_auth_grid-<version>`.
  - Create a tarball of that directory named `mod_auth_grid-<version>.tar.gz`.


## Creating an RPM build environment

To create an RPM build environment in `~/rpmbuild/`:

```sh
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
echo %_topdir ~/rpmbuild > ~/.rpmmacros
```


## Creating the RPM

  - Put the tarball in the `SOURCES` directory.
  - Copy the spec file from the `rpm` directory of the source tree to the `SPECS` directory. Change the `Version` field; you may also want to change the `Release` field.
  - Run `rpmbuild -ba <specfile>` to create the binary and source RPMs.
