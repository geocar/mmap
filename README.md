# mmap

mmap(2) bindings for node.js that work in 0.10

## Installing

    npm test
    npm install -g

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

If you wish to use [shared memory](http://en.wikipedia.org/wiki/Shared_memory#In_software) instead of a file, you can do so by calling: `mmap.map_shm(n_bytes, protection, flags, name, offset)` method.  All arguments are the same as <b>mmap()<b> except:

<table>
  <tr>
    <td><i>name</i></td>
    <td>Name of the <a href="http://man7.org/linux/man-pages/man7/shm_overview.7.html">POSIX shared memory</a> region.  <b>mmap.map_shm()</b> will create and/or attach to the region with read and write permissions, and truncate or expand the region to <b>size</b> bytes.</td>
  </tr>
</table>

## See Also

* POSIX 1003.1 [mmap](http://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html) and [msync](http://pubs.opengroup.org/onlinepubs/9699919799/functions/msync.html)
* The `example/` directory contains some sample uses of the mmap module
* [bnoordhuis/node-mmap](https://github.com/bnoordhuis/node-mmap), the original node-mmap, on which this is based.
