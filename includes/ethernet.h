// PixelNutApp Ethernet Communications
//
// Uses global variables: 'pixelNutSupport', 'pAppCmd'.
// Calls global routines: 'CheckExecCmd', 'ErrorHandler', 'BlinkStatusLED'.
//========================================================================================
/*
Copyright (c) 2015-2018, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if ETHERNET_COMM
#if defined(SPARK)

extern void CheckExecCmd(char *instr); // defined in main.h
extern AppCommands *pAppCmd;           // pointer to current instance

#include "softap_http.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

// Blink patterns (long, short):
// 0,1  Fast repeats: waiting for debugger
// 0,2  Just once: startup was successful
// 0,2  Waiting for network configuration
// 0,3  EEPROM format finished
// 1,0  Connecting to cloud
// 1,0  Sending Beacon message
// else repeating error message

#define FLASHOFF_PARTICLE_MODE    FLASHOFF_PATTERN_END
#define PARTICLE_MODE_WIFI        0   // value in flash to start in our own SoftAP mode, for use with the PixelNutCtrl application
#define PARTICLE_MODE_CLOUD       1   // value in flash to connect to Particle Cloud if have credentials, use in SoftAP mode but serve webpage below to be used by Particle application
#define PARTICLE_MODE_BEACON      2   // connect to Particle Cloud and broadcast beacon, cleared after customer claims ownership of device through PixelNut website (www.pixelnut.io)

typedef int  (*FunHandler)(String arg);                         // handler for subscribe event
typedef void (*PubHandler)(const char *name, const char *data); // handler for function calls

#define PARTICLE_SUBNAME_DEVICE   "particle/device/name"
#define PARTICLE_SUBNAME_IPADDR   "particle/device/ip"

#define PNUT_SUBNAME_DEVCMD_PFX   "PnutCmd"               // prefix for events targeting specific device
#define PNUT_SUBNAME_PRODCMD      "PixelNutCommand"       // name of event that reaches every PixelNut device (only in debug)
#define PNUT_PUBNAME_BEACON       "PixelNutBeacon"        // name of event broadcast from devices when unattached
#define PNUT_FUNNAME_BEACON       "Beacon"                // name of function to enable/disable the beacon
#define PNUT_FUNNAME_SETSSID      "SetSSID"               // name of function to set/clear WiFi SSID/PWD
#define PNUT_FUNNAME_RESTART      "Restart"               // name of function to cause device reboot

#define PNUT_VARNAME_CONFIGINFO   "ConfigInfo"            // name of variable with configuration info
#define PNUT_VARNAME_SEGINFO      "SegInfo"               // name of variable with segment offsets/lengths
#define PNUT_VARNAME_SEGVALS_PFX  "SegVals"               // prefix for variables with segment property values

static const char index_html[] = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Setup your device</title><link rel='stylesheet' type='text/css' href='style.css'></head><body><h1>Connect me to your WiFi!</h1><h3>My device ID:</h3><input type=text id='device-id' size='25' value='' disabled/><button type='button' class='input-helper' id='copy-button'>Copy</button><div id='scan-div'><h3>Scan for visible WiFi networks</h3><button id='scan-button' type='button'>Scan</button></div><div id='networks-div'></div><div id='connect-div' style='display: none'><p>Don't see your network? Move me closer to your router, then re-scan.</p><form id='connect-form'><input type='password' id='password' size='25' placeholder='password'/><button type='button' class='input-helper' id='show-button'>Show</button><button type='submit' id='connect-button'>Connect</button></form></div><script src='rsa-utils/jsbn_1.js'></script><script src='rsa-utils/jsbn_2.js'></script><script src='rsa-utils/prng4.js'></script><script src='rsa-utils/rng.js'></script><script src='rsa-utils/rsa.js'></script><script src='script.js'></script></body></html>";
static const char style_css[]  = "html{height:100%;margin:auto;background-color:white}body{box-sizing:border-box;min-height:100%;padding:20px;background-color:#1aabe0;font-family:'Lucida Sans Unicode','Lucida Grande',sans-serif;font-weight:normal;color:white;margin-top:0;margin-left:auto;margin-right:auto;margin-bottom:0;max-width:400px;text-align:center;border:1px solid #6e6e70;border-radius:4px}div{margin-top:25px;margin-bottom:25px}h1{margin-top:25px;margin-bottom:25px}button{border-color:#1c75be;background-color:#1c75be;color:white;border-radius:5px;height:30px;font-size:15px;font-weight:bold}button.input-helper{background-color:#bebebe;border-color:#bebebe;color:#6e6e70;margin-left:3px}button:disabled{background-color:#bebebe;border-color:#bebebe;color:white}input[type='text'],input[type='password']{background-color:white;color:#6e6e70;border-color:white;border-radius:5px;height:25px;text-align:center;font-size:15px}input:disabled{background-color:#bebebe;border-color:#bebebe}input[type='radio']{position:relative;bottom:-0.33em;margin:0;border:0;height:1.5em;width:15%}label{padding-top:7px;padding-bottom:7px;padding-left:5%;display:inline-block;width:80%;text-align:left}input[type='radio']:checked+label{font-weight:bold;color:#1c75be}.scanning-error{font-weight:bold;text-align:center}.radio-div{box-sizing:border-box;margin:2px;margin-left:auto;margin-right:auto;background-color:white;color:#6e6e70;border:1px solid #6e6e70;border-radius:3px;width:100%;padding:5px}#networks-div{margin-left:auto;margin-right:auto;text-align:left}#device-id{text-align:center}#scan-button{min-width:100px}#connect-button{display:block;min-width:100px;margin-top:10px;margin-left:auto;margin-right:auto;margin-bottom:20px}#password{margin-top:20px;margin-bottom:10px}";
static const char rsa_js[]     = "function parseBigInt(a,b){return new BigInteger(a,b);}function linebrk(a,b){var c='';var d=0;while(d+b<a.length){c+=a.substring(d,d+b)+'\\n';d+=b;}return c+a.substring(d,a.length);}function byte2Hex(a){if(a<0x10)return '0'+a.toString(16);else return a.toString(16);}function pkcs1pad2(a,b){if(b<a.length+11){alert('Message too long for RSA');return null;}var c=new Array();var d=a.length-1;while(d>=0&&b>0){var e=a.charCodeAt(d--);if(e<128)c[--b]=e;else if((e>127)&&(e<2048)){c[--b]=(e&63)|128;c[--b]=(e>>6)|192;}else{c[--b]=(e&63)|128;c[--b]=((e>>6)&63)|128;c[--b]=(e>>12)|224;}}c[--b]=0;var f=new SecureRandom();var g=new Array();while(b>2){g[0]=0;while(g[0]==0)f.nextBytes(g);c[--b]=g[0];}c[--b]=2;c[--b]=0;return new BigInteger(c);}function RSAKey(){this.n=null;this.e=0;this.d=null;this.p=null;this.q=null;this.dmp1=null;this.dmq1=null;this.coeff=null;}function RSASetPublic(a,b){if(a!=null&&b!=null&&a.length>0&&b.length>0){this.n=parseBigInt(a,16);this.e=parseInt(b,16);}else alert('Invalid RSA public key');}function RSADoPublic(a){return a.modPowInt(this.e,this.n);}function RSAEncrypt(a){var b=pkcs1pad2(a,(this.n.bitLength()+7)>>3);if(b==null)return null;var c=this.doPublic(b);if(c==null)return null;var d=c.toString(16);if((d.length&1)==0)return d;else return '0'+d;}RSAKey.prototype.doPublic=RSADoPublic;RSAKey.prototype.setPublic=RSASetPublic;RSAKey.prototype.encrypt=RSAEncrypt;";
static const char rng_js[]     = "var rng_state;var rng_pool;var rng_pptr;function rng_seed_int(a){rng_pool[rng_pptr++]^=a&255;rng_pool[rng_pptr++]^=(a>>8)&255;rng_pool[rng_pptr++]^=(a>>16)&255;rng_pool[rng_pptr++]^=(a>>24)&255;if(rng_pptr>=rng_psize)rng_pptr-=rng_psize;}function rng_seed_time(){rng_seed_int(new Date().getTime());}if(rng_pool==null){rng_pool=new Array();rng_pptr=0;var t;if(window.crypto&&window.crypto.getRandomValues){var ua=new Uint8Array(32);window.crypto.getRandomValues(ua);for(t=0;t<32;++t)rng_pool[rng_pptr++]=ua[t];}if(navigator.appName=='Netscape'&&navigator.appVersion<'5'&&window.crypto){var z=window.crypto.random(32);for(t=0;t<z.length;++t)rng_pool[rng_pptr++]=z.charCodeAt(t)&255;}while(rng_pptr<rng_psize){t=Math.floor(65536*Math.random());rng_pool[rng_pptr++]=t>>>8;rng_pool[rng_pptr++]=t&255;}rng_pptr=0;rng_seed_time();}function rng_get_byte(){if(rng_state==null){rng_seed_time();rng_state=prng_newstate();rng_state.init(rng_pool);for(rng_pptr=0;rng_pptr<rng_pool.length;++rng_pptr)rng_pool[rng_pptr]=0;rng_pptr=0;}return rng_state.next();}function rng_get_bytes(a){var b;for(b=0;b<a.length;++b)a[b]=rng_get_byte();}function SecureRandom(){}SecureRandom.prototype.nextBytes=rng_get_bytes;";
static const char jsbn_1_js[]  = "var dbits;var canary=0xdeadbeefcafe;var j_lm=((canary&0xffffff)==0xefcafe);function BigInteger(a,b,c){if(a!=null)if('number'==typeof a)this.fromNumber(a,b,c);else if(b==null&&'string'!=typeof a)this.fromString(a,256);else this.fromString(a,b);}function nbi(){return new BigInteger(null);}function am1(a,b,c,d,e,f){while(--f>=0){var g=b*this[a++]+c[d]+e;e=Math.floor(g/0x4000000);c[d++]=g&0x3ffffff;}return e;}function am2(a,b,c,d,e,f){var g=b&0x7fff,h=b>>15;while(--f>=0){var i=this[a]&0x7fff;var j=this[a++]>>15;var k=h*i+j*g;i=g*i+((k&0x7fff)<<15)+c[d]+(e&0x3fffffff);e=(i>>>30)+(k>>>15)+h*j+(e>>>30);c[d++]=i&0x3fffffff;}return e;}function am3(a,b,c,d,e,f){var g=b&0x3fff,h=b>>14;while(--f>=0){var i=this[a]&0x3fff;var j=this[a++]>>14;var k=h*i+j*g;i=g*i+((k&0x3fff)<<14)+c[d]+e;e=(i>>28)+(k>>14)+h*j;c[d++]=i&0xfffffff;}return e;}if(j_lm&&(navigator.appName=='Microsoft Internet Explorer')){BigInteger.prototype.am=am2;dbits=30;}else if(j_lm&&(navigator.appName!='Netscape')){BigInteger.prototype.am=am1;dbits=26;}else{BigInteger.prototype.am=am3;dbits=28;}BigInteger.prototype.DB=dbits;BigInteger.prototype.DM=((1<<dbits)-1);BigInteger.prototype.DV=(1<<dbits);var BI_FP=52;BigInteger.prototype.FV=Math.pow(2,BI_FP);BigInteger.prototype.F1=BI_FP-dbits;BigInteger.prototype.F2=2*dbits-BI_FP;var BI_RM='0123456789abcdefghijklmnopqrstuvwxyz';var BI_RC=new Array();var rr,vv;rr='0'.charCodeAt(0);for(vv=0;vv<=9;++vv)BI_RC[rr++]=vv;rr='a'.charCodeAt(0);for(vv=10;vv<36;++vv)BI_RC[rr++]=vv;rr='A'.charCodeAt(0);for(vv=10;vv<36;++vv)BI_RC[rr++]=vv;function int2char(a){return BI_RM.charAt(a);}function intAt(a,b){var c=BI_RC[a.charCodeAt(b)];return(c==null)?-1:c;}function bnpCopyTo(a){for(var b=this.t-1;b>=0;--b)a[b]=this[b];a.t=this.t;a.s=this.s;}function bnpFromInt(a){this.t=1;this.s=(a<0)?-1:0;if(a>0)this[0]=a;else if(a<-1)this[0]=a+this.DV;else this.t=0;}function nbv(a){var b=nbi();b.fromInt(a);return b;}function bnpFromString(a,b){var c;if(b==16)c=4;else if(b==8)c=3;else if(b==256)c=8;else if(b==2)c=1;else if(b==32)c=5;else if(b==4)c=2;else{this.fromRadix(a,b);return;}this.t=0;this.s=0;var d=a.length,e=false,f=0;while(--d>=0){var g=(c==8)?a[d]&0xff:intAt(a,d);if(g<0){if(a.charAt(d)=='-')e=true;continue;}e=false;if(f==0)this[this.t++]=g;else if(f+c>this.DB){this[this.t-1]|=(g&((1<<(this.DB-f))-1))<<f;this[this.t++]=(g>>(this.DB-f));}else this[this.t-1]|=g<<f;f+=c;if(f>=this.DB)f-=this.DB;}if(c==8&&(a[0]&0x80)!=0){this.s=-1;if(f>0)this[this.t-1]|=((1<<(this.DB-f))-1)<<f;}this.clamp();if(e)BigInteger.ZERO.subTo(this,this);}function bnpClamp(){var a=this.s&this.DM;while(this.t>0&&this[this.t-1]==a)--this.t;}function bnToString(a){if(this.s<0)return '-'+this.negate().toString(a);var b;if(a==16)b=4;else if(a==8)b=3;else if(a==2)b=1;else if(a==32)b=5;else if(a==4)b=2;else return this.toRadix(a);var c=(1<<b)-1,d,e=false,f='',g=this.t;var h=this.DB-(g*this.DB)%b;if(g-->0){if(h<this.DB&&(d=this[g]>>h)>0){e=true;f=int2char(d);}while(g>=0){if(h<b){d=(this[g]&((1<<h)-1))<<(b-h);d|=this[--g]>>(h+=this.DB-b);}else{d=(this[g]>>(h-=b))&c;if(h<=0){h+=this.DB;--g;}}if(d>0)e=true;if(e)f+=int2char(d);}}return e?f:'0';}function bnNegate(){var a=nbi();BigInteger.ZERO.subTo(this,a);return a;}function bnAbs(){return(this.s<0)?this.negate():this;}function bnCompareTo(a){var b=this.s-a.s;if(b!=0)return b;var c=this.t;b=c-a.t;if(b!=0)return(this.s<0)?-b:b;while(--c>=0)if((b=this[c]-a[c])!=0)return b;return 0;}function nbits(a){var b=1,c;if((c=a>>>16)!=0){a=c;b+=16;}if((c=a>>8)!=0){a=c;b+=8;}if((c=a>>4)!=0){a=c;b+=4;}if((c=a>>2)!=0){a=c;b+=2;}if((c=a>>1)!=0){a=c;b+=1;}return b;}function bnBitLength(){if(this.t<=0)return 0;return this.DB*(this.t-1)+nbits(this[this.t-1]^(this.s&this.DM));}function bnpDLShiftTo(a,b){var c;for(c=this.t-1;c>=0;--c)b[c+a]=this[c];for(c=a-1;c>=0;--c)b[c]=0;b.t=this.t+a;b.s=this.s;}function bnpDRShiftTo(a,b){for(var c=a;c<this.t;++c)b[c-a]=this[c];b.t=Math.max(this.t-a,0);b.s=this.s;}function bnpLShiftTo(a,b){var c=a%this.DB;var d=this.DB-c;var e=(1<<d)-1;var f=Math.floor(a/this.DB),g=(this.s<<c)&this.DM,h;for(h=this.t-1;h>=0;--h){b[h+f+1]=(this[h]>>d)|g;g=(this[h]&e)<<c;}for(h=f-1;h>=0;--h)b[h]=0;b[f]=g;b.t=this.t+f+1;b.s=this.s;b.clamp();}";
static const char jsbn_2_js[]  = "function bnpRShiftTo(a,b){b.s=this.s;var c=Math.floor(a/this.DB);if(c>=this.t){b.t=0;return;}var d=a%this.DB;var e=this.DB-d;var f=(1<<d)-1;b[0]=this[c]>>d;for(var g=c+1;g<this.t;++g){b[g-c-1]|=(this[g]&f)<<e;b[g-c]=this[g]>>d;}if(d>0)b[this.t-c-1]|=(this.s&f)<<e;b.t=this.t-c;b.clamp();}function bnpSubTo(a,b){var c=0,d=0,e=Math.min(a.t,this.t);while(c<e){d+=this[c]-a[c];b[c++]=d&this.DM;d>>=this.DB;}if(a.t<this.t){d-=a.s;while(c<this.t){d+=this[c];b[c++]=d&this.DM;d>>=this.DB;}d+=this.s;}else{d+=this.s;while(c<a.t){d-=a[c];b[c++]=d&this.DM;d>>=this.DB;}d-=a.s;}b.s=(d<0)?-1:0;if(d<-1)b[c++]=this.DV+d;else if(d>0)b[c++]=d;b.t=c;b.clamp();}function bnpMultiplyTo(a,b){var c=this.abs(),d=a.abs();var e=c.t;b.t=e+d.t;while(--e>=0)b[e]=0;for(e=0;e<d.t;++e)b[e+c.t]=c.am(0,d[e],b,e,0,c.t);b.s=0;b.clamp();if(this.s!=a.s)BigInteger.ZERO.subTo(b,b);}function bnpSquareTo(a){var b=this.abs();var c=a.t=2*b.t;while(--c>=0)a[c]=0;for(c=0;c<b.t-1;++c){var d=b.am(c,b[c],a,2*c,0,1);if((a[c+b.t]+=b.am(c+1,2*b[c],a,2*c+1,d,b.t-c-1))>=b.DV){a[c+b.t]-=b.DV;a[c+b.t+1]=1;}}if(a.t>0)a[a.t-1]+=b.am(c,b[c],a,2*c,0,1);a.s=0;a.clamp();}function bnpDivRemTo(a,b,c){var d=a.abs();if(d.t<=0)return;var e=this.abs();if(e.t<d.t){if(b!=null)b.fromInt(0);if(c!=null)this.copyTo(c);return;}if(c==null)c=nbi();var f=nbi(),g=this.s,h=a.s;var i=this.DB-nbits(d[d.t-1]);if(i>0){d.lShiftTo(i,f);e.lShiftTo(i,c);}else{d.copyTo(f);e.copyTo(c);}var j=f.t;var k=f[j-1];if(k==0)return;var l=k*(1<<this.F1)+((j>1)?f[j-2]>>this.F2:0);var m=this.FV/l,n=(1<<this.F1)/l,o=1<<this.F2;var p=c.t,q=p-j,r=(b==null)?nbi():b;f.dlShiftTo(q,r);if(c.compareTo(r)>=0){c[c.t++]=1;c.subTo(r,c);}BigInteger.ONE.dlShiftTo(j,r);r.subTo(f,f);while(f.t<j)f[f.t++]=0;while(--q>=0){var s=(c[--p]==k)?this.DM:Math.floor(c[p]*m+(c[p-1]+o)*n);if((c[p]+=f.am(0,s,c,q,0,j))<s){f.dlShiftTo(q,r);c.subTo(r,c);while(c[p]<--s)c.subTo(r,c);}}if(b!=null){c.drShiftTo(j,b);if(g!=h)BigInteger.ZERO.subTo(b,b);}c.t=j;c.clamp();if(i>0)c.rShiftTo(i,c);if(g<0)BigInteger.ZERO.subTo(c,c);}function bnMod(a){var b=nbi();this.abs().divRemTo(a,null,b);if(this.s<0&&b.compareTo(BigInteger.ZERO)>0)a.subTo(b,b);return b;}function Classic(a){this.m=a;}function cConvert(a){if(a.s<0||a.compareTo(this.m)>=0)return a.mod(this.m);else return a;}function cRevert(a){return a;}function cReduce(a){a.divRemTo(this.m,null,a);}function cMulTo(a,b,c){a.multiplyTo(b,c);this.reduce(c);}function cSqrTo(a,b){a.squareTo(b);this.reduce(b);}Classic.prototype.convert=cConvert;Classic.prototype.revert=cRevert;Classic.prototype.reduce=cReduce;Classic.prototype.mulTo=cMulTo;Classic.prototype.sqrTo=cSqrTo;function bnpInvDigit(){if(this.t<1)return 0;var a=this[0];if((a&1)==0)return 0;var b=a&3;b=(b*(2-(a&0xf)*b))&0xf;b=(b*(2-(a&0xff)*b))&0xff;b=(b*(2-(((a&0xffff)*b)&0xffff)))&0xffff;b=(b*(2-a*b%this.DV))%this.DV;return(b>0)?this.DV-b:-b;}function Montgomery(a){this.m=a;this.mp=a.invDigit();this.mpl=this.mp&0x7fff;this.mph=this.mp>>15;this.um=(1<<(a.DB-15))-1;this.mt2=2*a.t;}function montConvert(a){var b=nbi();a.abs().dlShiftTo(this.m.t,b);b.divRemTo(this.m,null,b);if(a.s<0&&b.compareTo(BigInteger.ZERO)>0)this.m.subTo(b,b);return b;}function montRevert(a){var b=nbi();a.copyTo(b);this.reduce(b);return b;}function montReduce(a){while(a.t<=this.mt2)a[a.t++]=0;for(var b=0;b<this.m.t;++b){var c=a[b]&0x7fff;var d=(c*this.mpl+(((c*this.mph+(a[b]>>15)*this.mpl)&this.um)<<15))&a.DM;c=b+this.m.t;a[c]+=this.m.am(0,d,a,b,0,this.m.t);while(a[c]>=a.DV){a[c]-=a.DV;a[++c]++;}}a.clamp();a.drShiftTo(this.m.t,a);if(a.compareTo(this.m)>=0)a.subTo(this.m,a);}function montSqrTo(a,b){a.squareTo(b);this.reduce(b);}function montMulTo(a,b,c){a.multiplyTo(b,c);this.reduce(c);}Montgomery.prototype.convert=montConvert;Montgomery.prototype.revert=montRevert;Montgomery.prototype.reduce=montReduce;Montgomery.prototype.mulTo=montMulTo;Montgomery.prototype.sqrTo=montSqrTo;function bnpIsEven(){return((this.t>0)?(this[0]&1):this.s)==0;}function bnpExp(a,b){if(a>0xffffffff||a<1)return BigInteger.ONE;var c=nbi(),d=nbi(),e=b.convert(this),f=nbits(a)-1;e.copyTo(c);while(--f>=0){b.sqrTo(c,d);if((a&(1<<f))>0)b.mulTo(d,e,c);else{var g=c;c=d;d=g;}}return b.revert(c);}function bnModPowInt(a,b){var c;if(a<256||b.isEven())c=new Classic(b);else c=new Montgomery(b);return this.exp(a,c);}BigInteger.prototype.copyTo=bnpCopyTo;BigInteger.prototype.fromInt=bnpFromInt;BigInteger.prototype.fromString=bnpFromString;BigInteger.prototype.clamp=bnpClamp;BigInteger.prototype.dlShiftTo=bnpDLShiftTo;BigInteger.prototype.drShiftTo=bnpDRShiftTo;BigInteger.prototype.lShiftTo=bnpLShiftTo;BigInteger.prototype.rShiftTo=bnpRShiftTo;BigInteger.prototype.subTo=bnpSubTo;BigInteger.prototype.multiplyTo=bnpMultiplyTo;BigInteger.prototype.squareTo=bnpSquareTo;BigInteger.prototype.divRemTo=bnpDivRemTo;BigInteger.prototype.invDigit=bnpInvDigit;BigInteger.prototype.isEven=bnpIsEven;BigInteger.prototype.exp=bnpExp;BigInteger.prototype.toString=bnToString;BigInteger.prototype.negate=bnNegate;BigInteger.prototype.abs=bnAbs;BigInteger.prototype.compareTo=bnCompareTo;BigInteger.prototype.bitLength=bnBitLength;BigInteger.prototype.mod=bnMod;BigInteger.prototype.modPowInt=bnModPowInt;BigInteger.ZERO=nbv(0);BigInteger.ONE=nbv(1);";
static const char prng4_js[]   = "function Arcfour(){this.i=0;this.j=0;this.S=new Array();}function ARC4init(a){var b,c,d;for(b=0;b<256;++b)this.S[b]=b;c=0;for(b=0;b<256;++b){c=(c+this.S[b]+a[b%a.length])&255;d=this.S[b];this.S[b]=this.S[c];this.S[c]=d;}this.i=0;this.j=0;}function ARC4next(){var a;this.i=(this.i+1)&255;this.j=(this.j+this.S[this.i])&255;a=this.S[this.i];this.S[this.i]=this.S[this.j];this.S[this.j]=a;return this.S[(a+this.S[this.i])&255];}Arcfour.prototype.init=ARC4init;Arcfour.prototype.next=ARC4next;function prng_newstate(){return new Arcfour();}var rng_psize=256;";
static const char script_js[]  = "var base_url='http://192.168.0.1/';var network_list;var public_key;var rsa=new RSAKey();var scanButton=document.getElementById('scan-button');var connectButton=document.getElementById('connect-button');var copyButton=document.getElementById('copy-button');var showButton=document.getElementById('show-button');var deviceID=document.getElementById('device-id');var connectForm=document.getElementById('connect-form');var public_key_callback={success:function(a){console.log('Public key: '+a.b);public_key=a.b;rsa.setPublic(public_key.substring(58,58+256),public_key.substring(318,318+6));},error:function(a,b){console.log(a);window.alert('There was a problem fetching important information from your device. Please verify your connection, then reload this page.');}};var device_id_callback={success:function(a){var b=a.id;deviceID.value=b;},error:function(a,b){console.log(a);var c='COMMUNICATION_ERROR';deviceID.value=c;}};var scan=function(){console.log('Scanning...!');disableButtons();scanButton.innerHTML='Scanning...';connectButton.innerHTML='Connect';document.getElementById('connect-div').style.display='none';document.getElementById('networks-div').style.display='none';getRequest(base_url+'scan-ap',scan_callback);};var scan_callback={success:function(a){network_list=a.scans;console.log('I found:');var b=document.getElementById('networks-div');b.innerHTML='';if(network_list.length>0)for(var c=0;c<network_list.length;c++){ssid=network_list[c].ssid;console.log(network_list[c]);add_wifi_option(b,ssid);document.getElementById('connect-div').style.display='block';}else b.innerHTML='<p class=\\'scanning-error\\'>No networks found.</p>';},error:function(a){console.log('Scanning error:'+a);document.getElementById('networks-div').innerHTML='<p class=\\'scanning-error\\'>Scanning error.</p>';},regardless:function(){scanButton.innerHTML='Re-Scan';enableButtons();document.getElementById('networks-div').style.display='block';}};var configure=function(a){a.preventDefault();var b=get_selected_network();var c=document.getElementById('password').value;if(!b){window.alert('Please select a network!');return false;}var d={idx:0,ssid:b.ssid,pwd:rsa.encrypt(c),sec:b.sec,ch:b.ch};connectButton.innerHTML='Sending credentials...';disableButtons();console.log('Sending credentials: '+JSON.stringify(d));postRequest(base_url+'configure-ap',d,configure_callback);};var configure_callback={success:function(a){console.log('Credentials received.');connectButton.innerHTML='Credentials received...';postRequest(base_url+'connect-ap',{idx:0},connect_callback);},error:function(a,b){console.log('Configure error: '+a);window.alert('The configuration command failed, check that you are still well connected to the device\\'s WiFi hotspot and retry.');connectButton.innerHTML='Retry';enableButtons();}};var connect_callback={success:function(a){console.log('Attempting to connect to the cloud.');connectButton.innerHTML='Attempting to connect...';window.alert('Your device should now start flashing green and attempt to connect to the cloud. This usually takes about 20 seconds, after which it will begin slowly blinking cyan. \\n\\n\\nIf this process fails because you entered the wrong password, the device will flash green indefinitely. In this case, hold the setup button for 6 seconds until the device starts blinking blue again. Then reconnect to the WiFi hotspot it generates and reload this page to try again.');},error:function(a,b){console.log('Connect error: '+a);window.alert('The connect command failed, check that you are still well connected to the device\\'s WiFi hotspot and retry.');connectButton.innerHTML='Retry';enableButtons();}};var disableButtons=function(){connectButton.disabled=true;scanButton.disabled=true;};var enableButtons=function(){connectButton.disabled=false;scanButton.disabled=false;};var add_wifi_option=function(a,b){var c=document.createElement('INPUT');c.type='radio';c.value=b;c.id=b;c.name='ssid';c.className='radio';var d=document.createElement('DIV');d.className='radio-div';d.appendChild(c);var e=document.createElement('label');e.htmlFor=b;e.innerHTML=b;d.appendChild(e);a.appendChild(d);};var get_selected_network=function(){for(var a=0;a<network_list.length;a++){ssid=network_list[a].ssid;if(document.getElementById(ssid).checked)return network_list[a];}};var copy=function(){window.prompt('Copy to clipboard: Ctrl + C, Enter',deviceID.value);};var toggleShow=function(){var a=document.getElementById('password');inputType=a.type;if(inputType==='password'){showButton.innerHTML='Hide';a.type='text';}else{showButton.innerHTML='Show';a.type='password';}};var getRequest=function(a,b){var c=new XMLHttpRequest();c.open('GET',a,true);c.timeout=8000;c.send();c.onreadystatechange=function(){if(c.readyState==4)if(b){if(c.status==200){if(b.success)b.success(JSON.parse(c.responseText));}else if(b.error)b.error(c.status,c.responseText);if(b.regardless)b.regardless();}};};var postRequest=function(a,b,c){var d=JSON.stringify(b);var e=new XMLHttpRequest();e.open('POST',a,true);e.timeout=4000;e.setRequestHeader('Content-Type','multipart/form-data');e.send(d);e.onreadystatechange=function(){if(e.readyState==4)if(c){if(e.status==200){if(c.success)c.success(JSON.parse(e.responseText));}else if(c.error)c.error(e.status,e.responseText);if(c.regardless)c.regardless();}};};if(scanButton.addEventListener){copyButton.addEventListener('click',copy);showButton.addEventListener('click',toggleShow);scanButton.addEventListener('click',scan);connectForm.addEventListener('submit',configure);}else if(scanButton.attachEvent){copyButton.attachEvent('onclick',copy);showButton.attachEvent('onclick',toggleShow);scanButton.attachEvent('onclick',scan);connectForm.attachEvent('onsubmit',configure);}getRequest(base_url+'device-id',device_id_callback);getRequest(base_url+'public-key',public_key_callback);";

struct Page
{
    const char* url;
    const char* mime_type;
    const char* data;
};
Page myPages[] =
{
     { "/index",                "text/html",              index_html },
     { "/index.html",           "text/html",              index_html },
     { "/rsa-utils/rsa.js",     "application/javascript", rsa_js },
     { "/style.css",            "text/css",               style_css },
     { "/rsa-utils/rng.js",     "application/javascript", rng_js },
     { "/rsa-utils/jsbn_2.js",  "application/javascript", jsbn_2_js },
     { "/rsa-utils/jsbn_1.js",  "application/javascript", jsbn_1_js },
     { "/script.js",            "application/javascript", script_js },
     { "/rsa-utils/prng4.js",   "application/javascript", prng4_js },
     { nullptr }
};

static bool doSoftAP;
static bool createWiFi;

//static uint32_t timePublished;
//static byte countPublished;

static char deviceID[100];          // holds device ID as assigned by Particle
static char ipAddress[16] = {0};    // holds IP address as string: AAA.BBB.CCC.DDD
static char dataString[1000];       // long enough for maximum ?, ?S, ?P output strings

static char vstrConfigInfo[100];    // holds configuration string for variable PNUT_VARNAME_CONFIGINFO
#if (SEGMENT_COUNT > 1)
static char vstrSegsInfo[100];      // holds segment string for variable PNUT_VARNAME_SEGINFO
static char vstrSegsVals[SEGMENT_COUNT][100]; // holds segment strings for variables with prefix PNUT_VARNAME_SEGINFO_PFX
static bool segStrFirst = true;     // false after first segment info string is retrieved
#endif
static short segmentCount = 0;      // nonzero for state of retrieving segment info strings

static const char indexpage[] = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>PixelNut!</title></head><body><h1>PixelNut!</h1></body></html>";

static void CmdSequence(const char *data);

static void SendBeacon(void);
Timer timerBeacon(3000, SendBeacon);

void httpHandler(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved)
{
  DBGOUT((F("SoftAP(%d): URL=%s"), createWiFi, url));

  if (createWiFi) // used by PixelNutCtrl application
  {
    // put up home page (just shows string: 'PixelNut!')
    if (!strcmp(url, "/index") || !strcmp(url, "/index.html"))
    {
      cb(cbArg, 0, 200, "text/html", nullptr);
      result->write(indexpage);
    }
    // endpoint for command execution
    else if (!strcmp(url, "/command"))
    {
      cb(cbArg, 0, 200, "text/plain", nullptr);

      if (body->bytes_left)
      {
        char *data = body->fetch_as_string();
        //DBGOUT((F("Cmd=\"%s\""), data));

        if (*data == '?') // single command to retrive info
        {
          dataString[0] = 0;
          pAppCmd->execCmd((char*)"?");
          result->write(dataString);
        }
        else // process (multiple) command(s)
        {
          CmdSequence(data);
          result->write("ok");
        }

        free(data);
      }
    }
    else if (!strcmp(url, "/generate_204"))
    {
      cb(cbArg, 0, 204, nullptr, nullptr);
      result->write("");
    }
    else
    {
      cb(cbArg, 0, 404, nullptr, nullptr);
      result->write("");
    }
  }
  else // used by the Particle application
  {
    int8_t idx = 0;
    for (;;idx++)
    {
      Page& p = myPages[idx];
      if (!p.url)
      {
        idx = -1;
        break;
      }
      else if (strcmp(url, p.url) == 0)
        break;
    }

    if (idx != -1)
    {
      cb(cbArg, 0, 200, myPages[idx].mime_type, nullptr);
      result->write(myPages[idx].data);
      return;
    }
    else
    {
      cb(cbArg, 0, 404, nullptr, nullptr);
      result->write("");
    }
  }
}

// Private network IP address: http://192.168.0.1
STARTUP(softap_set_application_page_handler(httpHandler, nullptr));

static void RestartDevice(void)
{
  // this doesn't work: WiFi.hasCredentials() hangs then reboots
  //DBGOUT((F("Restart device...")));
  //WiFi.listen(false);   // turn off listening mode
  //SetupDevice();        // restart setup process

  DBGOUT((F("Rebooting!!!")));
  delay(1000);
  System.reset(); // just reboot right away
}

static bool ClearWiFiCredentials(bool failstop)
{
  if (WiFi.clearCredentials())
  {
    DBGOUT((F("WiFi credentials cleared")));
    return true;
  }

  DBGOUT((F("Failed to clear WiFi credentials")));
  ErrorHandler(3, 3, failstop); // does not return if failstop=true
  return false;
}

static bool SetSSID(char *str)
{
  char ssid[32];
  char pass[32];

  int i = 0;
  while (*str == ' ') ++str; // skip spaces
  while(*str)
  {
    if (*str == ' ') break;
    ssid[i++] = *str++;
  }
  ssid[i] = 0;

  i = 0; // reset index for pass
  while (*str == ' ') ++str; // skip spaces
  while(*str)
  {
    if (*str == ' ') break;
    pass[i++] = *str++;
  }
  pass[i] = 0;

  if (ssid[0] != 0)
  {
    DBGOUT((F("WiFi credentials: ssid=%s pass=%s"), ssid, pass));
    WiFi.setCredentials(ssid, pass);
    /*
    WiFiAccessPoint ap[5];
    int found = WiFi.getCredentials(ap, 5);
    for (int i = 0; i < found; i++)
    {
        Serial.print(i);
        Serial.print(") SSID=");
        Serial.println(ap[i].ssid);
    }
    */

    EEPROM.write(FLASHOFF_PARTICLE_MODE, PARTICLE_MODE_CLOUD); // connect to cloud
    return true;
  }

  EEPROM.write(FLASHOFF_PARTICLE_MODE, PARTICLE_MODE_WIFI); // use own SoftAP mode
  ClearWiFiCredentials(true); // doesn't return if fails
  return false;
}

