# mmap

mmap(2) bindings for node.js that work in 5.00

## Usage

    mmap = require("mmap")
    buffer = mmap(n_bytes, protection, flags, fd, offset)

<table>
  <tr>
    <td><i>n_bytes</i></td>
    <td>The number of bytes to map into memory.</td>
  </tr>
  <tr>
    <td><i>protection</i></td>
    <td>Memory protection: either <b>mmap.PROT_NONE</b> or a bitwise OR of <b>mmap.PROT_READ</b>, <b>mmap.PROT_WRITE</b> and <b>mmap.PROT_EXEC</b>.</td>
  </tr>
  <tr>
    <td><i>flags</i></td>
    <td>Flags: either <b>mmap.MAP_SHARED</b> or <b>mmap.MAP_PRIVATE</b>.</td>
  </tr>
  <tr>
    <td><i>fd</i></td>
    <td>File descriptor. You can also use a file name (string). When you do this, <b>mmap()</b> tries to do the right thing by creating or growing the file to <i>n_bytes</i> if necessary.</td>
  </tr>
  <tr>
    <td><i>offset</i></td>
    <td>File offset. Must be either zero or a multiple of <b>mmap.PAGESIZE</b>.</td>
  </tr>
</table>

While a finaliser is installed to automatically unmap buffer, you can
force it to unmap immediately with:

    buffer.unmap()

You can also [msync()](http://pubs.opengroup.org/onlinepubs/9699919799/functions/msync.html) by calling: `buffer.sync([offset, [length, [flags]]])` method:

<table>
  <tr>
    <td><i>offset</i></td>
    <td>The start address to sync. This argument optional, the default is 0.</td>
  </tr>
  <tr>
    <td><i>length</i></td>
    <td>The number of bytes to sync. This argument optional, the default is the entire buffer.</td>
  </tr>
  <tr>
    <td><i>flags</i></td>
    <td>Flags: either <b>mmap.MS_SYNC</b> or <b>mmap.MS_ASYNC</b> optionally bitwise OR with <b>mmap.MS_INVALIDATE</b>. This argument is optional, the default is <b>mmap.MS_SYNC</b>.</td>
  </tr>
</table>

For compatibility, <b>mmap.map()</b> is an alias for <b>mmap()</b>

## See Also

* POSIX 1003.1 [mmap](http://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html) and [msync](http://pubs.opengroup.org/onlinepubs/9699919799/functions/msync.html)
* The `example/` directory contains some sample uses of the mmap module
