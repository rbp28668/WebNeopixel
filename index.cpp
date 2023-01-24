#include <string.h>
#include "index.hpp"

// Toolchain for generating this data from form.html
//https://www.toptal.com/developers/html-minifier
//https://tomeko.net/online_tools/cpp_text_escape.php?lang=en
static const char* webclient = 
"<html>\n"
"<head>\n"
"<title>PicoW NeoPixel</title>\n"
"<style>.grid{display:grid;column-gap:20px;row-gap:10px;grid-template-columns:100px auto auto}legend{font-style:italic;font-weight:700}fieldset{margin-bottom:7px}canvas{border:1px solid #000;height:20;width:256;grid-column:2}input[type=range]{width:256px;grid-column:2}label{grid-column:1}.c3{grid-column:3}.r1{grid-row:1}.r2{grid-row:2}.r3{grid-row:3}.r4{grid-row:4}</style>\n"
"<script>var hue=0,saturation=1,value=1,white=0,hue1=0,hue2=.5,rValue=1,inc=1,count=2,rWhite=0;function hsvToRgb(e,t,n){let o=g=b=0;e>1&&(e=1),t>1&&(t=1),n>1&&(n=1);const u=Math.floor(6*e),c=6*e-u,a=n*(1-t),h=n*(1-c*t),i=n*(1-(1-c)*t);function l(e){let t=Math.floor(e).toString(16);return 1==t.length?\"0\"+t:t}0==u?(o=n,g=i,b=a):1==u?(o=h,g=n,b=a):2==u?(o=a,g=n,b=i):3==u?(o=a,g=h,b=n):4==u?(o=i,g=a,b=n):u<=6&&(o=n,g=a,b=h);return\"#\"+l(255*o)+l(255*g)+l(255*b)}function h2RGB(e,t,n){let o=g=b=0;e>1&&(e=1);let u=Math.floor(6*e),c=6*e-u,a=1-c;0==u?(o=1,g=c,b=0):1==u?(o=a,g=1,b=0):2==u?(o=0,g=1,b=c):3==u?(o=0,g=a,b=1):4==u?(o=c,g=0,b=1):u<=6&&(o=1,g=0,b=a),t[n+0]=Math.floor(255*o),t[n+1]=Math.floor(255*g),t[n+2]=Math.floor(255*b),t[n+3]=255}function drawRGB(e,t){var n=document.getElementById(e),o=n.getContext(\"2d\");const u=o.getImageData(0,0,n.width,n.height),c=u.data;let a=0;for(let e=0;e<u.height;e++)for(let e=0;e<u.width;e++)h2RGB(e/u.width,c,a),a+=4;o.putImageData(u,0,0),n.addEventListener(\"click\",(e=>function(e){const o=n.getBoundingClientRect(),u=e.clientX-o.left,c=(e.clientY,o.top,u/o.width);return t&&t(c),c}(e)))}function sendCB(){const e=new URL(\"/set\",document.baseURI),t=hsvToRgb(hue,saturation,value);e.search=`?rgb=${t}&white=${Math.floor(255*white)}`;fetch(e,{method:\"GET\"})}function setBright(e){value=e,sendCB()}function setSat(e){saturation=e,sendCB()}function setWhite(e){white=e,sendCB()}function sendRipple(e){let t=document.getElementById(\"value\");const n=t.value;t=document.getElementById(\"inc\");const o=t.value;t=document.getElementById(\"count\");const u=t.value;t=document.getElementById(\"w2\");const c=t.value,a=new URL(e,document.baseURI);a.search=`?hue=${hue1}&hue2=${hue2}&value=${n}&white=${Math.floor(255*c)}&count=${u}&inc=${o}`;fetch(a,{method:\"GET\"})}function sendRate(){const e=document.getElementById(\"rate\").value,t=new URL(\"/cycle\",document.baseURI);t.search=`?rate=${e}`;fetch(t,{method:\"GET\"})}function sendSparkle(){const e=new URL(\"/sparkle\",document.baseURI);fetch(e,{method:\"GET\"})}function configure(){drawRGB(\"rgbPicker\",(function(e){hue=e,sendCB()})),drawRGB(\"hue1\",(function(e){hue1=e})),drawRGB(\"hue2\",(function(e){hue2=e}))}</script>\n"
"</head>\n"
"<body onload=configure()>\n"
"<fieldset>\n"
"<legend>Set colour and brightness</legend>\n"
"<div class=grid>\n"
"<label for=rgbPicker>Colour</label>\n"
"<canvas id=rgbPicker> </canvas>\n"
"<label for=brightness>Brightness</label>\n"
"<input id=brightness type=range min=0 max=1.0 step=0.001 value=0.5 onchange=setBright(this.value)>\n"
"<label for=sat>Saturation</label>\n"
"<input id=sat type=range min=0 max=1.0 step=0.001 value=0.5 onchange=setSat(this.value)>\n"
"<label for=white>White</label>\n"
"<input id=white type=range min=0 max=1.0 step=0.001 value=0.5 onchange=setWhite(this.value)>\n"
"</div>\n"
"</fieldset>\n"
"<fieldset>\n"
"<legend>Animated Waves</legend>\n"
"<div class=grid>\n"
"<label for=hue1>Colour</label>\n"
"<canvas id=hue1> </canvas>\n"
"<label for=hue2>Back Colour</label>\n"
"<canvas id=hue2> </canvas>\n"
"<label for=value>Brightness</label>\n"
"<input id=value type=range min=0 max=1.0 step=0.001 value=0.5>\n"
"<label for=inc>Increment</label>\n"
"<input type=number id=inc value=1 min=0 max=255 name=brt><br>\n"
"<label for=count>count</label>\n"
"<input type=number id=count value=2 min=0 max=255 name=brt><br>\n"
"<label for=w2>White</label>\n"
"<input id=w2 type=range min=0 max=1.0 step=0.001 value=0>\n"
"<button class=\"r1 c3\" onclick='sendRipple(\"/ripples\")'>Ripples</button>\n"
"<button class=\"r2 c3\" onclick='sendRipple(\"/spokes\")'>Spokes</button>\n"
"<button class=\"r3 c3\" onclick='sendRipple(\"/horizontal\")'>Horizontal</button>\n"
"<button class=\"r4 c3\" onclick='sendRipple(\"/vertical\")'>Vertical</button>\n"
"</div>\n"
"</fieldset>\n"
"<fieldset>\n"
"<legend>Other</legend>\n"
"<div class=grid>\n"
"<button class=r1 onclick=sendSparkle()>Sparkle</button>\n"
"</div>\n"
"</fieldset>\n"
"<fieldset>\n"
"<legend>Update rate</legend>\n"
"<div class=grid>\n"
"<label for=rate>Increment</label>\n"
"<input type=number id=rate value=1 min=0 max=255 name=brt><br>\n"
"<button class=\"r1 c3\" onclick=sendRate()>Set Rate</button>\n"
"</div>\n"
"</fieldset>\n"
"</body>\n"
"</html>"
;



bool IndexPage::matches(const char* verb, const char* path){
    bool accept = (strcmp(verb,"GET") == 0) && (
        strncmp(path, "/",1) == 0 ||
        strncmp(path, "/index",6) == 0
        );
     return accept;
}

void IndexPage::process( HttpRequest& request, HttpResponse& response){
    response.setStatus(200,"OK");
    response.addHeader("Server", "PicoW");
    response.addHeader("Content-Type", "text/html");
    response.addHeader("Access-Control-Allow-Origin","*");
    response.setBody(webclient);
}
