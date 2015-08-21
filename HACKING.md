# Working on schroot

A short guide to the conventions used in the schroot project.


## Building from git

First, clone the git repositories:

```
% git clone https://github.com/codelibre-net/schroot.git
% cd schroot
% git remote add origin-dist1x https://github.com/codelibre-net/schroot-dist-1x.git
% git fetch origin-dist1x
```

`schroot-dist1x` is only required if you want to run `make git-dist`;
this repository is where the distribution branches are stored (from
which the distribution tarballs are created with `git archive`).  See
"Releasing", below.

## Coding style

The style should be apparent from the source.  It is the default Emacs
`c++-mode` style, with paired brackets aligned vertically.

- Use `0` rather than `NULL`.
- Use C++ casts rather than C-style casts.
- Don't use `void *` unless there is no alternative.
- Add Doxygen comments for everything; use `EXTRACT_ALL = NO` in
  `doc/schroot.dox` to check for missing or incomplete documentation.

## Format strings

The sources use `boost::format` for type-safe formatted output.  Make
sure that the maximum number of options passed is the same as the
highest `%n%` in the format string.  UTF-8 string literals are used in
the sources.

The following styles are used:

Style        | Formatting    | Syntax
-------------|---------------|-------
Values       | Single quotes | `‘’`
Example text | Double quotes | `“”`
User input   | Double quotes | `“”`

## Documentation

All the documentation is in UNIX manual page format.  GNU roff
(`groff`) extensions are permitted, as is use of `tbl`.  Make sure the
printed output is as good as terminal display.  Run `make ps` or `make
pdf` to build the printed documentation.

The following styles are used:

Style                 | Formatting               | Syntax
----------------------|--------------------------|----------------------
New term              | Bold                     | `.B or \fB`
Option definition     | Bold, args in italic     | `.BR and \fI`
Option reference      | Italic                   | `.I or \fI`
File definition       | Bold italic              | `\f[BI]`
File reference        | Italic                   | `.I or \fI`
Config key definition | Courier bold italic      | `\f[CBI]`
Config key reference  | Courier italic           | `\f[CI]`
Values                | Single quotes            | `\[oq] and \[cq]`
Example text          | Double quotes            | `\[lq] and \[rq]`
Cross references      | Italics in double quotes | `\[lq]\fI...\fP\[rq]`
Verbatim examples     | Courier                  | `\f[CR]`
Verbatim user input   | Courier bold             | `\f[CB]`

## Releasing

### Test conformance

The code must pass the testsuite:

```
% fakeroot ctest -V
```

It must also pass some tests which must be run by hand:

### Chdir fallback behaviour

Fallback behaviour has been documented in the manual pages.

Note that `--debug=notice` will show the internal fallback list
computed for the session.

### Setup script behaviour

To check if process termination works:

```
schroot -v -c sid -- sh -c "trap '' INT; trap '' TERM; sleep 2 &"
```

To check if process killing works:

```
schroot -v -c sid -- sh -c "trap '' INT; trap '' TERM; sleep 20 &"
```

### Release process

In order to make a release, the following must be done:

- Use a fresh clone or make sure the tree is pristine
- Make sure all generated files are up to date.  Run `make`
  and/or `make update-po` to update all translations.
  Commit any changes.
- Ensure that the distribution branch is branched locally.  For
  example, if making a 1.6.x release, checkout
  `origin-dist1x/distribution-1.6` as `distribution-1.6` e.g.

  ```
  % git checkout -b distribution-1.6 origin-dist1x/distribution-1.6
  ```

  If this is the first release in a stable series, e.g. 1.6.0,
  the local branch will be created automatically.
- Make the release.  Run `cmake` as normal, but add the options:

  ```
  -DGIT_RELEASE_ENABLE=ON -DGIT_RELEASE_VERSION=${new_version}
  ```

  This will provide a new `git-release` target.  If using make, run:

  ```
  % make git-release
  ```

  You will be prompted for your GPG key passphrase in order to
  sign the release tag (`release/schroot-$version`).
- Double check that there are no modifed files.  If there are,
  delete the release tag and start again.
- Make the distribution.  Run `cmake` as normal, but add the option:

  ```
  -DGIT_DIST_ENABLE=ON
  ```

  This will provide a new "git-dist" target.  If using make, run:

  ```
  % make git-dist
  ```

  You will again be prompted for your GPG key passphrase, to sign the
  distribution tag (`distribution/schroot-$version`).  This will also
  inject the distributed release onto the distribution-x.y branch.
  Verify with `gitk distribution-x.y` that the distribution is sane,
  and that the history ties in sanely with the previous distributions
  and releases via the distribution commit's parents.  If there's a
  mistake, delete the distribution tag and `git reset --hard` the
  `distribution-x.y` branch to its previous state.
- Make the distributed release tarball:

  ```
  git archive --format=tar --prefix=schroot-$version/ distribution/schroot-$version | xz --best > schroot-$version.tar.xz
  ```

This creates a tarball from the release tag.
- Push all branches and tags to the correct places:

  ```
  % git push origin $branch
  % git push origin release/schroot-$version
  % git push origin-dist1x distribution-x.y
  % git push origin-dist1x distribution/schroot-$version
  ```
