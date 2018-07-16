// PixelNutApp WiFi Communications
//
// Uses global variables: 'pixelNutSupport', 'pAppCmd'.
// Calls global routines: 'CheckExecCmd', 'ErrorHandler', 'BlinkStatusLED'.
//========================================================================================
/*
Copyright (c) 2015-2018, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if WIFI_COMM
#if defined(SPARK)

extern void CheckExecCmd(char *instr); // defined in main.h

#include "softap_http.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

PRODUCT_ID(6876);   // ID for the "PixelNutDevice"
PRODUCT_VERSION(1);

// Blink patterns (long, short):
// 0,1  waiting for debugger
// 0,2  once: startup successful
// 0,3  EEPROM format finished
// 1,0  Connecting to wifi/cloud
// 2,0  Sending Beacon message
// else an error message

#define FLASHOFF_PARTICLE_MODE    FLASHOFF_PATTERN_END
#define PARTICLE_MODE_SOFTAP_BIT  0x80                    // bit=1 to start in SoftAP mode
#define PARTICLE_MODE_BEACON_MAX  0x7F                    // max value for beacon counter (forever)

typedef int  (*FunHandler)(String arg);                         // handler for subscribe event
typedef void (*PubHandler)(const char *name, const char *data); // handler for function calls

#define POSTFIX_DEVNAME           "--!P"                  // postfix on device name

#define PARTICLE_SUBNAME_IPADDR   "particle/device/ip"
//#define PARTICLE_SUBNAME_DEVICE   "particle/device/name"

#define PNUT_SUBNAME_DEVCMD_PFX   "PnutCmd"               // prefix for events targeting specific device
#define PNUT_SUBNAME_PRODCMD      "PixelNutCommand"       // name of event that reaches every PixelNut device (only in debug)
#define PNUT_PUBNAME_BEACON       "PixelNutBeacon"        // name of event broadcast from devices when unattached
#define PNUT_FUNNAME_BEACON       "Beacon"                // name of function to enable/disable the beacon
#define PNUT_FUNNAME_SOFTAP       "SoftAP"                // name of function to set startup mode
#define PNUT_FUNNAME_RESTART      "Restart"               // name of function to cause device reboot

#define PNUT_VARNAME_NETWORKS     "Networks"              // name of variable with list of configured network SSIDs
#define PNUT_VARNAME_CONFIGINFO   "ConfigInfo"            // name of variable with configuration info
#define PNUT_VARNAME_SEGINFO      "SegInfo"               // name of variable with segment offsets/lengths
#define PNUT_VARNAME_SEGVALS_PFX  "SegVals"               // prefix for variables with segment property values

static const char* WLAN_Security_Strs[] = { "None", "WEP", "WPA", "WPA2", 0 };
static const int   WLAN_Security_Vals[] = { 0, 1, 2, 3 };

static const char* WLAN_Cipher_Strs[] = { "None", "AES", "TKIP", "AES_TKIP", 0 };
static const int   WLAN_Cipher_Vals[] = { 0, 1, 2, 3 };

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

static char deviceID[100];                    // holds device ID as assigned by Particle
static char ipAddress[16] = {0};              // holds IP address as string: AAA.BBB.CCC.DDD
static char dataString[1000];                 // long enough for maximum ?, ?S, ?P output strings
static char cmdString[STRLEN_PATTERNS];       // long enough for maximum PixelNut command

static char vstrNetworks[200];                // holds list of network SSID names for variable PNUT_VARNAME_NETWORKS
static char vstrConfigInfo[100];              // holds configuration string for variable PNUT_VARNAME_CONFIGINFO
#if (SEGMENT_COUNT > 1)
static char vstrSegsInfo[100];                // holds segment string for variable PNUT_VARNAME_SEGINFO
static char vstrSegsVals[SEGMENT_COUNT][100]; // holds segment strings for variables with prefix PNUT_VARNAME_SEGINFO_PFX
static bool segStrFirst = true;               // false after first segment info string is retrieved
#endif
static short segmentCount = 0;                // nonzero for state of retrieving segment info strings

#if USE_PARTICLE_NAME
static char* deviceName = PREFIX_DEVICE_NAME "Photon"; // needed to be recognized by Particle (after prefix removed)
#else
static char* deviceName = DEFAULT_DEVICE_NAME;
#endif

static void SendBeacon(void);
Timer timerBeacon(3000, SendBeacon);
static int beaconCounter = 0;

static void ScanForNetworks(void);
static boolean SetupCloud(void);
static void ProcessCmd(char *cmdstr, Writer* result);

static uint32_t rebootTime = 0;

void httpHandler(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved)
{
  // endpoint for command execution used by PixelNut application
  if (!strcmp(url, "/command"))
  {
    cb(cbArg, 0, 200, "text/plain", nullptr);

    if (body->bytes_left)
    {
      char *data = body->fetch_as_string();
      //DBGOUT((F("Command: \"%s\""), data));

      // splits command sequence (as defined by newline chars)
      // into separate strings to be individually processed

      char *p = data;
      while (*p)
      {
        int i = 0;
        while (*p && (*p != '\n'))
          cmdString[i++] = *p++;

        cmdString[i] = 0;
        if (*p == '\n') ++p;

        if (i > 0) ProcessCmd(cmdString, result);
      }

      free(data);
    }
  }
  else // used to load above assets held in strings above
  {
    DBGOUT((F("SoftAP: URL=%s"), url));

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
  DBGOUT((F("Rebooting!")));
  System.reset(); // this never returns
}

static void SetSoftAP(boolean enable)
{
  int bit = (enable ? PARTICLE_MODE_SOFTAP_BIT : 0);
  int count = EEPROM.read(FLASHOFF_PARTICLE_MODE) & PARTICLE_MODE_BEACON_MAX;
  DBGOUT((F("Setting SoftAP: value=%02X"), (bit | count)));
  EEPROM.write(FLASHOFF_PARTICLE_MODE, (bit | count));
}

static void SetBeacon(int count)
{
  if (!timerBeacon.isActive() && (count > 0))
  {
    DBGOUT((F("Beacon: count=%d"), count));
    int bit = EEPROM.read(FLASHOFF_PARTICLE_MODE) & PARTICLE_MODE_SOFTAP_BIT;
    EEPROM.write(FLASHOFF_PARTICLE_MODE, (bit | count));
    beaconCounter = count;
    timerBeacon.start();
  }
  else
  if (timerBeacon.isActive() && !count)
  {
    DBGOUT((F("Beacon: Off")));
    int bit = EEPROM.read(FLASHOFF_PARTICLE_MODE) & PARTICLE_MODE_SOFTAP_BIT;
    EEPROM.write(FLASHOFF_PARTICLE_MODE, bit);
    timerBeacon.stop();
  }
  else { DBGOUT((F("Beacon: no change"))); }
}

static void ScanForNetworks(void)
{
  WiFiAccessPoint aps[16];
  int found = WiFi.scan(aps, 16);

  for (int i = 0; i < found; ++i)
  {
    WiFiAccessPoint& ap = aps[i];
    if (ap.ssid[0])
    {
      DBGOUT((F("   SSID:     %s"), ap.ssid));
      DBGOUT((F("   RSSI:     %d"), ap.rssi));
      DBGOUT((F("   Security: %s"), WLAN_Security_Strs[ap.security]));
      DBGOUT((F("   Cipher:   %s"), WLAN_Cipher_Strs[ap.cipher]));
      DBGOUT((F("   Channel:  %d"), ap.channel));

      strcat(dataString, ap.ssid);
      strcat(dataString, " ");
      strcat(dataString, WLAN_Security_Strs[ap.security]);
      strcat(dataString, " ");
      strcat(dataString, WLAN_Cipher_Strs[ap.cipher]);
      strcat(dataString, "\n");
    }
    else
    {
      DBGOUT((F("Empty SSID!")));
    }
  }
}

static void SaveNetworkNames(void)
{
  vstrNetworks[0] = 0;

  WiFiAccessPoint aps[5];
  int found = WiFi.getCredentials(aps, 5);

  for (int i = 0; i < found; i++)
  {
    WiFiAccessPoint& ap = aps[i];
    strcat(vstrNetworks, ap.ssid);

    if (ap.security > 0)
    {
      strcat(vstrNetworks, " ");
      strcat(vstrNetworks, WLAN_Security_Strs[ap.security]);

      if (ap.cipher > 0)
      {
        strcat(vstrNetworks, " ");
        strcat(vstrNetworks, WLAN_Cipher_Strs[ap.cipher]);
      }
    }

    strcat(vstrNetworks, "\n");

    DBGOUT((F(vstrNetworks)));
    //DBGOUT((F("%d) SSID=%s (%d,%d,%d)"), i+1, ap.ssid, ap.security, ap.cipher, ap.channel));
  }
}

static char* SetNetwork(char *str)
{
  char ssid[100];
  char pass[100];
  char sec[100];
  char cip[100];
  int security = 0;
  int cipher = 0;

  int i = 0;
  str = pAppCmd->skipSpaces(str);
  while(*str)
  {
    if (*str == ' ') break;
    ssid[i++] = *str++;
  }
  ssid[i] = 0;

  if (i == 0)
  {
    if (!WiFi.clearCredentials())
    {
      DBGOUT((F("Failed to clear WiFi credentials")));
      ErrorHandler(3, 3, false);
      return NULL;
    }

    DBGOUT((F("All WiFi credentials cleared")));
    SaveNetworkNames(); // should be empty string
    return str;
  }

  i = 0;
  str = pAppCmd->skipSpaces(str);
  while(*str)
  {
    if (*str == ' ') break;
    pass[i++] = *str++;
  }
  pass[i] = 0;

  // special placeholder for empty passphrase
  if (!strcmp(pass, "*")) pass[0] = 0;

  i = 0;
  str = pAppCmd->skipSpaces(str);
  while(*str)
  {
    if (*str == ' ') break;
    sec[i++] = toupper(*str++);
  }
  sec[i] = 0;

  if (i > 0)
  {
    for (int j = 0; WLAN_Security_Strs[j]; ++j)
    {
      if (!strcmp(WLAN_Security_Strs[j], sec))
      {
        security = WLAN_Security_Vals[j];
        break;
      }
    }
  }

  i = 0;
  str = pAppCmd->skipSpaces(str);
  while(*str)
  {
    if (*str == ' ') break;
    cip[i++] = toupper(*str++);
  }
  cip[i] = 0;

  if (i > 0)
  {
    for (int j = 0; WLAN_Cipher_Strs[j]; ++j)
    {
      if (!strcmp(WLAN_Cipher_Strs[j], sec))
      {
        cipher = WLAN_Cipher_Vals[j];
        break;
      }
    }
  }

  DBGOUT((F("WiFi credentials: ssid=%s pass=%s sec=%s cip=%s"), ssid, pass, sec, cip));

  bool success;

       if (cipher > 0)   success = WiFi.setCredentials(ssid, pass, security, cipher);
  else if (security > 0) success = WiFi.setCredentials(ssid, pass, security);
  else                   success = WiFi.setCredentials(ssid, pass);

  if (!success)
  {
    DBGOUT((F("Failed to set WiFi credentials")));
    ErrorHandler(3, 3, false);
    return NULL;
  }

  SaveNetworkNames(); // save new network names
  return str;
}

static void SendBeacon(void)
{
  char outstr[100];
  sprintf(outstr, "%s %s", deviceID, ipAddress);

  if (!Particle.publish(PNUT_PUBNAME_BEACON, outstr, 60, PRIVATE))
  {
    DBGOUT((F("Particle publish(Beacon) failed")));
    ErrorHandler(3, 1, false);
  }
  else
  {
    DBGOUT((F("Published: %s"), outstr));
    BlinkStatusLED(2, 0); // indicate sending beacon

    if (!--beaconCounter) timerBeacon.stop();
  }
}

static void ProcessCmd(char *cmdstr, Writer* result)
{
  DBGOUT((F("Process: \"%s\""), cmdstr));

  bool success = true;

  if (*cmdstr == '*') // escape for Particle specific commands
  {
    bool badcmd = false;
    char *p = cmdstr;
    ++p; // skip over *

    while (*p)
    {
      switch (toupper(*p))
      {
        default:
        {
          badcmd = true;
          break;
        }
        case 'A': // set AP mode
        {
          SetSoftAP(*(p+1) == '1');
          p = pAppCmd->skipNumber(p+1);
          break;
        }
        case 'C': // connect to Cloud
        {
          // BUG: doesn't work: make bug report!
          ++p; // skip over command

          if (!WiFi.listening())
          {
            DBGOUT((F("Already connected to the Cloud!")));
            break;
          }

          WiFi.listen(false);
          if (WiFi.listening())
          {
            DBGOUT((F("Failed to turn off SoftAP mode!")));
            break;
          }

          if (!SetupCloud())
          {
            // return to SoftAP mode
            if (!WiFi.listening()) WiFi.listen();
            success = false;
          }

          break;
        }
        //*/
        case 'B': // set Beacon counter
        {
          int count = atoi(p+1);
          SetBeacon(count);
          p = pAppCmd->skipNumber(p+1);
          break;
        }
        case 'N': // add/clear network SSID/PWD values
        {
          p = SetNetwork(p+1);
          if (p == NULL) success = false;
          break;
        }
        case 'R': // Reboot
        {
          ++p; // skip over command
          if (result) rebootTime = millis(); // wait for response
          else RestartDevice(); // never returns
          break;
        }
      }

      if (badcmd)
      {
        ErrorHandler(4, 1, false);
        success = false;
        break;
      }

      if (p == NULL) break;
      p = pAppCmd->skipSpaces(p);
      //DBGOUT((F("Next: \"%s\""), p));
    }
  }
  else if (*cmdstr == '?') // single command to retrive info
  {
    dataString[0] = 0;

    if (toupper(*(cmdstr+1)) == 'A')
    {
      DBGOUT((F("Retrieving Available Networks")));
      ScanForNetworks();
    }
    else if (toupper(*(cmdstr+1)) == 'N')
    {
      DBGOUT((F("Retrieving Stored Networks")));
      DBGOUT((F(vstrNetworks)));
      strcpy(dataString, vstrNetworks);
    }
    else
    {
      DBGOUT((F("Retrieving Configuration: %s"), cmdstr));
      pAppCmd->execCmd(cmdstr);
    }

    if (result) result->write(dataString);
    return; // avoid writing result again
  }
  // where all other PixelNut commands get processed
  else if (pAppCmd->execCmd(cmdstr)) CheckExecCmd(cmdstr);

  if (result) result->write(success? "ok" : "failed");
}

