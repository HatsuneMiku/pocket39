/*
  Chrome with --allow-file-access-from-files --unlimited-quota-for-files
  chrome://blob-internals/

  e.code
    FileError.QUOTA_EXCEEDED_ERR
    FileError.NOT_FOUND_ERR
    FileError.SECURITY_ERR
    FileError.INVALID_MODIFICATION_ERR
    FileError.INVALID_STATE_ERR

  (fileEntry or dirEntry).copyTo moveTo remove removeRecursively
  fe.file(function(f){ ... }, errProc);
  fe.toURL();

  function traverse(de){ // old style
    var reader = de.createReader();
    reader.readEntries(
      function(entries){
        var i;
        for(i = 0; i < entries.length; ++i){
          var entry = entries[i];
          printAnchor(entry.toURL());
          if(entry.isDirectory){
            traverse(entry);
          }
        }
      },
      function(e){ console.log('error: ' + e.message); }
    );
  }
*/

(function(global){
  var greqFS = global.requestFileSystem || global.webkitRequestFileSystem;
  var grLFSU = global.resolveLocalFileSystemURI || global.webkitResolveLocalFileSystemURI;
  var filesystemPack = function(loc, quota, basedir, filename,
    callback, successStartCallback, successCallback, successEndCallback,
    progressCallback, abortCallback, errorCallback){
    var atfunc = 'filesystemPack'; // arguments.callee.name;
    var errProc = function(e){ errorCallback(e, atfunc); };
    var nStorage = navigator.webkitTemporaryStorage; // without agreement
    if(loc == global.PERSISTENT)
      nStorage = navigator.webkitPersistentStorage; // requires users agreement
    if(!global.File || !nStorage)
      return errProc({code: -1, message: 'cannot use File API'});
    atfunc = 'nStorage.requestQuota';
    nStorage.requestQuota(quota, function(grantedBytes){
      atfunc = 'greqFS';
      greqFS(loc, grantedBytes, function(fs){
        atfunc = 'getDirectory';
        fs.root.getDirectory(basedir, {create: true}, function(de){
          atfunc = 'getFile';
/*
onerror     onerror
onabort     onabort
onprogress  onprogress
onloadend   onwriteend
onload      onwrite
onloadstart onwritestart
*/
/**/
          de.getFile(filename, {create: true, exclusive: false}, function(fe){
            atfunc = 'createWriter';
            fe.createWriter(function(fw){
              atfunc = 'writeCallbacks';
              fw.onerror = errProc;
              fw.onabort = function(e){ abortCallback(e, atfunc); };
              fw.onprogress = function(e){ progressCallback(e); };
              fw.onwriteend = function(){ successEndCallback(fw); };
              fw.onwrite = function(){ successCallback(fw); };
              fw.onwritestart = function(){ successStartCallback(fw); };
              callback(fw);
            }, errProc);
          }, errProc);
/**/
/*
          de.getFile(filename, {create: false}, function(fe){
            atfunc = 'remove';
            fe.remove(function(){
              atfunc = 'removeCallbacks';
              console.log('erase OK');
            }, errProc);
          }, errProc);
*/
/*
          atfunc = 'remove';
          de.remove(function(){
            atfunc = 'removeCallbacks';
            console.log('erase OK');
          }, errProc);
*/
        }, errProc);
      }, errProc);
    }, errProc);
  };

  global.filesystemPack = filesystemPack;

}(window));
