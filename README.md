# mmap

mmap(2) bindings for node.js that work in 0.10

## Installing

    npm install -g

## Usage

    mmap = require("mmap")
    buffer = mmap.map(n_bytes, protection, flags, fd, offset)

While a finaliser is installed to automatically unmap buffer, you can
force it to unmap immediately with:

    buffer.unmap()

## See Also

* POSIX 1003.1 [mmap](http://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html)
* [bnoordhuis/node-mmap](https://github.com/bnoordhuis/node-mmap), the original node-mmap, on which this is based.
