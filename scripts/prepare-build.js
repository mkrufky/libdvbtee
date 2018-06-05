var clone = require('git-clone')
var fs = require('fs')
var rimraf = require('rimraf')

var libdvbpsiDir = __dirname+'/../libdvbpsi'

var cloneLibdvbpsi = function() {
    return fs.stat(libdvbpsiDir+'/bootstrap', function(err, stats) {
        if (err) {
            return rimraf(libdvbpsiDir, function() {
                return clone('git://github.com/mkrufky/libdvbpsi.git', libdvbpsiDir, {'shallow': true}, function() {
                    return fs.writeFileSync(libdvbpsiDir+'/.dont_del', 0)
                })
            })
        }
    })
}

cloneLibdvbpsi()
