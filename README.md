# `sockexec`

`sockexec` is a small server for executing local processes. It accepts
connections on a UNIX socket, launches programs, and sends program output
to the connected client.

It's similar to [sockproc](https://github.com/juce/sockproc) but with a few changes:

* Instead of sending a line to run in a shell, you send an array of program arguments. No shell is launched (unless you explicitly want to).
* Input/output is communicated via [netstring](https://cr.yp.to/proto/netstrings.txt)
* Single-threaded, non-blocking
* Enforces connection limits and timeouts/deadlines.

## Installation

### Quickest and easiest install

Just download a statically-compiled binary from the releases page.

### Quick and easy install

`sockexec` requires the `skarnet` library from [skalibs](http://skarnet.org/software/skalibs/)

You can use the standard `configure` process:

```
./configure
make
make install
```


## Usage

`sockexec [-v] [-d] [-q max queue] [-m max connections] [-t timeout] [-k kill_timeout] /tmp/exec.sock`

This starts up `sockexec`, listening for connections at `/tmp/exec.sock`

By default, `sockexec` accepts a maximum of 100 queued connections, 100
active connections, and enforces a 60-second max connection time. This
can be changed with the `-q`, `-m` and `-t` flags, respectively.

Setting the max connection time to 0 allows processes to run indefinitely. When
a client disconnects, `sockexec` will send a `TERM`, then wait up to
`kill_timeout` seconds before sending a `KILL` signal (default: 10 seconds).
This can be changed with the `-k` flag.

The `-d` flag enables a debug mode. By default, `sockexec` only prints warnings
and errors. The debug flag will allow you to track what `sockexec` is up to -
accepting connections, reading data, etc.

The `-v` flag causes `sockexec` to dump the version number and exit. Only available
starting with version 1.2.0.

## Running as a service

If your distro is using `systemd`, create a file at `/etc/systemd/system/sockexec.service` with
something like the following - adjust your paths as necessary.

```
[Unit]
description=sockexec
After=network.target

[Service]
ExecStart=/opt/sockexec/bin/sockexec /tmp/exec.sock
User=nobody

[Install]
WantedBy=multi-user.target
```

Additionally, this will run fine under `s6`, `upstart`, `launchd`, `runit`, and
`supervisor` without any real special setup - those init systems expect/prefer
processes to run in the foreground.

If your init system expects double-forking/backgrounding, PID files, etc, you *can*
use a utility like `start-stop-daemon`, but I recommend against it. Instead,
setup s6, runit, or supervisor as a service, and have that supervise sockexec.

## Protocol

### Input

After connecting, `sockexec` expects a minimum of 3 strings:

* One string for the number of program arguments (minimum 1)
* One or more strings of program arguments
* Zero or more strings of standard input
* A 0-byte termination string

All strings need to be encoded as netstrings. Some examples:

* `1:2,5:uname,2:-a,0:,`
    * A string indicating you have two arguments, the arguments themselves (`uname` and `-a`); and the zero-byte termination string

* `1:1,3:cat,5:hello,0:,`
    * A string indicating you have one argument, the argument (`cat`), a string to send to standard input (`hello`), and the zero-byte termination string.

`sockexec` will launch the program as soon as its done reading your argument
strings. It will continue reading strings for the program's standard input until
it receives the 0-byte termination string. See `src/example-cat.pl` for an
example of using `cat` as a sort of echo server.

### Output

Once `sockexec` launches your program, it will start sending pairs of
netstrings to the connected client. Within each pair, the first netstring
indicates the type of data, and the second is the actual data.

There's four kinds of data that `sockexec` will send:

* `stdout` - data from the program's standard output
* `stderr` - data from the program's standard error
* `exitcode` - the program's exit code (only for normal exits)
* `termsig` - the signal code that terminated the program (abnormal exits)
* `unknown` - if this happens, a real error has occured - `sockexec` was unable to determine if output was from `stdout` or `stderr`, please send in a bug report!

An example:

```
6:stdout,91:Linux jprjr 4.6.5-1 #1 SMP PREEMPT Thu Jul 28 21:58:03 UTC 2016 x86_64 GNU/Linux
,8:exitcode,1:0,
```

This has two pairs of netstrings:

* The first pair is data from `stdout`
* The second pair is the program's exit code

## Copyright and License

This program is licensed under the ISC license (see `LICENSE`).

This program includes modified code from skalibs (`src/child_spawn3.c`),
which is copyright Laurent Bercot. The original text of the skalibs license is
included as `LICENSE.skalibs`.

Static binaries compiled with `musl-libc` are available on the releases page.
`musl-libc` is copyright Rich Felker, et al and released under an MIT license.
The original text of the `musl-libc` license is included in the static
downloads.

