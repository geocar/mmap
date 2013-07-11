lru = [],
cache = {},
parseurl = require("url").parse,
fs = require("fs"),
mmap = require("../build/Release/mmap"); // obv. use require("mmap") if you lift

require("http").createServer(handler).listen(8080, function() {
  console.log("> http ready on http://localhost:8080/");
});


function handler(req,res) {
  var fd, path = parseurl(req.url).pathname;

  try {
    fd = fs.openSync(__dirname + path, "r");
  } catch(e) {
console.log(" error = ",e);
    res.writeHead(404, "File not found");
    return res.end("404 not found");
  }
  var stat = fs.fstatSync(fd);
  if(stat.isDirectory()) {
    fs.closeSync(fd);
    return serve_dir();
  }

  if(cache[path] && cache[path].length == stat.size) return serve_buffer();

  cache[path] =  mmap.map(stat.size, mmap.PROT_READ, mmap.MAP_SHARED, fd);
  fs.closeSync(fd);
  return serve_buffer();

  function serve_buffer() {
    var buffer = cache[path];
    res.writeHead(200, "OK", {"Content-Type":"text/plain"});
    res.end(buffer);
    dolru();
  }
  function serve_dir() {
    res.writeHead(200, "OK", {"Content-Type":"text/html"});
    res.end("<ul>" + fs.readdirSync(__dirname + path).map(function(x) {
      return '<li><a href="' + html(x) + '">' + x +'</a></li>';
    }).join("") + "</ul>");
  }
  function dolru() {
    // simple lru handling
    lru.unshift(path);
    while (lru.length >= 100) {
      var lp = lru.pop();
      if(lru.indexOf(lp) === -1) {
        delete cache[lp];
      }
    }
  }
}
function html(x) {
  x = "" + x;
  return x.replace(/\&/g, "&amp;").replace(/\"/g, "&quot;").replace(/</g, "&lt;");
}

