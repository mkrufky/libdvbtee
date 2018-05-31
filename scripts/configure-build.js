var fs = require('fs')
var mkdirp = require('mkdirp')

var configh = __dirname+'/../libdvbpsi/config.h'

var confighContents = '#define HAVE_SYS_TYPES_H 1\n'+
                      '#define HAVE_INTTYPES_H 1\n'+
                      '#define HAVE_STDINT_H 1\n'

var libdvbpsiSrc = __dirname+'/../libdvbpsi/src'
var libdvbpsiDst = __dirname+'/../usr/include/dvbpsi/'

// for compat with node 0.12:
if (!String.prototype.hasOwnProperty('endsWith')) {
  String.prototype.endsWith = function(suffix) {
    return this.indexOf(suffix, this.length - suffix.length) !== -1
  }
}

fs.writeFile(configh, confighContents, function (err) {
  if (err) throw err

  mkdirp(libdvbpsiDst, function (err) {
    if (err) throw err;

    var cpHeaders = function (from) {
      fs.readdir(from, function (err, files) {
        if (err) throw err;
        files.forEach(function (file) {
          if (file.endsWith('.h')) {
            fs.createReadStream(from+'/'+file).pipe(fs.createWriteStream(libdvbpsiDst+file))
          }
        })
      })
    }

    cpHeaders(libdvbpsiSrc)
    cpHeaders(libdvbpsiSrc+'/..')
    cpHeaders(libdvbpsiSrc+'/tables')
    cpHeaders(libdvbpsiSrc+'/descriptors')
  })
})
