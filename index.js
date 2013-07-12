fs = require("fs"),
mmap = require("./build/Release/mmap"), real_map = mmap.map;

function mmap_wrapper(size, protection, flags, fd, offset) {
  if(typeof fd === "number" || fd instanceof Number)
    return real_map(size,protection,flags,fd,offset);

  var writep = (protection & mmap.PROT_WRITE);
  if(writep) fs.closeSync(fs.openSync(fd, "a"));
  fd = fs.openSync(fd, writep? "r+" : "r");
  var stat = fs.fstatSync(fd);
  if(!size) {
    size = stat.size;
  } else if (writep && size > stat.size) {
    fs.ftruncateSync(fd, size); // extend if needed
  }

  var buffer = real_map(size, protection, flags, fd, offset);
  fs.closeSync(fd);
  return buffer;
};
for(var k in mmap) mmap_wrapper[k] = mmap[k];
mmap_wrapper.map = mmap_wrapper;

module.exports = mmap_wrapper;