static int funBeacon(String funstr)
{
  SetBeacon(funstr.toInt());
  return 1;
}

static int funSoftAP(String funstr)
{
  SetSoftAP(funstr.toInt());
  return 1;
}

static int funRestart(String funstr)
{
  RestartDevice();
  return 1;
}

static void cmdHandler(const char *name, const char *data)
{
  if (*data) // have some data
  {
    if (!strncmp(name, PNUT_SUBNAME_DEVCMD_PFX, strlen(PNUT_SUBNAME_DEVCMD_PFX)))
    {
      DBGOUT((F("Device Command: \"%s\""), data));
      strcpy(cmdString, data); // MUST save to local storage
      ProcessCmd(cmdString, 0);
    }
    #if DEBUG_OUTPUT
    else if (!strcmp(name, PNUT_SUBNAME_PRODCMD))
    {
      DBGOUT((F("Global Command: \"%s\""), data));
      strcpy(cmdString, data); // MUST save to local storage
      ProcessCmd(cmdString, 0);
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

      int val = EEPROM.read(FLASHOFF_PARTICLE_MODE);
      int count = val & ~PARTICLE_MODE_SOFTAP_BIT;
      if (count) beaconCounter = count;
      else beaconCounter = 1; // always send at least one beacon on startup
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

static void Publish(const char *name)
{
  if (!Particle.publish(name))
  {
    DBGOUT((F("Particle publish(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static void Subscribe(const char *name, PubHandler handler)
{
  if (!Particle.subscribe(name, handler, MY_DEVICES))
  {
    DBGOUT((F("Particle subscribe(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static void SetFunction(const char *name, FunHandler handler)
{
  if (!Particle.function(name, handler))
  {
    DBGOUT((F("Particle function(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static void SetVariable(const char *name, char *varstr)
{
  if (!Particle.variable(name, varstr))
  {
    DBGOUT((F("Particle variable(%s) failed"), name));
    ErrorHandler(3, 1, true); // does not return from this call
  }
}

static boolean SetupCloud(void)
{
  DBGOUT((F("---------------------------------------")));
  DBGOUT((F("Setup Cloud:")));

  WiFi.connect();

  int count = 0;
  uint32_t time = millis();
  while (!WiFi.ready())
  {
    BlinkStatusLED(1, 0);

    if ((millis() - time) > 1000)
    {
      if (++count > 10) // timeout after 10 seconds
      {
        DBGOUT((F("Failed to connect to WiFi: use SoftAP")));
        return false;
      }

      DBGOUT((F("Waiting for WiFi connection...")));
      time = millis();
    }
  }

  Particle.connect();

  count = 0;
  time = millis();
  while (!Particle.connected())
  {
    BlinkStatusLED(1, 0);

    if ((millis() - time) > 1000)
    {
      if (++count > 5) // timeout after 5 seconds
      {
        DBGOUT((F("Failed to connect to Cloud: use SoftAP")));
        return false;
      }

      DBGOUT((F("Waiting for Cloud connection...")));
      time = millis();
    }
  }

  IPAddress localip = WiFi.localIP();
  uint32_t *addr = (uint32_t*)&localip;

  DBGOUT((F("SSID:    %s"), WiFi.SSID()));
  DBGOUT((F("RSSI:    %s"), WiFi.RSSI()));

  if (*(uint32_t*)&addr == 0)
  {
    DBGOUT((F("Failed to connect to Cloud: use SoftAP")));
    return false;
  }
  DBGOUT((F("LocalIP: %d.%d.%d.%d"), addr[0], addr[1], addr[2], addr[4]));

  Subscribe(PARTICLE_SUBNAME_IPADDR, sparkHandler);
  Publish(PARTICLE_SUBNAME_IPADDR);

  /*
  Subscribe(PARTICLE_SUBNAME_DEVICE, sparkHandler);
  Publish(PARTICLE_SUBNAME_DEVICE);
  */

  char cmdname[100];
  sprintf(cmdname, "%s-%s", PNUT_SUBNAME_DEVCMD_PFX, deviceID);
  Subscribe(cmdname, cmdHandler);

  #if DEBUG_OUTPUT
  Subscribe(PNUT_SUBNAME_PRODCMD, cmdHandler);
  #endif

  SetFunction(PNUT_FUNNAME_BEACON,    funBeacon);
  SetFunction(PNUT_FUNNAME_SOFTAP,    funSoftAP);
  SetFunction(PNUT_FUNNAME_RESTART,   funRestart);

  DBGOUT((F("Device Configuration:")));

  SetVariable(PNUT_VARNAME_NETWORKS, vstrNetworks);

  segmentCount = 0;
  dataString[0] = 0;
  pAppCmd->execCmd((char*)"?"); // get main configuration strings
  strcpy(vstrConfigInfo, dataString);
  SetVariable(PNUT_VARNAME_CONFIGINFO, vstrConfigInfo);

  #if (SEGMENT_COUNT > 1)
  segStrFirst = true;
  for (segmentCount = 1; segmentCount <= SEGMENT_COUNT; ++segmentCount)
  {
    dataString[0] = 0;
    pAppCmd->execCmd((char*)"?S"); // get segment configuration strings
  }
  segStrFirst = true; // reset in case can switch modes without reboot

  SetVariable(PNUT_VARNAME_SEGINFO, vstrSegsInfo);

  for (int seg = 1; seg <= SEGMENT_COUNT; ++seg)
  {
    sprintf(cmdname, "%s-%d", PNUT_VARNAME_SEGVALS_PFX, seg);
    SetVariable(cmdname, vstrSegsVals[seg-1]);
  }
  #endif

  DBGOUT((F("---------------------------------------")));
  return true;
}

static void SetNameForSoftAP(char *name1, char *name2)
{
  DBGOUT((F("SoftAP Name: \"%s\""), name1));
  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, name1);
  System.set(SYSTEM_CONFIG_SOFTAP_SUFFIX, name2);
}

class WiFiNet : public CustomCode
{
public:

  #if EEPROM_FORMAT
  // used to initialize the operating mode according to device configuration
  virtual void flash(void)
  {
    int bit = (USE_WIFI_SOFTAP ? PARTICLE_MODE_SOFTAP_BIT : 0);
    int val = (bit | PARTICLE_MODE_BEACON_MAX);
    DBGOUT((F("Writing Particle configuration: offset=%d value=%02X"), FLASHOFF_PARTICLE_MODE, val));
    EEPROM.write(FLASHOFF_PARTICLE_MODE, val);

    setName(deviceName); // override device name

    SetNetwork((char*)""); // clear all networks
    #if DEFAULT_WIFI_INFO
    SetNetwork(DEFAULT_WIFI_INFO); // set default network
    #endif
  }
  #endif

  // Entry point on bootup for all modes of operation
  void setup(void)
  {
    DBGOUT((F("---------------------------------------")));

    byte instr[100];
    DBGOUT((F("Particle Photon:")));
    DBGOUT((F("  Version=%s"), System.version().c_str()));
    System.deviceID().toCharArray(deviceID, 100);
    DBGOUT((F("  DeviceID=%s"), deviceID));
    DBGOUT((F("  Memory=%d free bytes"), System.freeMemory()));
    Time.timeStr().getBytes(instr, 100);
    DBGOUT((F("  Time=%s"), instr));

    WiFi.on(); // make sure it's on
    SaveNetworkNames(); // save network names

    char name[MAXLEN_DEVICE_NAME+1];
    FlashGetName(name);

    int softap = EEPROM.read(FLASHOFF_PARTICLE_MODE) & PARTICLE_MODE_SOFTAP_BIT;
    DBGOUT((F("Starting Mode: %s"), (softap ? "SoftAP" : "Cloud")));

    if (!softap && !WiFi.hasCredentials())
    {
      softap = true;
      DBGOUT((F("No WiFi credentials: use SoftAP")));
    }

    if (softap || !SetupCloud())
    {
      if (!WiFi.listening()) WiFi.listen(); // put into SoftAP mode if not already

      DBGOUT((F("---------------------------------------")));
    }
  }

  bool setName(char *name)
  {
    // if use explicit prefix then don't use in naming
    if (!strncmp(name, PREFIX_DEVICE_NAME, PREFIX_LEN_DEVNAME))
    {
      name += PREFIX_LEN_DEVNAME;
      FlashSetName((char*)name);

      SetNameForSoftAP(name, (char*)"");
    }
    else // else add special prefix here
    {
      FlashSetName((char*)name);

      char devname[MAXLEN_DEVICE_NAME + PREFIX_LEN_DEVNAME + 1];
      strcpy(devname, PREFIX_DEVICE_NAME);
      strcat(devname, name);
  
      SetNameForSoftAP(devname, (char*)POSTFIX_DEVNAME);
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
      //DBGOUT((F("ReplyStr: \"%s\""), instr));
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

  bool loop()
  {
    if (rebootTime > 0) // waiting to reboot
    {
      if ((millis() - rebootTime) > 1000)
        RestartDevice(); // never returns
    }

    return false; // check physical controls
  }

};
WiFiNet wifinet;

#endif // defined(SPARK)
#endif // WIFI_COMM
//========================================================================================
