(function(global){

  var c39normal = '#0CA39A'; // '#66DDCC';
  var c39hilight = '#99FFEE';
  var c39hover = '#99DDCC'; // '#66CCDD';
  var c39gray = '#333333';
  var c39lgray = '#CCCCCC';
  var c39white = '#FFFFFF';
  var c39error = '#FF3333';
  var kb_oct = 7;
  var kb_notes_len = kb_oct * 2 + 2;
  var kb_notes_first = 3; // F
  var kb_gif = './img/0.gif';
  var kb_width = 40;
  var kb_bheight = 80;
  var kb_wheight = 60;
  var kb_rbwidth = 20;
  var kb_rwwidth = 30;
  var kb_rheight = 4;
  var kb_tonestr = [
    'C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];

  var slice = 10;
  var tick = Date.now();
  var range = 0;
  var flg = 0;
  var tone = 0x4E;
  var offset = 0;
  var pitch = 0;
  var oct = 99;
  var bend = 0;
  var vel = 0;
  var dat = 0;
  var cmd = 0;

  var tbl_out = null;
  var tbl_in = null;

  var el = {};

  var midi, m_in, m_ic, m_out, m_oc, d_in, d_out;
  var Pocket39;
  var p39;

  function toHex(b, w){
    var s = '0000000000000000' + b.toString(16);
    return s.substr(s.length - w, w);
  }

  function val2vlv(val){
    var i, vlv = 0;
    for(i = 3; i >= 0; --i){
      var v = (val >> (i * 7)) & 0x7F;
      vlv |= (v | (((v || vlv) && i) ? 0x080 : 0x00)) << (i * 8);
    }
    return vlv;
  }

  function Pocket39(){
    this.getEl([
      'm_in', 'm_ic', 'm_out', 'm_oc', 'status', 'info',
      'trans_opt', 'save_opt_0', 'save_opt_1', 'save_opt_2', 'save',
      'replay_opt_v', 'replay_opt_0', 'replay_opt_1', 'replay', 'testplay',
      'erase',
      'rollout_scr', 'rollout', 'keyboard', 'rollin_scr', 'rollin',
      'notes', 'notes_reverse', 'notes_play']);
    this.drawkeyboard();
    this.drawrolls();
    el['trans_opt'].onchange = this.ontransopt;
    el['replay_opt_0'].onclick = this.onreplayopt;
    el['replay_opt_1'].onclick = this.onreplayopt;
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
      d_in.onmidimessage = p39.onmidimessage;
    }
    el['save'].onclick = p39.onsave;
    el['replay'].onclick = p39.onreplay;
    el['testplay'].onclick = p39.ontestplay;
    el['erase'].onclick = p39.onerase;
  }

  Pocket39.prototype.rejector = function(er){
    el['status'].innerHTML = '<b>MIDI access rejected:<br>'
      + 'code: ' + er.code + '<b>';
    el['status'].style.color = c39error;
  }

  Pocket39.prototype.recorder = function(d, mode){ // mode 0: record, 1: replay
    var delta = 0;
    var tmp = '';
    tmp += toHex(d[0], 2) + toHex(d[1], 2) + toHex(d[2], 2);
    tmp += ' ' + toHex(Date.now(), 8);
    bend = d[2] * 256 + d[1];
    vel = d[2];
    dat = d[1];
    cmd = d[0] & 0xF0;
    if(cmd == 0xE0){
      tmp += ' bend';
      pitch = Math.floor((bend - 0x4000) / 0x0400);
    }else if(cmd == 0x80){
      tmp += ' off.';
      flg = 0;
      offset = 0;
    }else if(cmd == 0x90){
      tmp += ' on..';
      flg = 1;
      offset = 0x4000 + pitch * 0x0400;
      tone = dat + pitch;
      tick = Date.now();
    }
    delta = Date.now() - tick;
    tmp += ' ' + toHex(delta, 8);
    tmp += ' b' + toHex(cmd == 0xE0 ? (bend - offset) : 0, 4);
    tmp += ' p' + toHex(pitch, 4);
    tmp += ' o' + (oct = Math.floor(tone / 12) - 2);
    tmp += ' ' + (cmd == 0xE0 ? (
      '[] ' + (flg ? (bend >= 0x4000 ? '+' : '-') : '|')) : (
      toHex(tone, 2) + ' ' + kb_tonestr[tone % 12]));
    tmp += '\n';
    if(!mode){
      el['notes'].innerHTML = tmp + el['notes'].innerHTML;
      el['notes_reverse'].innerHTML += tmp;
    }
    if(cmd == 0x80) tick = Date.now();

    if(cmd == 0x80) p39.noteoff(oct, tone, delta);
    if(cmd == 0x90) p39.noteon(oct, tone, delta);
  }

  Pocket39.prototype.onmidimessage = function(ev){
    if(ev.data.length <= 0) return;
    p39.recorder(ev.data, 0);
  }

  Pocket39.prototype.noteoff = function(o, t, d){
    var k = p39.getNoteElement(o, t);
    if(k) k.style.background = k.id[1] == 'b' ? c39gray : c39white;
    {
      var n = p39.getNoteNum(o, t);
      tbl_out.rows[tbl_out.rows.length - 1].cells[n].style.height = d / 10;
    }
  }

  Pocket39.prototype.noteon = function(o, t, d){
    var k = p39.getNoteElement(o, t);
    if(k) p39.flashkey(k, c39hilight, 0);
    {
      var n = p39.getNoteNum(o, t);
      var l = tbl_out.rows.length;
      tbl_out.insertRow(l);
      tbl_out.rows[l].innerHTML = tbl_out.rows[0].innerHTML;
      tbl_out.rows[l].cells[n].style.background = c39hilight;
      el['rollout_scr'].scrollTop = el['rollout_scr'].scrollHeight;
    }
  }

  Pocket39.prototype.getNoteElement = function(o, t){
    var r = t % 12;
    var kb = 'kb_';
    if(r == 1 || r == 3)                 r = (r + 1) / 2;
    else if(r == 6 || r == 8 || r == 10) r = (r - 6) / 2 + 4;
    else{
      r = (r < 5) ? (r / 2) : ((r - 5) / 2 + 3);
      kb = 'kw_';
    }
    r += (o - 4) * kb_oct + kb_notes_first + 1; // why + 1 ?
    return document.getElementById(kb + r);
  }

  Pocket39.prototype.getNoteNum = function(o, t){
    // 7 = F start, + 1 means <td> spacer
    return (t % 12) + (o - 4) * 12 + 7 + 1;
  }

  Pocket39.prototype.ontransopt = function(ev){
    if(el['trans_opt'].selectedIndex != 1){
      alert('(補正)このバージョンでは「音名補正」のみ利用できます');
      el['trans_opt'].selectedIndex = 1;
    }
  }

  Pocket39.prototype.onsave = function(){
    var txt = el['notes_reverse'].innerHTML;
    if(txt == ''){
      alert('データがありません。先にスタイラスで演奏してください。');
    }else{
      var rsmf = 0;
      var smfv = 0;
      var dary = [
/*
        0x00, 0x90, 0x4E, 0x7F,
        0x78, 0x90, 0x52, 0x7F,
        0x78, 0x90, 0x55, 0x7F,
        0x81, 0x70, 0x80, 0x4E, 0x00,
        0x00, 0x80, 0x52, 0x00,
        0x00, 0x80, 0x55, 0x00
*/
      ];
      var o = document.getElementsByName('save_opt');
      var i, v = 0;
      for(i = 0; i < o.length; ++i) if(o[i].checked) v += parseInt(o[i].value);
      if(v){
        rsmf = 1;
        smfv = v - 1;
      }
{
// these codes are almost same as onreplay() and to marge them later
// but converted recursive call (setTimeout) to loop (must process indirectly)
      var lines = txt.split('\n');
      var line = 0;
      var delta = -1;
      var u = 0;
      while(true){
        var l = lines[line];
        var d0 = parseInt(l.substr(0, 2), 16);
        var d1 = parseInt(l.substr(2, 2), 16);
        var d2 = parseInt(l.substr(4, 2), 16);
        var t = parseInt(l.substr(7, 8), 16);
// el['notes'].innerHTML = '' + line + ',' + delta;
        ++line;
        if(!l.length) break;
        if(delta < 0) u = t;
        delta = t - u;
        u = t;
        if(rsmf){ // SMFdata
// regulate d1 (see Pocket39.prototype.recorder)
          var dd = parseInt(l.substr(45, 2), 16);
          var i, vlv = val2vlv(delta / 4);
          if(!vlv){
            dary.push(0);
          }else{
            for(i = 3; i >= 0; --i){
              var v = (vlv >> (i * 8)) & 0x0FF;
              if(v) dary.push(v);
            }
          }
          dary.push(d0);
          dary.push((d0 & 0xF0) == 0xE0 ? d1 : dd);
          dary.push(d2);
        }else{ // RAWdata
          if(true){ // binary
            dary.push(0); dary.push(d0); dary.push(d1); dary.push(d2);
            dary.push((t >> 24) & 0x0FF); dary.push((t >> 16) & 0x0FF);
              dary.push((t >> 8) & 0x0FF); dary.push(t & 0x0FF);
            dary.push((delta >> 24) & 0x0FF); dary.push((delta >> 16) & 0x0FF);
              dary.push((delta >> 8) & 0x0FF); dary.push(delta & 0x0FF);
            dary.push(0);  dary.push(0);  dary.push(0);  dary.push(0);
          }else{ // text
            var j;
            for(j = 0; j < l.length; ++j) dary.push(l.charCodeAt(j));
            dary.push(0x0A);
          }
        }
        if(line < lines.length) continue;
        break;
      }
}
      RAWSMFwriter(rsmf, smfv, new Uint8Array(dary));
    }
  }

  Pocket39.prototype.onreplayopt = function(ev){
    if(el['replay_opt_0'].checked) slice = 10;
    else if(el['replay_opt_1'].checked) slice = 1;
    el['replay_opt_v'].innerHTML = '' + slice;
  }

  Pocket39.prototype.onreplay = function(){
    var txt = el['notes_reverse'].innerHTML;
    var p = el['notes_play'];
    p.innerHTML = '';
    {
      tbl_in.innerHTML = tbl_out.innerHTML;
      el['rollin_scr'].scrollTop = 0;
      tbl_out.innerHTML = '';
      tbl_out.insertRow(-1);
      tbl_out.rows[0].innerHTML = tbl_in.rows[0].innerHTML;
      el['rollout_scr'].scrollTop = 0;
    }
    if(m_oc > 0 && txt != ''){
      var lines = txt.split('\n');
      var line = 0;
      var delta = 0;
      var clk = Date.now();
      var ply = function(){
        var msg = [];
        var l = lines[line];
        var d0 = parseInt(l.substr(0, 2), 16);
        var d1 = parseInt(l.substr(2, 2), 16);
        var d2 = parseInt(l.substr(4, 2), 16);
        var t = parseInt(l.substr(7, 8), 16);
        var n = Date.now();
        if(n - clk < delta){ setTimeout(ply, slice); return; }
// el['notes'].innerHTML = '' + line + ',' + n + ',' + clk + ',' + delta;
        clk = n;
        ++line;
        if(!l.length) return;
        if(line < lines.length){
          var m = lines[line];
          var u = parseInt(m.substr(7, 8), 16);
          delta = u - t;
        }
        p.innerHTML += toHex(d0, 2) + toHex(d1, 2) + toHex(d2, 2);
        p.innerHTML += ' ' + toHex(t, 8) + ' ' + delta + '\n';
// regulate d1 (see Pocket39.prototype.recorder)
//        d1 = (d0 & 0xF0) == 0xE0 ? d1 : parseInt(l.substr(45, 2), 16); // bug
        msg = [d0, d1, d2];
        if((d0 & 0xF0) == 0x90)
          if(tbl_in.rows.length > 1) tbl_in.deleteRow(1);
        p39.recorder(msg, 1);
        d_out.send(msg);
        if(line < lines.length) setTimeout(ply, slice);
      };
      ply();
    }
  }

  Pocket39.prototype.ontestplay = function(){
    el['notes'].innerHTML = '';
    el['notes_reverse'].innerHTML = global.sampleREC;
    p39.onreplay();
  }

  Pocket39.prototype.onerase = function(){
    el['notes'].innerHTML = '';
    el['notes_reverse'].innerHTML = '';

    tbl_out.innerHTML = '';
    tbl_out.insertRow(-1);
    tbl_out.rows[0].innerHTML = tbl_in.rows[0].innerHTML;
    el['rollout_scr'].scrollTop = 0;
    tbl_in.innerHTML = '';
    tbl_in.insertRow(-1);
    tbl_in.rows[0].innerHTML = tbl_out.rows[0].innerHTML;
    el['rollin_scr'].scrollTop = 0;
  }

  Pocket39.prototype.onmousemove = function(ev){
    ev = ev || global.event;
    el['info'].innerHTML = 'mouse: ' + ev.clientX + ', ' + ev.clientY;
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
      if(t) setTimeout(function(){
        if(k) k.style.background = k.id[1] == 'b' ? c39gray : c39white;
      }, t);
    }
  }

  Pocket39.prototype.onkeydown = function(ev){
    ev = ev || global.event;
    el['info'].innerHTML = 'key: ' + ev.keyCode;
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

  Pocket39.prototype.drawrolls = function(){
    ['out', 'in'].forEach(function(roll){
      var i;
      var rl = '<table border="0" cellspacing="1" cellpadding="0"';
      rl += ' bgcolor="' + c39normal + '" id="tbl_' + roll + '">';
      rl += '<tr>';
      for(i = 0; i < kb_notes_len + 1; ++i){
        var j = (i + kb_notes_first) % kb_oct;
        if(i == 0){
          rl += '<td bgcolor="' + c39normal + '">';
          rl += '<img src="' + kb_gif + '" border="0"';
          rl += ' width="' + 4 + '" height="' + kb_rheight + '">';
          rl += '</td>';
        }
        if(!(j == 0 || j == 3)){
          var wid = kb_rbwidth;
          if(i == kb_notes_len) wid -= 6;
          rl += '<td bgcolor="' + c39lgray + '">';
          rl += '<img src="' + kb_gif + '" border="0"';
          rl += ' width="' + wid + '" height="' + kb_rheight + '">';
          rl += '</td>';
        }
        if(i < kb_notes_len){
          rl += '<td bgcolor="' + c39white + '">';
          rl += '<img src="' + kb_gif + '" border="0"';
          rl += ' width="' + kb_rwwidth + '" height="' + kb_rheight + '">';
          rl += '</td>';
        }
      }
      rl += '</tr>';
      rl += '</table>';
      el['roll' + roll].innerHTML = rl;
    });
    tbl_out = document.getElementById('tbl_out');
    tbl_in = document.getElementById('tbl_in');
  }

  Pocket39.prototype.getEl = function(ka){
    ka.forEach(function(k){ el[k] = document.getElementById(k); });
  }

  global.p39_create = function(){
    return (p39 = new Pocket39);
  }

}(window));
