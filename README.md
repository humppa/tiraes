
# Tiraes

[spec]: http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
[padd]: http://en.wikipedia.org/wiki/ISO/IEC_9797-1#Padding
[spid]: http://cr.yp.to/aes-speed.html
[mode]: http://en.wikipedia.org/wiki/Block_cipher_mode_of_operation

A naïve implementation of [AES algorithm][spec] in ANSI C99.

List of features:
 + AES cipher and inverse cipher
 + Support for 128-bit key
 + ISO/IEC 9797-1 [padding][padd]

Notable missing features:
 - Support for 192-bit key (untested)
 - Support for 256-bit key (untested)
 - [Speed optimizations][spid] for 32-bit platform
 - Support for [counter mode of operation][mode] (CTR)
 - Support for [cipher-block chaining][mode] (CBC)
 - Support for [output feedback][mode] (OFB)
 - Protection against cache timing attacks

**Be advised that tiraes is completely unsuitable for any
real-world application.**

## Compiling

```
$ make
```

Above command should be enough. Please, note that compiling requires
an ANSI C compiler and some standard tools such as *make*(1).

## Usage

*Tiraes* has a single executable, which does encryption and decryption.
In both cases input is always read from *stdin* and result is written
to *stdout*. Also, a secret key must be provided.

Following command line options are recognized:

<table>
<tr>
<td>-d</td><td>Decrypt.</td>
</tr>

<tr>
<td>-e</td><td>Encrypt. This is the default.</td>
</tr>

<tr>
<td>-⁠f file</td>
<td>
Read key from file. File is expected to be exactly 16 bytes
long and the content is used without any conversion.
</td>
</tr>

<tr>
<td>-⁠k key</td>
<td>
Read key from command line. The key argument must have 32 hexadecimal
characters which are translated to 128-bit key. Any space, dot, colon
and dash character is silently ignored and those may be used as
delimiters for improved readability.
</td>
</tr>
</table>

## Examples

To encrypt a file:
```
$ tiraes -k 2b7e1516.28aed2a6.abf71588.09cf4f3c < file.plain > file.crypt
```

To decrypt and display previously crypted file:
```
$ tiraes -d -k 2b7e1516.28aed2a6.abf71588.09cf4f3c < file.crypt
```

## License

Copyright (c) 2013 Tuomas Starck

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