// splits command sequence (as defined by newline chars)
// into separate strings to be individually processed
static void CmdSequence(const char *data)
{
  char instr[STRLEN_PATTERNS];
  while (*data)
  {
    int i = 0;
    while (*data && (*data != '\n'))
      instr[i++] = *data++;

    instr[i] = 0;
    if (*data == '\n') ++data;

    if (i > 0)
    {
      DBGOUT((F("Web --> \"%s\""), instr));

      // special command to load/clear WiFi credentials
      if (!strncmp(instr, "*wifi*", 6))
      {
        SetSSID(instr+6);
        RestartDevice();
      }
      // this is where PixelNut commands get processed
      else if (pAppCmd->execCmd(instr)) CheckExecCmd(instr);
    }
  }
}

static void SendBeacon(void)
{
  char outstr[100];
  sprintf(outstr, "IP=%s ID=%s", ipAddress, deviceID);

  if (!Particle.publish(PNUT_PUBNAME_BEACON, outstr, 60, PRIVATE))
  {
    DBGOUT((F("Particle publish(Beacon) failed")));
    ErrorHandler(3, 1, false);
  }
  else
  {
    DBGOUT((F("Published: %s"), outstr));
    BlinkStatusLED(1, 0); // indicate sending beacon
  }
}

static int funBeacon(String funstr)
{
  bool turnon = funstr.toInt() == 1;
  DBGOUT((F("Beacon function: %s"), (turnon ? "On" : "Off")));

  if (!timerBeacon.isActive() && turnon)
  {
    EEPROM.write(FLASHOFF_PARTICLE_MODE, PARTICLE_MODE_BEACON);
    timerBeacon.start();
  }
  else
  if (timerBeacon.isActive() && !turnon)
  {
    EEPROM.write(FLASHOFF_PARTICLE_MODE, PARTICLE_MODE_CLOUD);
    timerBeacon.stop();
  }
  else
  {
    DBGOUT((F("Beacon: no change of state")));
    return 0;
  }

  return 1;
}

