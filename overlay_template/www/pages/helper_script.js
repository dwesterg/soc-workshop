// First Time Visit Processing
// copyright 10th January 2006, Stephen Chapman
// permission to use this Javascript on your web page is granted
// provided that all of the below code in this script (including this
// comment) is used without any alteration
// http://javascript.about.com/library/blfirst1.htm
function rC(nam) {var tC = document.cookie.split('; '); for (var i = tC.length - 1; i >= 0; i--) {var x = tC[i].split('='); if (nam == x[0]) return unescape(x[1]);} return '~';} function wC(nam,val) {document.cookie = nam + '=' + escape(val);} function lC(nam,pg) {var val = rC(nam); if (val.indexOf('~'+pg+'~') != -1) return false; val += pg + '~'; wC(nam,val); return true;} function firstTime(cN) {return lC('pWrD4jBo',cN);} function thisPage() {var page = location.href.substring(location.href.lastIndexOf('\/')+1); pos = page.indexOf('.');if (pos > -1) {page = page.substr(0,pos);} return page;}

// example code to call it - you may modify this as required
function start() {
   if (firstTime(thisPage())) {
      // this code only runs for first visit
	  //check if local storage is supported by the browser
	  if(typeof(Storage)!=="undefined"){}
	  else{ alert('Please use browser that support HTML5 to view this page.');}
   }
   // other code to run every time once page is loaded goes here
   document.getElementById("lightshow").value = sessionStorage.getItem("lightshow_text");
   document.getElementById("led0_id").value = sessionStorage.getItem("led0_text");
   document.getElementById("led1_id").value = sessionStorage.getItem("led1_text");
   document.getElementById("led2_id").value = sessionStorage.getItem("led2_text");
   document.getElementById("led3_id").value = sessionStorage.getItem("led3_text");
}
onload = start;


function valuevalidation(value, index)
{
	if(index != 0) {
	if( isNaN(value) )
        alert('Please enter integer number which is greater than zero.');
    else if (value < 1)
		alert('Please enter integer number which is greater than zero.');
	else if (value.search(/\./)!=-1)
		alert('Please enter integer number which is greater than zero.');
	}

	if(index == 0)
		sessionStorage.setItem("lightshow_text", value);
	else if(index == 1)
		sessionStorage.setItem("led0_text", value);
	else if(index == 2)
		sessionStorage.setItem("led1_text", value);
	else if(index == 3)
		sessionStorage.setItem("led2_text", value);
	else if(index == 4)
		sessionStorage.setItem("led3_text", value);
}
