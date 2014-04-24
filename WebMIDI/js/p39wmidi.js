(function(global){

  var c39normal = '#66DDCC';
  var c39hilight = '#99FFEE';
  var c39hover = '#66CCDD';
  var c39gray = '#333333';
  var c39white = '#FFFFFF';
  var c39error = '#FF3333';
  var kb_oct = 7;
  var kb_notes_len = kb_oct * 2 + 2;
  var kb_notes_first = 3; // F
  var kb_gif = './img/0.gif';
  var kb_width = 40;
  var kb_bheight = 80;
  var kb_wheight = 60;

  var el = {};

  var midi, m_in, m_ic, m_out, m_oc, d_in, d_out;
  var Pocket39;
  var p39;

  function Pocket39(){
    this.getEl([
      'm_in', 'm_ic', 'm_out', 'm_oc', 'status', 'notes', 'keyboard']);
    this.drawkeyboard();
    document.onmousemove = this.onmousemove;
    document.onkeydown = this.onkeydown;
    global.navigator.requestMIDIAccess({sysex: true}).then(
      this.acceptor, this.rejector);
  }

  Pocket39.prototype.acceptor = function(ma){
    var i;
    midi = ma;
    if(ma == null) alert('invalid MIDI Accessor');
    m_in = midi.inputs();
    m_ic = m_in ? m_in.length : 0;
    el['m_ic'].innerHTML += m_ic;
    for(i = 0; i < m_ic; ++i)
      el['m_in'].innerHTML += '' + i + ': [' + m_in[i].name + ']<br>';
    m_out = midi.outputs();
    m_oc = m_out ? m_out.length : 0;
    el['m_oc'].innerHTML += m_oc;
    for(i = 0; i < m_oc; ++i)
      el['m_out'].innerHTML += '' + i + ': [' + m_out[i].name + ']<br>';
    el['status'].innerHTML = 'OK';

    if(m_oc > 0){
      d_out = m_out[m_oc - 1];
      d_out.send([0xCF, 0x7B]); // ch15 bird
      d_out.send([0x9F, 0x4E, 0x64]); // ch15 4F# (0x4E) velocity 0x64
    }
    if(m_ic > 0){
      d_in = m_in[m_ic - 1];
      d_in.onmidimessage = this.onmidimessage;
    }
  }

  Pocket39.prototype.rejector = function(er){
    el['status'].innerHTML = '<b>MIDI access rejected:<br>'
      + 'code: ' + er.code + '<b>';
    el['status'].style.color = c39error;
  }

  Pocket39.prototype.onmidimessage = function(ev){
    var cmd = ev.data[0] & 0xF0;
    if(ev.data.length <= 0) return;
    if(cmd == 0x80){
      el['notes'].innerHTML += ev.data[1] + ' off' + ev.data[2];
    }else if(cmd == 0x90){
      el['notes'].innerHTML += ev.data[1] + ' on' + ev.data[2];
    }else if(cmd == 0xE0){
      el['notes'].innerHTML += ev.data[1] + ' bend ' + ev.data[2];
    }
  }

  Pocket39.prototype.onmousemove = function(ev){
    ev = ev || global.event;
    el['notes'].innerHTML = 'mouse: ' + ev.clientX + ', ' + ev.clientY;
    p39.flashkey(p39.getHoveringElement(ev.clientX, ev.clientY), c39hover, 50);
    return false;
  }

  Pocket39.prototype.getHoveringElement = function(x, y){
    var result = null;
    ['kb_', 'kw_'].forEach(function(kb){
      var i;
      for(i = 0; i < kb_notes_len + (kb[1] == 'b' ? 1 : 0); ++i){
        var k = document.getElementById(kb + i);
        var j = (i + kb_notes_first) % kb_oct;
        if(kb[1] == 'b' && (j == 0 || j == 3)) continue;
        if(k.ownerDocument.elementFromPoint(x, y) == k){
          result = k;
          return;
        }
      }
    });
    return result;
  }

  Pocket39.prototype.flashkey = function(k, c, t){
    if(k){
      k.style.background = c;
      setTimeout(function(){
        if(k) k.style.background = k.id[1] == 'b' ? c39gray : c39white;
      }, t);
    }
  }

  Pocket39.prototype.onkeydown = function(ev){
    ev = ev || global.event;
    el['notes'].innerHTML = 'key: ' + ev.keyCode;
    if(ev.keyCode == 116) return true;
    if(ev.keyCode == 32)
      p39.flashkey(document.getElementById('kb_8'), c39hilight, 100);
    return false;
  }

  Pocket39.prototype.drawkeyboard = function(){
    var i;
    var kb = '<table border="0" cellspacing="2" cellpadding="2"';
    kb += ' bgcolor="' + c39normal + '">';
    kb += '<tr><td><table border="0" cellspacing="2" cellpadding="2"';
    kb += ' bgcolor="' + c39normal + '">';
    kb += '<tr id="k_black">';
    for(i = 0; i < kb_notes_len + 1; ++i){
      var j = (i + kb_notes_first) % kb_oct;
      var col = (j == 0 || j == 3) ? c39normal : c39gray;
      var wid = (i == 0 || i == kb_notes_len) ? kb_width / 2 - 2 : kb_width;
      kb += '<td id="kb_' + i + '" bgcolor="' + col + '">';
      kb += '<img src="' + kb_gif + '" border="0"';
      kb += ' width="' + wid + '" height="' + kb_bheight + '"></td>';
    }
    kb += '</tr>';
    kb += '</table></td></tr>';
    kb += '<tr><td><table border="0" cellspacing="2" cellpadding="2"';
    kb += ' bgcolor="' + c39normal + '">';
    kb += '<tr id="k_white">';
    for(i = 0; i < kb_notes_len; ++i){
      kb += '<td id="kw_' + i + '" bgcolor="' + c39white + '">';
      kb += '<img src="' + kb_gif + '" border="0"';
      kb += ' width="' + kb_width + '" height="' + kb_wheight + '"></td>';
    }
    kb += '</tr>';
    kb += '</table></td></tr>';
    kb += '</table>';
    el['keyboard'].innerHTML = kb;
  }

  Pocket39.prototype.getEl = function(ka){
    var i;
    for(i = 0; i < ka.length; ++i) el[ka[i]] = document.getElementById(ka[i]);
  }

  global.p39_create = function(){
    return (p39 = new Pocket39);
  }

}(window));