static int funSetSSID(String funstr)
{
  char instr[100];
  funstr.toCharArray(instr, 100);

  // reboot if changing modes to use own WiFi SoftAP
  if (!SetSSID(instr)) RestartDevice();

  return 0;
}

#if DEBUG_OUTPUT
static int funRestart(String funstr)
{
  RestartDevice();
  return 0;
}
#endif

static void cmdHandler(const char *name, const char *data)
{
  if (*data) // have some data
  {
    if (!strncmp(name, PNUT_SUBNAME_DEVCMD_PFX, strlen(PNUT_SUBNAME_DEVCMD_PFX)))
    {
      DBGOUT((F("Device Command: %s"), data));

      strcpy(dataString, data); // MUST save to local storage
      // this is where PixelNut commands get processed
      if (pAppCmd->execCmd(dataString)) CheckExecCmd(dataString);
    }
    #if DEBUG_OUTPUT
    else if (!strcmp(name, PNUT_SUBNAME_PRODCMD))
    {
      DBGOUT((F("Global Command: %s"), data));

      strcpy(dataString, data); // MUST save to local storage
      // this is where PixelNut commands get processed
      if (pAppCmd->execCmd(dataString)) CheckExecCmd(dataString);
    }
    #endif
    else { DBGOUT((F("CmdHandler: unknown name=%s"), name)); }
  }
  else { DBGOUT((F("CmdHandler: empty data, name=%s"), name)); }
}

