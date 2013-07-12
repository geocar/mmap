fs = require("fs"),
mmap = require("../index.js"); // obv. use require("mmap") if you lift
HASH_FUNCTIONS=7; // how many hashes do you want?

size = 1024*1024; // 1mb
buffer = mmap.map(size, mmap.PROT_READ|mmap.PROT_WRITE, mmap.MAP_SHARED, "/tmp/test_bloom");
add(buffer, get_offsets("testing", buffer.length));
console.log("check: ", check(buffer, get_offsets("testing", buffer.length)));


/* basic bloom filter tricks */
function get_offsets(string, buffer_size) {
  var a = fnv_1a(string);
  var b = fnv_1a_b(a);
  var r = [];
  var i = -1;
  var x = a % buffer_size;
  while (++i < HASH_FUNCTIONS) {
    r.push(x < 0 ? (x + buffer_size) : x);
    x = (x + b) % buffer_size;
  }
  return r;
}
function add(buffer, offsets) {
  var i = -1;
  var w = 0;
  while (++i < HASH_FUNCTIONS) w += !!(buffer[ offsets[i] ]++);
  if (HASH_FUNCTIONS == w) {
    while (i-->0) --buffer[offsets[i]];
    return true;
  }
  return false;
}
function check(buffer, offsets) {
  var i = -1;
  while (++i < HASH_FUNCTIONS)
    if (!buffer[offsets[i]])
        return false;
  return true;
}

/* fowler-noll-vo (fnv) hashing */
function fnv_1a(v) {
  var n = v.length, a = 2166136261, c, d, i = -1;
  while (++i < n) {
    c = v.charCodeAt(i);
    if (d = c & 0xff000000) {
      a ^= d >> 24;
      a += (a << 1) + (a << 4) + (a << 7) + (a << 8) + (a << 24);
    }
    if (d = c & 0xff0000) {
      a ^= d >> 16;
      a += (a << 1) + (a << 4) + (a << 7) + (a << 8) + (a << 24);
    }
    if (d = c & 0xff00) {
      a ^= d >> 8;
      a += (a << 1) + (a << 4) + (a << 7) + (a << 8) + (a << 24);
    }
    a ^= c & 0xff;
    a += (a << 1) + (a << 4) + (a << 7) + (a << 8) + (a << 24);
  }
  /* from http://home.comcast.net/~bretm/hash/6.html */
  a += a << 13;
  a ^= a >> 7;
  a += a << 3;
  a ^= a >> 17;
  a += a << 5;
  return a & 0xffffffff;
}

/* one additional iteration of fnv, given a hash */
function fnv_1a_b(a) {
  a += (a << 1) + (a << 4) + (a << 7) + (a << 8) + (a << 24);
  a += a << 13;
  a ^= a >> 7;
  a += a << 3;
  a ^= a >> 17;
  a += a << 5;
  return a & 0xffffffff;
}


