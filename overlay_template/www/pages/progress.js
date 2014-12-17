/*
 * Copyright (c) 2013-2014, Altera Corporation <www.altera.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Altera Corporation nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

interval_msec = 2000;
debug = false;

function update() {
  // Handle the various browsers... Pre-IE7 doesn't support XMLHttpRequest.
  if (window.XMLHttpRequest && !(window.ActiveXObject)) {
    try {
      req = new XMLHttpRequest();
    } catch(e) {
      req = false;
    }
  // IE/Windows ActiveX
    } else if (window.ActiveXObject) {
      try {
        req = new ActiveXObject("Msxml2.XMLHTTP");
      } catch(e) {
        try {
          req = new ActiveXObject("Microsoft.XMLHTTP");
      } catch(e) {
        req = false;
      }
    }
  }

  // If request isn't possible, just return right away.
  if (!req) return;

  req.open("GET","/PROGRESS", false);
  req.send(null);
  if(req.status == 200) {
    // The server sent a response.
    var progress = eval(req.responseText);
    if(debug) {
      document.getElementById('debug_panel').innerHTML += "speed = " + progress.speed + " " + progress.percent + "%" + '<br />';
    }
    bar = document.getElementById('progressbar');
    if(progress.state == 'done'){
      w = 540;
      bar.style.width = w + 'px';
      window.clearInterval(interval);
      window.clearTimeout(interval);
      return;
    }
    w = (progress.percent/100) * 520;
    bar.style.width = w + 'px';
    document.getElementById('tp').innerHTML = progress.percent + "%";
  }
  else {
    // Something didn't work....at least set an alert with useful information!
    alert("Error " + request.status + ":  " + request.statusText );
  }
}

function startProgress() {
  // Code needs to go here to generate the progress bar.
  interval = window.setInterval(
    function () {
      update();
    },
    interval_msec
  );
};

