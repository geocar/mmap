tempfile=(+new Date()) + "." + process.pid + ".tmp",
assert=require("assert"), fs=require("fs"), mmap = require("./build/Release/mmap");

var t = 0 | (Math.random() * 256);
var b = new Buffer(1);
b[0] = t;

fs.writeFileSync(tempfile, b);
process.on("exit", function() {
  fs.unlink(tempfile);
});

fd = fs.openSync(tempfile, "r+");
fs.truncateSync(fd, mmap.PAGESIZE);

(function(b) {
  assert.equal(b[0],     t,             "value check (map read)");
  assert.equal(b.sync(), true,          "sync failed");

  assert.equal(b.length, mmap.PAGESIZE, "map length is incorrect");

  assert.equal(b.unmap(), true,         "unmap failed");
  assert.equal(b.length, 0, "unmap truncated fixed buffer");

})( mmap.map(mmap.PAGESIZE,
  mmap.PROT_READ|mmap.PROT_WRITE,
  mmap.MAP_SHARED, 
  fd) );

console.log("> ok");
