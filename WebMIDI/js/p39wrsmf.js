/*
  Record to SMF file

  (DataStream.js)
*/

(function(global){
  var gURL = global.URL || global.webkitURL;

  var download = null;
  var downloadURL = null;
  var progress = null;
  var percent = null;
  var updateProgress = function(e){
    if(percent){
      if(e && e.lengthComputable){
        var pct = Math.round(100.0 * e.loaded / e.total);
        if(pct <= 100.0){
          var msg = '' + pct + '% : ' + e.loaded + ' / ' + e.total + ' bytes';
          percent.style.width = '' + pct + '%';
          percent.textContent = 'processing ' + msg;
        }
      }else{
        percent.style.width = '0%';
        percent.textContent = 'processing 0%';
      }
    }
  };

  var RAWSMFwriter = function(rsmf, smfv, uary){ // rsmf 0: RAW, 1: SMF (smfv)
    var basedir = rsmf ? 'SMFdata' : 'RAWdata';
    var filename = rsmf ? 'sample.mid' : 'sample.dat';
/*
    var datatype = 'text/plain;charset=UTF-8';
    var datatowrite = ['test', 'UTF8日本語テスト\n'];
*/
    var datatype = 'application/octet-stream'; // octet-binary
    var datatowrite = [];
    if(rsmf){
      datatowrite = datatowrite.concat(new Uint8Array([
        0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06])); // SMF head half
      if(smfv > 0){
        datatowrite = datatowrite.concat(new Uint8Array([
          0x00, 0x01, 0x00, 0x02, 0x00, 0x78])); // SMF: 1, tracks: 2, q: 120
        datatowrite = datatowrite.concat(new Uint8Array([
          0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x0B, // SMFv1 first track
          0x00, 0xFF, 0x51, 0x03, 0x07, 0xA1, 0x20, // tempo: 120
          0x00, 0xFF, 0x2F, 0x00]));
      }else{
        datatowrite = datatowrite.concat(new Uint8Array([
          0x00, 0x00, 0x00, 0x01, 0x00, 0x78])); // SMF: 0, tracks: 1, q: 120
      }
      var len = uary.length + 4;
      datatowrite = datatowrite.concat(new Uint8Array([
        0x4D, 0x54, 0x72, 0x6B,
        (len >> 24) & 0x0FF, (len >> 16) & 0x0FF,
        (len >> 8) & 0x0FF, len & 0x0FF]));
      datatowrite = datatowrite.concat(uary);
      datatowrite = datatowrite.concat(new Uint8Array([
        0x00, 0xFF, 0x2F, 0x00]));
    }else{
      datatowrite = datatowrite.concat(uary);
    }

    filesystemPack(
      global.TEMPORARY, // region ( global.TEMPORARY or global.PERSISTENT )
      4096 * 1024, // allocation size
      basedir,
      filename,
      function(fw){ // callback
        fw.truncate(0);
      },
      function(fw){ // successStartCallback
        if(!progress) progress = document.getElementById('progress_bar');
        if(progress) progress.className = 'loading';
        if(!percent) percent = document.querySelector('.percent');
        console.log('Start');
      },
      function(fw){ // successCallback
        console.log('OK');
      },
      function(fw){ // successEndCallback
        // called twice by fw.truncate(0);
        if(fw.length === 0){
          updateProgress(null);
          if(0){ // old style
            var blobdata = new BlobBuilder();
            var b = blobdata.getBlob(datatype);
            blobdata.append(datatowrite.join(''));
            fw.write(b);
            console.log(gURL.createObjectURL(b));
          }else{
/*
            var a = [];
            for(var b = 0; b < 2; ++b)
              a = Array.prototype.concat(a, datatowrite);
            var blobdata = new Blob(a, {type: datatype});
*/
            var blobdata = new Blob(datatowrite, {type: datatype});
            fw.write(blobdata);
            // downloadURL = fw.totURL(); // only for reader ?
            downloadURL = gURL.createObjectURL(blobdata);
            console.log(downloadURL);
            // call later gURL.revokeObjectURL(downloadURL);
          }
          console.log('1');
        }else{
          if(!download) download = document.getElementById('download');
          if(download){
            var txt = '<a href="' + downloadURL + '" target="_blank"';
            txt += ' download="' + filename + '">' + filename + '</a>';
            download.innerHTML = downloadURL ? txt : '';
          }
          if(progress)
            setTimeout(function(){ progress.className = ""; }, 3000);
          console.log('2'); // before overwritten with blob
        }
        console.log('Done');
      },
      function(e){ // progressCallback
        updateProgress(e);
      },
      function(e, atfunc){ // abortCallback
        console.log('abort: ' + e.message + ' [at: ' + atfunc + ']');
      },
      function(e, atfunc){ // errorCallback
        console.log('error: ' + e.message + ' [at: ' + atfunc + ']');
      }
    );
  };

  global.RAWSMFwriter = RAWSMFwriter;

}(window));
