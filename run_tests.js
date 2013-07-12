tempfile=(+new Date()) + "." + process.pid + ".tmp",
assert=require("assert"), fs=require("fs"), mmap = require("./index")

var t = 128 | (Math.random() * 256);
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
  assert.equal(b[1],     0,             "value check (line noise)");
  assert.equal(b[2],     0,             "value check (line noise)");
  assert.equal(b.sync(), true,          "sync failed");

  assert.equal(b.length, mmap.PAGESIZE, "map length is incorrect");

  assert.equal(b.unmap(), true,         "unmap failed");
  assert.equal(b.length, 0, "unmap truncated fixed buffer");

})( mmap.map(mmap.PAGESIZE,
  mmap.PROT_READ|mmap.PROT_WRITE,
  mmap.MAP_SHARED, 
  fd) );

fs.truncateSync(fd, 0);
fs.truncateSync(fd, mmap.PAGESIZE);
(function(b) {
  assert.equal(b[0],     0,             "value check (truncated 0)");
  assert.equal(b[1],     0,             "value check (always zero)");
  assert.equal(b[2],     0,             "value check (always zero)");
  assert.equal(b.length, mmap.PAGESIZE, "map length is incorrect");

  b[2] = t;
  assert.equal(b.sync(), true,          "sync failed");

})( mmap.map(mmap.PAGESIZE,
  mmap.PROT_READ|mmap.PROT_WRITE,
  mmap.MAP_SHARED, 
  fd) );


(function(b) {
  fs.closeSync(fd);

  assert.equal(b[0],     0,             "value check (truncated 0)");
  assert.equal(b[1],     0,             "value check (always zero)");
  assert.equal(b[2],     t,             "third mapping");
  assert.equal(b.length, mmap.PAGESIZE, "map length is incorrect");

})( mmap.map(mmap.PAGESIZE,
  mmap.PROT_READ,
  mmap.MAP_SHARED, 
  fd) );
console.log("> ok");