static void sparkHandler(const char *name, const char *data)
{
  if (!strcmp(name, PARTICLE_SUBNAME_IPADDR))
  {
    if (*data && (strlen(data) <= 15)) // insure string isn't too short/long
    {
      strcpy(ipAddress, data);
      DBGOUT((F("Found IP address: %s"), ipAddress));

      if (EEPROM.read(FLASHOFF_PARTICLE_MODE) == PARTICLE_MODE_BEACON)
        timerBeacon.start();
    }
    else // NOTE: have seen this be garbage the first time powered up FIXME: must retry
    {
      DBGOUT((F("Invalid IP address: %s"), data));
      ErrorHandler(3, 2, true); // does not return from this call
    }
  }
  /*
  else if (!strcmp(name, PARTICLE_SUBNAME_DEVICE))
  {
    DBGOUT((F("Saving device name: \"%s\""), data));
    FlashSetName((char*)data);
  }
  */
  else { DBGOUT((F("SparkHandler: name=%s data=%s"), name, data)); }
}

static void Subscribe(char *name, PubHandler handler)
{
  if (!Particle.subscribe(name, handler, MY_DEVICES))
  {
    DBGOUT((F("Particle subscribe(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static void SetFunction(char *name, FunHandler handler)
{
  if (!Particle.function(name, handler))
  {
    DBGOUT((F("Particle function(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static void SetVariable(char *name, char *varstr)
{
  if (!Particle.variable(name, varstr))
  {
    DBGOUT((F("Particle variable(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static bool ConnectToCloud(void)
{
  //timePublished = pixelNutSupport.getMsecs();
  //countPublished = 0;

  DBGOUT((F("Connecting to network...")));
  Particle.connect();

  int count = 0;
  uint32_t time = millis();
  while (!WiFi.ready())
  {
    //Particle.process();   // don't need this since have system thread
    BlinkStatusLED(1, 0);   // connecting to cloud with local network

    if ((millis() - time) > 1000)  // timeout: bad credentials?
    {
      if (++count >= 10)
      {
        DBGOUT((F("Failed: use Particle App to configure")));
        EEPROM.write(FLASHOFF_PARTICLE_MODE, PARTICLE_MODE_BEACON);
        ClearWiFiCredentials(true); // doesn't return if fails
        return false;
      }

      DBGOUT((F("...waiting to connect")));
      time = millis();
    }
  }

  DBGOUT((F("Connected to Particle Cloud")));

  #if DEBUG_OUTPUT
  Serial.print("  SSID:    "); Serial.println(WiFi.SSID());
  Serial.print("  LocalIP: "); Serial.println(WiFi.localIP());
  //Serial.print("  Subnet:  "); Serial.println(WiFi.subnetMask());
  //Serial.print("  Gateway: "); Serial.println(WiFi.gatewayIP());
  #endif

  DBGOUT((F("Configuring device...")));

  if (!Particle.subscribe(PARTICLE_SUBNAME_IPADDR, sparkHandler) ||
      !Particle.publish(PARTICLE_SUBNAME_IPADDR))
  {
    DBGOUT((F("Retrieving IP address failed")));
    ErrorHandler(3, 1, true); // does not return from this call
  }

  /*
  if (!Particle.subscribe(PARTICLE_SUBNAME_DEVICE, sparkHandler) ||
      !Particle.publish(PARTICLE_SUBNAME_DEVICE))
  {
    DBGOUT((F("Retrieving device name failed")));
    ErrorHandler(3, 1, true); // does not return from this call
  }
  */

  char cmdname[100];
  sprintf(cmdname, "%s-%s", PNUT_SUBNAME_DEVCMD_PFX, deviceID);
  Subscribe(cmdname, cmdHandler);

  #if DEBUG_OUTPUT
  Subscribe((char*)PNUT_SUBNAME_PRODCMD, cmdHandler);
  #endif

  SetFunction((char*)PNUT_FUNNAME_BEACON,  funBeacon);
  SetFunction((char*)PNUT_FUNNAME_SETSSID, funSetSSID);

  #if DEBUG_OUTPUT
  SetFunction((char*)PNUT_FUNNAME_RESTART, funRestart);
  #endif

  segmentCount = 0;
  dataString[0] = 0;
  pAppCmd->execCmd((char*)"?"); // get main configuration strings
  strcpy(vstrConfigInfo, dataString);
  SetVariable((char*)PNUT_VARNAME_CONFIGINFO, vstrConfigInfo);

  #if (SEGMENT_COUNT > 1)
  segStrFirst = true;
  for (segmentCount = 1; segmentCount <= SEGMENT_COUNT; ++segmentCount)
  {
    dataString[0] = 0;
    pAppCmd->execCmd((char*)"?S"); // get segment configuration strings
  }
  segStrFirst = true; // reset in case can switch modes without reboot

  SetVariable((char*)PNUT_VARNAME_SEGINFO, vstrSegsInfo);

  for (int seg = 1; seg <= SEGMENT_COUNT; ++seg)
  {
    sprintf(cmdname, "%s-%d", PNUT_VARNAME_SEGVALS_PFX, seg);
    SetVariable(cmdname, vstrSegsVals[seg-1]);
  }
  #endif

  DBGOUT((F("...Finished")));
  return true;
}

static void SetDeviceName(void)
{
  char devname[MAXLEN_DEVICE_NAME+1];
  devname[0] = 0;
  pAppCmd->setDeviceName(devname);
  char netname[MAXLEN_DEVICE_NAME+3];
  strcpy(netname, "P!");
  strcat(netname, devname);
  
  DBGOUT((F("SoftAP name: \"%s\""), netname));
  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, netname);
  System.set(SYSTEM_CONFIG_SOFTAP_SUFFIX, "!   ");
}

static void SetupDevice(void)
{
  // Possible scenarios:
  // 1) Start in custom SoftAP mode for use with PixelNut app.
  //    (createWiFi = true, doSoftAP = true)
  // 2) Using the app, user chose to connect to the Cloud,
  //    so now we start in SoftAP mode but use built-in
  //    web pages to perform the configuration with the
  //    Particle application.
  //    (createWiFi = false, doSoftAP = true)
  // 3) Have already configured, so use local WiFi to
  //    connect to and be controlled from the Cloud.
  //    (createWiFi = false, doSoftAP = false)

  DBGOUT((F("Setup device...")));
  do
  {
    createWiFi = EEPROM.read(FLASHOFF_PARTICLE_MODE) == PARTICLE_MODE_WIFI;
    if (createWiFi) doSoftAP = true;
    else doSoftAP = !WiFi.hasCredentials();
    DBGOUT((F("CreateWiFi=%d SoftAP=%d"), createWiFi, doSoftAP));

    if (doSoftAP)
    {
      SetDeviceName();

      DBGOUT((F("Setting SoftAP:  P!mode=%s..."), (createWiFi ? "true" : "false")));
      if (!WiFi.listening()) WiFi.listen(); // put into SoftAP mode if not already

      if (createWiFi) return; // use PixelNutCtrl application

      uint32_t time = millis();
      while(true)
      {
        BlinkStatusLED(0, 2); // connect with browser to set SSID

        if ((millis() - time) > 1000)
        {
          //DBGOUT((F("Listening=%d"), WiFi.listening()));
          //DBGOUT((F("Connecting=%d"), WiFi.connecting()));
          if (!WiFi.listening() && !WiFi.connecting())
            break;

          Particle.process();   // don't need this since have system thread?
          time = millis();
        }
      }
    }
  }
  while (!ConnectToCloud());
}

class Ethernet : public CustomCode
{
public:

  #if EEPROM_FORMAT
  // used to initialize the operating mode according to device configuration
  virtual void flash(void)
  {
    int value = (USE_WIFI_DIRECT ? PARTICLE_MODE_WIFI : PARTICLE_MODE_BEACON);
    DBGOUT((F("Writing EEPROM configuration: address=%d flag=%d"), FLASHOFF_PARTICLE_MODE, value));
    EEPROM.write(FLASHOFF_PARTICLE_MODE, value);
  }
  #endif

  // Entry point on bootup for all modes of operation
  void setup(void)
  {
    byte instr[100];
    DBGOUT((F("Particle Photon:")));
    DBGOUT((F("  Version=%s"), System.version().c_str()));
    System.deviceID().toCharArray(deviceID, 100);
    DBGOUT((F("  DeviceID=%s"), deviceID));
    DBGOUT((F("  Memory=%d free bytes"), System.freeMemory()));
    Time.timeStr().getBytes(instr, 100);
    DBGOUT((F("  Time=%s"), instr));

    SetupDevice();
  }

  // used by PixelNutCtrl application only
  bool setName(char *name)
  {
    if (!strcmp(name, "*wifi*"))
    {
      DBGOUT((F("Device => user config")));
      EEPROM.write(FLASHOFF_PARTICLE_MODE, PARTICLE_MODE_BEACON);
      ClearWiFiCredentials(false); // always returns
      RestartDevice();
    }
    else
    {
      FlashSetName((char*)name);
      SetDeviceName();
      // FIXME: must restart device for name change to take effect
    }
    return true;
  }

  // used by both the PixelNutCtrl application and
  // when connected to the Particle Cloud to retrieve
  // information strings from the PixelNut application
  bool sendReply(char *instr)
  {
    if (segmentCount == 0)
    {
      DBGOUT((F("ReplyStr: \"%s\""), instr));
      strcat(dataString, instr);
      strcat(dataString, "\r\n");
    }

    #if (SEGMENT_COUNT > 1)
    else if (segStrFirst)
    {
      strcpy(vstrSegsInfo, instr);
      segStrFirst = false;
    }
    else strcpy(vstrSegsVals[segmentCount-1], instr);
    #endif

    return true; // this cannot fail
  }
};
Ethernet ethernet;

#endif // defined(SPARK)
#endif // ETHERNET_COMM
//========================================================================================
