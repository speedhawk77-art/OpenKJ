#include "remotecontrolserver.h"

#include <QAbstractSocket>
#include <QFile>
#include <QHostAddress>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkInterface>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <utility>

namespace {
constexpr int kMaxRequestBytes = 8192;

// All CSS, markup and JavaScript are embedded in OpenKJ: NO web connection or CDN.
// OKJ_REMOTE_V5_GUEST_REQUESTS
// Offline guest songbook: no remote-control PIN and no external requests.
// OKJ_REMOTE_V6_LOCAL_QR - embedded assets, served by OpenKJ over local HTTP.
constexpr char kQrPage[] = 
R"OKJQRHTML0(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#151922"><title>OpenKJ – Singer Request QR</title>
<style>
*{box-sizing:border-box}body{background:#151922;color:#f8fafc;margin:0;padding:14px;font:17px system-ui,-apple-system,Segoe UI,sans-serif}
main{max-width:460px;margin:auto;text-align:center}h1{font-size:1.55rem;margin:18px 0 10px}p{color:#b8c2d1;line-height:1.45}
.card{background:#212938;border:1px solid #354156;border-radius:20px;padding:18px;margin:20px 0}
#code{padding:14px;background:#fff;border-radius:16px;display:inline-flex;align-items:center;justify-content:center;margin:10px auto 4px;max-width:100%}
#code svg{display:block;width:min(72vw,340px);height:auto;shape-rendering:crispEdges}
#url{overflow-wrap:anywhere;font-size:.84rem;color:#dfe9fb;background:#101827;border-radius:10px;padding:10px;margin-top:14px}
a{color:#ffc48d;display:inline-block;margin-top:12px;padding:12px;text-decoration:none;border:1px solid #975c30;border-radius:11px}
</style></head><body><main>
<h1>🎤 Request a Karaoke Song</h1>
<p>Ask singers to connect to the <strong>karaoke Wi-Fi</strong>, then scan this code with their phone camera.</p>
<section class="card"><div id="code"><span style="color:#111">Generating QR…</span></div>
<p id="url"></p><p style="font-size:.85rem">No internet needed. Requests go to the DJ for approval.</p></section>
<a href="/">← Back to DJ Remote</a>
<script src="/qr-library.js"></script>
<script>
'use strict';
(function(){
 const address = new URL('/request', window.location.href).href;
 document.getElementById('url').textContent = address;
 const target = document.getElementById('code');
 try {
   // The QR encoder is embedded into OpenKJ, NOT loaded from the web.
   const qr = new OpenKJQRCode(0, 0); // ECC M; auto-select QR version
   qr.addData(address);
   qr.make();
   const n = qr.getModuleCount(), quiet = 4, dim = n + quiet * 2;
   const ns = 'http://www.w3.org/2000/svg';
   const svg = document.createElementNS(ns, 'svg');
   svg.setAttribute('viewBox', '0 0 ' + dim + ' ' + dim);
   svg.setAttribute('role', 'img');
   svg.setAttribute('aria-label', 'QR code for singer requests');
   const bg = document.createElementNS(ns, 'rect');
   bg.setAttribute('width', dim); bg.setAttribute('height', dim); bg.setAttribute('fill', '#ffffff');
   svg.appendChild(bg);
   let bits = [];
   for (let y = 0; y < n; ++y)
     for (let x = 0; x < n; ++x)
       if (qr.isDark(y,x)) bits.push('M'+(x+quiet)+' '+(y+quiet)+'h1v1h-1z');
   const path = document.createElementNS(ns, 'path');
   path.setAttribute('d', bits.join(''));
   path.setAttribute('fill', '#000000');
   svg.appendChild(path);
   target.replaceChildren(svg);
 } catch (e) {
   target.textContent = 'QR could not be generated. Share the address below instead.';
   target.style.color = '#b00020';
 }
})();
</script></main></body></html>)OKJQRHTML0";

constexpr char kQrLibrary[] =
R"OKJQRLIB0(/* Offline QR encoder. QRCode for JavaScript, Copyright (c) 2009 Kazuhiko Arase (MIT).
 * Portions packaged by qrcode-terminal (Apache-2.0); see accompanying licenses. */
(function(root){'use strict';
const modules={
"./QRMode":function(module,exports,require){
module.exports = {
    MODE_NUMBER :       1 << 0,
    MODE_ALPHA_NUM :    1 << 1,
    MODE_8BIT_BYTE :    1 << 2,
    MODE_KANJI :        1 << 3
};

},
"./QRErrorCorrectLevel":function(module,exports,require){
module.exports = {
	L : 1,
	M : 0,
	Q : 3,
	H : 2
};


},
"./QRMaskPattern":function(module,exports,require){
module.exports = {
	PATTERN000 : 0,
	PATTERN001 : 1,
	PATTERN010 : 2,
	PATTERN011 : 3,
	PATTERN100 : 4,
	PATTERN101 : 5,
	PATTERN110 : 6,
	PATTERN111 : 7
};

},
"./QRMath":function(module,exports,require){
var QRMath = {

	glog : function(n) {
	
		if (n < 1) {
			throw new Error("glog(" + n + ")");
		}
		
		return QRMath.LOG_TABLE[n];
	},
	
	gexp : function(n) {
	
		while (n < 0) {
			n += 255;
		}
	
		while (n >= 256) {
			n -= 255;
		}
	
		return QRMath.EXP_TABLE[n];
	},
	
	EXP_TABLE : new Array(256),
	
	LOG_TABLE : new Array(256)

};
	
for (var i = 0; i < 8; i++) {
	QRMath.EXP_TABLE[i] = 1 << i;
}
for (var i = 8; i < 256; i++) {
	QRMath.EXP_TABLE[i] = QRMath.EXP_TABLE[i - 4]
		^ QRMath.EXP_TABLE[i - 5]
		^ QRMath.EXP_TABLE[i - 6]
		^ QRMath.EXP_TABLE[i - 8];
}
for (var i = 0; i < 255; i++) {
	QRMath.LOG_TABLE[QRMath.EXP_TABLE[i] ] = i;
}

module.exports = QRMath;

},
"./QR8bitByte":function(module,exports,require){
var QRMode = require('./QRMode');

function QR8bitByte(data) {
	this.mode = QRMode.MODE_8BIT_BYTE;
	this.data = data;
}

QR8bitByte.prototype = {

	getLength : function() {
		return this.data.length;
	},
	
	write : function(buffer) {
		for (var i = 0; i < this.data.length; i++) {
			// not JIS ...
			buffer.put(this.data.charCodeAt(i), 8);
		}
	}
};

module.exports = QR8bitByte;

},
"./QRBitBuffer":function(module,exports,require){
function QRBitBuffer() {
	this.buffer = [];
	this.length = 0;
}

QRBitBuffer.prototype = {

	get : function(index) {
		var bufIndex = Math.floor(index / 8);
		return ( (this.buffer[bufIndex] >>> (7 - index % 8) ) & 1) == 1;
	},
	
	put : function(num, length) {
		for (var i = 0; i < length; i++) {
			this.putBit( ( (num >>> (length - i - 1) ) & 1) == 1);
		}
	},
	
	getLengthInBits : function() {
		return this.length;
	},
	
	putBit : function(bit) {
	
		var bufIndex = Math.floor(this.length / 8);
		if (this.buffer.length <= bufIndex) {
			this.buffer.push(0);
		}
	
		if (bit) {
			this.buffer[bufIndex] |= (0x80 >>> (this.length % 8) );
		}
	
		this.length++;
	}
};

module.exports = QRBitBuffer;

},
"./QRPolynomial":function(module,exports,require){
var QRMath = require('./QRMath');

function QRPolynomial(num, shift) {
	if (num.length === undefined) {
		throw new Error(num.length + "/" + shift);
	}

	var offset = 0;

	while (offset < num.length && num[offset] === 0) {
		offset++;
	}

	this.num = new Array(num.length - offset + shift);
	for (var i = 0; i < num.length - offset; i++) {
		this.num[i] = num[i + offset];
	}
}

QRPolynomial.prototype = {

	get : function(index) {
		return this.num[index];
	},
	
	getLength : function() {
		return this.num.length;
	},
	
	multiply : function(e) {
	
		var num = new Array(this.getLength() + e.getLength() - 1);
	
		for (var i = 0; i < this.getLength(); i++) {
			for (var j = 0; j < e.getLength(); j++) {
				num[i + j] ^= QRMath.gexp(QRMath.glog(this.get(i) ) + QRMath.glog(e.get(j) ) );
			}
		}
	
		return new QRPolynomial(num, 0);
	},
	
	mod : function(e) {
	
		if (this.getLength() - e.getLength() < 0) {
			return this;
		}
	
		var ratio = QRMath.glog(this.get(0) ) - QRMath.glog(e.get(0) );
	
		var num = new Array(this.getLength() );
		
		for (var i = 0; i < this.getLength(); i++) {
)OKJQRLIB0"
R"OKJQRLIB1(			num[i] = this.get(i);
		}
		
		for (var x = 0; x < e.getLength(); x++) {
			num[x] ^= QRMath.gexp(QRMath.glog(e.get(x) ) + ratio);
		}
	
		// recursive call
		return new QRPolynomial(num, 0).mod(e);
	}
};

module.exports = QRPolynomial;

},
"./QRRSBlock":function(module,exports,require){
var QRErrorCorrectLevel = require('./QRErrorCorrectLevel');

function QRRSBlock(totalCount, dataCount) {
	this.totalCount = totalCount;
	this.dataCount  = dataCount;
}

QRRSBlock.RS_BLOCK_TABLE = [

	// L
	// M
	// Q
	// H

	// 1
	[1, 26, 19],
	[1, 26, 16],
	[1, 26, 13],
	[1, 26, 9],
	
	// 2
	[1, 44, 34],
	[1, 44, 28],
	[1, 44, 22],
	[1, 44, 16],

	// 3
	[1, 70, 55],
	[1, 70, 44],
	[2, 35, 17],
	[2, 35, 13],

	// 4		
	[1, 100, 80],
	[2, 50, 32],
	[2, 50, 24],
	[4, 25, 9],
	
	// 5
	[1, 134, 108],
	[2, 67, 43],
	[2, 33, 15, 2, 34, 16],
	[2, 33, 11, 2, 34, 12],
	
	// 6
	[2, 86, 68],
	[4, 43, 27],
	[4, 43, 19],
	[4, 43, 15],
	
	// 7		
	[2, 98, 78],
	[4, 49, 31],
	[2, 32, 14, 4, 33, 15],
	[4, 39, 13, 1, 40, 14],
	
	// 8
	[2, 121, 97],
	[2, 60, 38, 2, 61, 39],
	[4, 40, 18, 2, 41, 19],
	[4, 40, 14, 2, 41, 15],
	
	// 9
	[2, 146, 116],
	[3, 58, 36, 2, 59, 37],
	[4, 36, 16, 4, 37, 17],
	[4, 36, 12, 4, 37, 13],
	
	// 10		
	[2, 86, 68, 2, 87, 69],
	[4, 69, 43, 1, 70, 44],
	[6, 43, 19, 2, 44, 20],
	[6, 43, 15, 2, 44, 16],

	// 11
	[4, 101, 81],
	[1, 80, 50, 4, 81, 51],
	[4, 50, 22, 4, 51, 23],
	[3, 36, 12, 8, 37, 13],

	// 12
	[2, 116, 92, 2, 117, 93],
	[6, 58, 36, 2, 59, 37],
	[4, 46, 20, 6, 47, 21],
	[7, 42, 14, 4, 43, 15],

	// 13
	[4, 133, 107],
	[8, 59, 37, 1, 60, 38],
	[8, 44, 20, 4, 45, 21],
	[12, 33, 11, 4, 34, 12],

	// 14
	[3, 145, 115, 1, 146, 116],
	[4, 64, 40, 5, 65, 41],
	[11, 36, 16, 5, 37, 17],
	[11, 36, 12, 5, 37, 13],

	// 15
	[5, 109, 87, 1, 110, 88],
	[5, 65, 41, 5, 66, 42],
	[5, 54, 24, 7, 55, 25],
	[11, 36, 12],

	// 16
	[5, 122, 98, 1, 123, 99],
	[7, 73, 45, 3, 74, 46],
	[15, 43, 19, 2, 44, 20],
	[3, 45, 15, 13, 46, 16],

	// 17
	[1, 135, 107, 5, 136, 108],
	[10, 74, 46, 1, 75, 47],
	[1, 50, 22, 15, 51, 23],
	[2, 42, 14, 17, 43, 15],

	// 18
	[5, 150, 120, 1, 151, 121],
	[9, 69, 43, 4, 70, 44],
	[17, 50, 22, 1, 51, 23],
	[2, 42, 14, 19, 43, 15],

	// 19
	[3, 141, 113, 4, 142, 114],
	[3, 70, 44, 11, 71, 45],
	[17, 47, 21, 4, 48, 22],
	[9, 39, 13, 16, 40, 14],

	// 20
	[3, 135, 107, 5, 136, 108],
	[3, 67, 41, 13, 68, 42],
	[15, 54, 24, 5, 55, 25],
	[15, 43, 15, 10, 44, 16],

	// 21
	[4, 144, 116, 4, 145, 117],
	[17, 68, 42],
	[17, 50, 22, 6, 51, 23],
	[19, 46, 16, 6, 47, 17],

	// 22
	[2, 139, 111, 7, 140, 112],
	[17, 74, 46],
	[7, 54, 24, 16, 55, 25],
	[34, 37, 13],

	// 23
	[4, 151, 121, 5, 152, 122],
	[4, 75, 47, 14, 76, 48],
	[11, 54, 24, 14, 55, 25],
	[16, 45, 15, 14, 46, 16],

	// 24
	[6, 147, 117, 4, 148, 118],
	[6, 73, 45, 14, 74, 46],
	[11, 54, 24, 16, 55, 25],
	[30, 46, 16, 2, 47, 17],

	// 25
	[8, 132, 106, 4, 133, 107],
	[8, 75, 47, 13, 76, 48],
	[7, 54, 24, 22, 55, 25],
	[22, 45, 15, 13, 46, 16],

	// 26
	[10, 142, 114, 2, 143, 115],
	[19, 74, 46, 4, 75, 47],
	[28, 50, 22, 6, 51, 23],
	[33, 46, 16, 4, 47, 17],

	// 27
	[8, 152, 122, 4, 153, 123],
	[22, 73, 45, 3, 74, 46],
	[8, 53, 23, 26, 54, 24],
	[12, 45, 15, 28, 46, 16],

	// 28
	[3, 147, 117, 10, 148, 118],
	[3, 73, 45, 23, 74, 46],
	[4, 54, 24, 31, 55, 25],
	[11, 45, 15, 31, 46, 16],

	// 29
	[7, 146, 116, 7, 147, 117],
	[21, 73, 45, 7, 74, 46],
	[1, 53, 23, 37, 54, 24],
	[19, 45, 15, 26, 46, 16],

	// 30
	[5, 145, 115, 10, 146, 116],
	[19, 75, 47, 10, 76, 48],
	[15, 54, 24, 25, 55, 25],
	[23, 45, 15, 25, 46, 16],

	// 31
	[13, 145, 115, 3, 146, 116],
	[2, 74, 46, 29, 75, 47],
	[42, 54, 24, 1, 55, 25],
	[23, 45, 15, 28, 46, 16],

	// 32
	[17, 145, 115],
	[10, 74, 46, 23, 75, 47],
	[10, 54, 24, 35, 55, 25],
	[19, 45, 15, 35, 46, 16],

	// 33
	[17, 145, 115, 1, 146, 116])OKJQRLIB1"
R"OKJQRLIB2(,
	[14, 74, 46, 21, 75, 47],
	[29, 54, 24, 19, 55, 25],
	[11, 45, 15, 46, 46, 16],

	// 34
	[13, 145, 115, 6, 146, 116],
	[14, 74, 46, 23, 75, 47],
	[44, 54, 24, 7, 55, 25],
	[59, 46, 16, 1, 47, 17],

	// 35
	[12, 151, 121, 7, 152, 122],
	[12, 75, 47, 26, 76, 48],
	[39, 54, 24, 14, 55, 25],
	[22, 45, 15, 41, 46, 16],

	// 36
	[6, 151, 121, 14, 152, 122],
	[6, 75, 47, 34, 76, 48],
	[46, 54, 24, 10, 55, 25],
	[2, 45, 15, 64, 46, 16],

	// 37
	[17, 152, 122, 4, 153, 123],
	[29, 74, 46, 14, 75, 47],
	[49, 54, 24, 10, 55, 25],
	[24, 45, 15, 46, 46, 16],

	// 38
	[4, 152, 122, 18, 153, 123],
	[13, 74, 46, 32, 75, 47],
	[48, 54, 24, 14, 55, 25],
	[42, 45, 15, 32, 46, 16],

	// 39
	[20, 147, 117, 4, 148, 118],
	[40, 75, 47, 7, 76, 48],
	[43, 54, 24, 22, 55, 25],
	[10, 45, 15, 67, 46, 16],

	// 40
	[19, 148, 118, 6, 149, 119],
	[18, 75, 47, 31, 76, 48],
	[34, 54, 24, 34, 55, 25],
	[20, 45, 15, 61, 46, 16]
];

QRRSBlock.getRSBlocks = function(typeNumber, errorCorrectLevel) {
	
	var rsBlock = QRRSBlock.getRsBlockTable(typeNumber, errorCorrectLevel);
	
	if (rsBlock === undefined) {
		throw new Error("bad rs block @ typeNumber:" + typeNumber + "/errorCorrectLevel:" + errorCorrectLevel);
	}

	var length = rsBlock.length / 3;
	
	var list = [];
	
	for (var i = 0; i < length; i++) {

		var count = rsBlock[i * 3 + 0];
		var totalCount = rsBlock[i * 3 + 1];
		var dataCount  = rsBlock[i * 3 + 2];

		for (var j = 0; j < count; j++) {
			list.push(new QRRSBlock(totalCount, dataCount) );	
		}
	}
	
	return list;
};

QRRSBlock.getRsBlockTable = function(typeNumber, errorCorrectLevel) {

	switch(errorCorrectLevel) {
	case QRErrorCorrectLevel.L :
		return QRRSBlock.RS_BLOCK_TABLE[(typeNumber - 1) * 4 + 0];
	case QRErrorCorrectLevel.M :
		return QRRSBlock.RS_BLOCK_TABLE[(typeNumber - 1) * 4 + 1];
	case QRErrorCorrectLevel.Q :
		return QRRSBlock.RS_BLOCK_TABLE[(typeNumber - 1) * 4 + 2];
	case QRErrorCorrectLevel.H :
		return QRRSBlock.RS_BLOCK_TABLE[(typeNumber - 1) * 4 + 3];
	default :
		return undefined;
	}
};

module.exports = QRRSBlock;

},
"./QRUtil":function(module,exports,require){
var QRMode = require('./QRMode');
var QRPolynomial = require('./QRPolynomial');
var QRMath = require('./QRMath');
var QRMaskPattern = require('./QRMaskPattern');

var QRUtil = {

    PATTERN_POSITION_TABLE : [
        [],
        [6, 18],
        [6, 22],
        [6, 26],
        [6, 30],
        [6, 34],
        [6, 22, 38],
        [6, 24, 42],
        [6, 26, 46],
        [6, 28, 50],
        [6, 30, 54],        
        [6, 32, 58],
        [6, 34, 62],
        [6, 26, 46, 66],
        [6, 26, 48, 70],
        [6, 26, 50, 74],
        [6, 30, 54, 78],
        [6, 30, 56, 82],
        [6, 30, 58, 86],
        [6, 34, 62, 90],
        [6, 28, 50, 72, 94],
        [6, 26, 50, 74, 98],
        [6, 30, 54, 78, 102],
        [6, 28, 54, 80, 106],
        [6, 32, 58, 84, 110],
        [6, 30, 58, 86, 114],
        [6, 34, 62, 90, 118],
        [6, 26, 50, 74, 98, 122],
        [6, 30, 54, 78, 102, 126],
        [6, 26, 52, 78, 104, 130],
        [6, 30, 56, 82, 108, 134],
        [6, 34, 60, 86, 112, 138],
        [6, 30, 58, 86, 114, 142],
        [6, 34, 62, 90, 118, 146],
        [6, 30, 54, 78, 102, 126, 150],
        [6, 24, 50, 76, 102, 128, 154],
        [6, 28, 54, 80, 106, 132, 158],
        [6, 32, 58, 84, 110, 136, 162],
        [6, 26, 54, 82, 110, 138, 166],
        [6, 30, 58, 86, 114, 142, 170]
    ],

    G15 : (1 << 10) | (1 << 8) | (1 << 5) | (1 << 4) | (1 << 2) | (1 << 1) | (1 << 0),
    G18 : (1 << 12) | (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 5) | (1 << 2) | (1 << 0),
    G15_MASK : (1 << 14) | (1 << 12) | (1 << 10)    | (1 << 4) | (1 << 1),

    getBCHTypeInfo : function(data) {
        var d = data << 10;
        while (QRUtil.getBCHDigit(d) -)OKJQRLIB2"
R"OKJQRLIB3( QRUtil.getBCHDigit(QRUtil.G15) >= 0) {
            d ^= (QRUtil.G15 << (QRUtil.getBCHDigit(d) - QRUtil.getBCHDigit(QRUtil.G15) ) );    
        }
        return ( (data << 10) | d) ^ QRUtil.G15_MASK;
    },

    getBCHTypeNumber : function(data) {
        var d = data << 12;
        while (QRUtil.getBCHDigit(d) - QRUtil.getBCHDigit(QRUtil.G18) >= 0) {
            d ^= (QRUtil.G18 << (QRUtil.getBCHDigit(d) - QRUtil.getBCHDigit(QRUtil.G18) ) );    
        }
        return (data << 12) | d;
    },

    getBCHDigit : function(data) {

        var digit = 0;

        while (data !== 0) {
            digit++;
            data >>>= 1;
        }

        return digit;
    },

    getPatternPosition : function(typeNumber) {
        return QRUtil.PATTERN_POSITION_TABLE[typeNumber - 1];
    },

    getMask : function(maskPattern, i, j) {
        
        switch (maskPattern) {
            
        case QRMaskPattern.PATTERN000 : return (i + j) % 2 === 0;
        case QRMaskPattern.PATTERN001 : return i % 2 === 0;
        case QRMaskPattern.PATTERN010 : return j % 3 === 0;
        case QRMaskPattern.PATTERN011 : return (i + j) % 3 === 0;
        case QRMaskPattern.PATTERN100 : return (Math.floor(i / 2) + Math.floor(j / 3) ) % 2 === 0;
        case QRMaskPattern.PATTERN101 : return (i * j) % 2 + (i * j) % 3 === 0;
        case QRMaskPattern.PATTERN110 : return ( (i * j) % 2 + (i * j) % 3) % 2 === 0;
        case QRMaskPattern.PATTERN111 : return ( (i * j) % 3 + (i + j) % 2) % 2 === 0;

        default :
            throw new Error("bad maskPattern:" + maskPattern);
        }
    },

    getErrorCorrectPolynomial : function(errorCorrectLength) {

        var a = new QRPolynomial([1], 0);

        for (var i = 0; i < errorCorrectLength; i++) {
            a = a.multiply(new QRPolynomial([1, QRMath.gexp(i)], 0) );
        }

        return a;
    },

    getLengthInBits : function(mode, type) {

        if (1 <= type && type < 10) {

            // 1 - 9

            switch(mode) {
            case QRMode.MODE_NUMBER     : return 10;
            case QRMode.MODE_ALPHA_NUM  : return 9;
            case QRMode.MODE_8BIT_BYTE  : return 8;
            case QRMode.MODE_KANJI      : return 8;
            default :
                throw new Error("mode:" + mode);
            }

        } else if (type < 27) {

            // 10 - 26

            switch(mode) {
            case QRMode.MODE_NUMBER     : return 12;
            case QRMode.MODE_ALPHA_NUM  : return 11;
            case QRMode.MODE_8BIT_BYTE  : return 16;
            case QRMode.MODE_KANJI      : return 10;
            default :
                throw new Error("mode:" + mode);
            }

        } else if (type < 41) {

            // 27 - 40

            switch(mode) {
            case QRMode.MODE_NUMBER     : return 14;
            case QRMode.MODE_ALPHA_NUM  : return 13;
            case QRMode.MODE_8BIT_BYTE  : return 16;
            case QRMode.MODE_KANJI      : return 12;
            default :
                throw new Error("mode:" + mode);
            }

        } else {
            throw new Error("type:" + type);
        }
    },

    getLostPoint : function(qrCode) {
        
        var moduleCount = qrCode.getModuleCount();
        var lostPoint = 0;
        var row = 0; 
        var col = 0;

        
        // LEVEL1
        
        for (row = 0; row < moduleCount; row++) {

            for (col = 0; col < moduleCount; col++) {

                var sameCount = 0;
                var dark = qrCode.isDark(row, col);

                for (var r = -1; r <= 1; r++) {

                    if (row + r < 0 || moduleCount <= row + r) {
                        continue;
                    }

                    for (var c = -1; c <= 1; c++) {

                        if (col + c)OKJQRLIB3"
R"OKJQRLIB4( < 0 || moduleCount <= col + c) {
                            continue;
                        }

                        if (r === 0 && c === 0) {
                            continue;
                        }

                        if (dark === qrCode.isDark(row + r, col + c) ) {
                            sameCount++;
                        }
                    }
                }

                if (sameCount > 5) {
                    lostPoint += (3 + sameCount - 5);
                }
            }
        }

        // LEVEL2

        for (row = 0; row < moduleCount - 1; row++) {
            for (col = 0; col < moduleCount - 1; col++) {
                var count = 0;
                if (qrCode.isDark(row,     col    ) ) count++;
                if (qrCode.isDark(row + 1, col    ) ) count++;
                if (qrCode.isDark(row,     col + 1) ) count++;
                if (qrCode.isDark(row + 1, col + 1) ) count++;
                if (count === 0 || count === 4) {
                    lostPoint += 3;
                }
            }
        }

        // LEVEL3

        for (row = 0; row < moduleCount; row++) {
            for (col = 0; col < moduleCount - 6; col++) {
                if (qrCode.isDark(row, col) && 
                        !qrCode.isDark(row, col + 1) && 
                         qrCode.isDark(row, col + 2) && 
                         qrCode.isDark(row, col + 3) && 
                         qrCode.isDark(row, col + 4) && 
                        !qrCode.isDark(row, col + 5) && 
                         qrCode.isDark(row, col + 6) ) {
                    lostPoint += 40;
                }
            }
        }

        for (col = 0; col < moduleCount; col++) {
            for (row = 0; row < moduleCount - 6; row++) {
                if (qrCode.isDark(row, col) &&
                        !qrCode.isDark(row + 1, col) &&
                         qrCode.isDark(row + 2, col) &&
                         qrCode.isDark(row + 3, col) &&
                         qrCode.isDark(row + 4, col) &&
                        !qrCode.isDark(row + 5, col) &&
                         qrCode.isDark(row + 6, col) ) {
                    lostPoint += 40;
                }
            }
        }

        // LEVEL4
        
        var darkCount = 0;

        for (col = 0; col < moduleCount; col++) {
            for (row = 0; row < moduleCount; row++) {
                if (qrCode.isDark(row, col) ) {
                    darkCount++;
                }
            }
        }
        
        var ratio = Math.abs(100 * darkCount / moduleCount / moduleCount - 50) / 5;
        lostPoint += ratio * 10;

        return lostPoint;       
    }

};

module.exports = QRUtil;

},
"./index":function(module,exports,require){
//---------------------------------------------------------------------
// QRCode for JavaScript
//
// Copyright (c) 2009 Kazuhiko Arase
//
// URL: http://www.d-project.com/
//
// Licensed under the MIT license:
//   http://www.opensource.org/licenses/mit-license.php
//
// The word "QR Code" is registered trademark of 
// DENSO WAVE INCORPORATED
//   http://www.denso-wave.com/qrcode/faqpatent-e.html
//
//---------------------------------------------------------------------
// Modified to work in node for this project (and some refactoring)
//---------------------------------------------------------------------

var QR8bitByte = require('./QR8bitByte');
var QRUtil = require('./QRUtil');
var QRPolynomial = require('./QRPolynomial');
var QRRSBlock = require('./QRRSBlock');
var QRBitBuffer = require('./QRBitBuffer');

function QRCode(typeNumber, errorCorrectLevel) {
	this.typeNumber = typeNumber;
	this.errorCorrectLevel = errorCorrectLevel;
	this.modules = null;
	this.moduleCount = 0;
	this.dataCache = null;
	this.dataLis)OKJQRLIB4"
R"OKJQRLIB5(t = [];
}

QRCode.prototype = {
	
	addData : function(data) {
		var newData = new QR8bitByte(data);
		this.dataList.push(newData);
		this.dataCache = null;
	},
	
	isDark : function(row, col) {
		if (row < 0 || this.moduleCount <= row || col < 0 || this.moduleCount <= col) {
			throw new Error(row + "," + col);
		}
		return this.modules[row][col];
	},

	getModuleCount : function() {
		return this.moduleCount;
	},
	
	make : function() {
		// Calculate automatically typeNumber if provided is < 1
		if (this.typeNumber < 1 ){
			var typeNumber = 1;
			for (typeNumber = 1; typeNumber < 40; typeNumber++) {
				var rsBlocks = QRRSBlock.getRSBlocks(typeNumber, this.errorCorrectLevel);

				var buffer = new QRBitBuffer();
				var totalDataCount = 0;
				for (var i = 0; i < rsBlocks.length; i++) {
					totalDataCount += rsBlocks[i].dataCount;
				}

				for (var x = 0; x < this.dataList.length; x++) {
					var data = this.dataList[x];
					buffer.put(data.mode, 4);
					buffer.put(data.getLength(), QRUtil.getLengthInBits(data.mode, typeNumber) );
					data.write(buffer);
				}
				if (buffer.getLengthInBits() <= totalDataCount * 8)
					break;
			}
			this.typeNumber = typeNumber;
		}
		this.makeImpl(false, this.getBestMaskPattern() );
	},
	
	makeImpl : function(test, maskPattern) {
		
		this.moduleCount = this.typeNumber * 4 + 17;
		this.modules = new Array(this.moduleCount);
		
		for (var row = 0; row < this.moduleCount; row++) {
			
			this.modules[row] = new Array(this.moduleCount);
			
			for (var col = 0; col < this.moduleCount; col++) {
				this.modules[row][col] = null;//(col + row) % 3;
			}
		}
	
		this.setupPositionProbePattern(0, 0);
		this.setupPositionProbePattern(this.moduleCount - 7, 0);
		this.setupPositionProbePattern(0, this.moduleCount - 7);
		this.setupPositionAdjustPattern();
		this.setupTimingPattern();
		this.setupTypeInfo(test, maskPattern);
		
		if (this.typeNumber >= 7) {
			this.setupTypeNumber(test);
		}
	
		if (this.dataCache === null) {
			this.dataCache = QRCode.createData(this.typeNumber, this.errorCorrectLevel, this.dataList);
		}
	
		this.mapData(this.dataCache, maskPattern);
	},

	setupPositionProbePattern : function(row, col)  {
		
		for (var r = -1; r <= 7; r++) {
			
			if (row + r <= -1 || this.moduleCount <= row + r) continue;
			
			for (var c = -1; c <= 7; c++) {
				
				if (col + c <= -1 || this.moduleCount <= col + c) continue;
				
				if ( (0 <= r && r <= 6 && (c === 0 || c === 6) ) || 
                     (0 <= c && c <= 6 && (r === 0 || r === 6) ) || 
                     (2 <= r && r <= 4 && 2 <= c && c <= 4) ) {
					this.modules[row + r][col + c] = true;
				} else {
					this.modules[row + r][col + c] = false;
				}
			}		
		}		
	},
	
	getBestMaskPattern : function() {
	
		var minLostPoint = 0;
		var pattern = 0;
	
		for (var i = 0; i < 8; i++) {
			
			this.makeImpl(true, i);
	
			var lostPoint = QRUtil.getLostPoint(this);
	
			if (i === 0 || minLostPoint >  lostPoint) {
				minLostPoint = lostPoint;
				pattern = i;
			}
		}
	
		return pattern;
	},
	
	createMovieClip : function(target_mc, instance_name, depth) {
	
		var qr_mc = target_mc.createEmptyMovieClip(instance_name, depth);
		var cs = 1;
	
		this.make();

		for (var row = 0; row < this.modules.length; row++) {
			
			var y = row * cs;
			
			for (var col = 0; col < this.modules[row].length; col++) {
	
				var x = col * cs;
				var dark = this.modules[row][col];
			
				if (dark) {
					qr_mc.beginFill(0, 100);
					qr_mc.moveTo(x, y);
					qr_mc.lineTo(x + cs, y);
					qr_mc.lineTo(x + cs, y + cs);
					qr_mc.lineTo(x, y + cs);
					qr_mc.endFill();
				}
			}
		}
		
		return qr_mc;
	},

	setupTimingPattern : function() {
		
		for (var r = 8; r < this.moduleCount - 8; r++) {
			if (this.modules[r][6] !== null) {
				continue;
		)OKJQRLIB5"
R"OKJQRLIB6(	}
			this.modules[r][6] = (r % 2 === 0);
		}
	
		for (var c = 8; c < this.moduleCount - 8; c++) {
			if (this.modules[6][c] !== null) {
				continue;
			}
			this.modules[6][c] = (c % 2 === 0);
		}
	},
	
	setupPositionAdjustPattern : function() {
	
		var pos = QRUtil.getPatternPosition(this.typeNumber);
		
		for (var i = 0; i < pos.length; i++) {
		
			for (var j = 0; j < pos.length; j++) {
			
				var row = pos[i];
				var col = pos[j];
				
				if (this.modules[row][col] !== null) {
					continue;
				}
				
				for (var r = -2; r <= 2; r++) {
				
					for (var c = -2; c <= 2; c++) {
					
						if (Math.abs(r) === 2 || 
                            Math.abs(c) === 2 ||
                            (r === 0 && c === 0) ) {
							this.modules[row + r][col + c] = true;
						} else {
							this.modules[row + r][col + c] = false;
						}
					}
				}
			}
		}
	},
	
	setupTypeNumber : function(test) {
	
		var bits = QRUtil.getBCHTypeNumber(this.typeNumber);
        var mod;
	
		for (var i = 0; i < 18; i++) {
			mod = (!test && ( (bits >> i) & 1) === 1);
			this.modules[Math.floor(i / 3)][i % 3 + this.moduleCount - 8 - 3] = mod;
		}
	
		for (var x = 0; x < 18; x++) {
			mod = (!test && ( (bits >> x) & 1) === 1);
			this.modules[x % 3 + this.moduleCount - 8 - 3][Math.floor(x / 3)] = mod;
		}
	},
	
	setupTypeInfo : function(test, maskPattern) {
	
		var data = (this.errorCorrectLevel << 3) | maskPattern;
		var bits = QRUtil.getBCHTypeInfo(data);
        var mod;
	
		// vertical		
		for (var v = 0; v < 15; v++) {
	
			mod = (!test && ( (bits >> v) & 1) === 1);
	
			if (v < 6) {
				this.modules[v][8] = mod;
			} else if (v < 8) {
				this.modules[v + 1][8] = mod;
			} else {
				this.modules[this.moduleCount - 15 + v][8] = mod;
			}
		}
	
		// horizontal
		for (var h = 0; h < 15; h++) {
	
			mod = (!test && ( (bits >> h) & 1) === 1);
			
			if (h < 8) {
				this.modules[8][this.moduleCount - h - 1] = mod;
			} else if (h < 9) {
				this.modules[8][15 - h - 1 + 1] = mod;
			} else {
				this.modules[8][15 - h - 1] = mod;
			}
		}
	
		// fixed module
		this.modules[this.moduleCount - 8][8] = (!test);
	
	},
	
	mapData : function(data, maskPattern) {
		
		var inc = -1;
		var row = this.moduleCount - 1;
		var bitIndex = 7;
		var byteIndex = 0;
		
		for (var col = this.moduleCount - 1; col > 0; col -= 2) {
	
			if (col === 6) col--;
	
			while (true) {
	
				for (var c = 0; c < 2; c++) {
					
					if (this.modules[row][col - c] === null) {
						
						var dark = false;
	
						if (byteIndex < data.length) {
							dark = ( ( (data[byteIndex] >>> bitIndex) & 1) === 1);
						}
	
						var mask = QRUtil.getMask(maskPattern, row, col - c);
	
						if (mask) {
							dark = !dark;
						}
						
						this.modules[row][col - c] = dark;
						bitIndex--;
	
						if (bitIndex === -1) {
							byteIndex++;
							bitIndex = 7;
						}
					}
				}
								
				row += inc;
	
				if (row < 0 || this.moduleCount <= row) {
					row -= inc;
					inc = -inc;
					break;
				}
			}
		}
		
	}

};

QRCode.PAD0 = 0xEC;
QRCode.PAD1 = 0x11;

QRCode.createData = function(typeNumber, errorCorrectLevel, dataList) {
	
	var rsBlocks = QRRSBlock.getRSBlocks(typeNumber, errorCorrectLevel);
	
	var buffer = new QRBitBuffer();
	
	for (var i = 0; i < dataList.length; i++) {
		var data = dataList[i];
		buffer.put(data.mode, 4);
		buffer.put(data.getLength(), QRUtil.getLengthInBits(data.mode, typeNumber) );
		data.write(buffer);
	}

	// calc num max data.
	var totalDataCount = 0;
	for (var x = 0; x < rsBlocks.length; x++) {
		totalDataCount += rsBlocks[x].dataCount;
	}

	if (buffer.getLengthInBits() > totalDataCount * 8) {
		throw new Error("code length overflow. (" + 
            buffer.getLengthInBits() + 
            ">" +  
            totalDataCount * 8 + 
         )OKJQRLIB6"
R"OKJQRLIB7(   ")");
	}

	// end code
	if (buffer.getLengthInBits() + 4 <= totalDataCount * 8) {
		buffer.put(0, 4);
	}

	// padding
	while (buffer.getLengthInBits() % 8 !== 0) {
		buffer.putBit(false);
	}

	// padding
	while (true) {
		
		if (buffer.getLengthInBits() >= totalDataCount * 8) {
			break;
		}
		buffer.put(QRCode.PAD0, 8);
		
		if (buffer.getLengthInBits() >= totalDataCount * 8) {
			break;
		}
		buffer.put(QRCode.PAD1, 8);
	}

	return QRCode.createBytes(buffer, rsBlocks);
};

QRCode.createBytes = function(buffer, rsBlocks) {

	var offset = 0;
	
	var maxDcCount = 0;
	var maxEcCount = 0;
	
	var dcdata = new Array(rsBlocks.length);
	var ecdata = new Array(rsBlocks.length);
	
	for (var r = 0; r < rsBlocks.length; r++) {

		var dcCount = rsBlocks[r].dataCount;
		var ecCount = rsBlocks[r].totalCount - dcCount;

		maxDcCount = Math.max(maxDcCount, dcCount);
		maxEcCount = Math.max(maxEcCount, ecCount);
		
		dcdata[r] = new Array(dcCount);
		
		for (var i = 0; i < dcdata[r].length; i++) {
			dcdata[r][i] = 0xff & buffer.buffer[i + offset];
		}
		offset += dcCount;
		
		var rsPoly = QRUtil.getErrorCorrectPolynomial(ecCount);
		var rawPoly = new QRPolynomial(dcdata[r], rsPoly.getLength() - 1);

		var modPoly = rawPoly.mod(rsPoly);
		ecdata[r] = new Array(rsPoly.getLength() - 1);
		for (var x = 0; x < ecdata[r].length; x++) {
            var modIndex = x + modPoly.getLength() - ecdata[r].length;
			ecdata[r][x] = (modIndex >= 0)? modPoly.get(modIndex) : 0;
		}

	}
	
	var totalCodeCount = 0;
	for (var y = 0; y < rsBlocks.length; y++) {
		totalCodeCount += rsBlocks[y].totalCount;
	}

	var data = new Array(totalCodeCount);
	var index = 0;

	for (var z = 0; z < maxDcCount; z++) {
		for (var s = 0; s < rsBlocks.length; s++) {
			if (z < dcdata[s].length) {
				data[index++] = dcdata[s][z];
			}
		}
	}

	for (var xx = 0; xx < maxEcCount; xx++) {
		for (var t = 0; t < rsBlocks.length; t++) {
			if (xx < ecdata[t].length) {
				data[index++] = ecdata[t][xx];
			}
		}
	}

	return data;

};

module.exports = QRCode;

}
};
const cache={};
function require(name){if(!Object.prototype.hasOwnProperty.call(modules,name))throw new Error('No QR module: '+name);
if(cache[name])return cache[name].exports;let m={exports:{}};cache[name]=m;modules[name](m,m.exports,require);return m.exports;}
root.OpenKJQRCode=require('./index');
})(typeof window!=='undefined'?window:globalThis);
)OKJQRLIB7";

constexpr char kGuestRequestPage[] = R"OKJGUESTHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover"><meta name="theme-color" content="#141c29"><title>OpenKJ Song Requests</title>
<style>
*{box-sizing:border-box}body{background:#141c29;color:#f6f7fa;margin:0;padding:16px 12px 50px;font:17px system-ui,Arial,sans-serif}main{max-width:620px;margin:auto}header{text-align:center;padding:15px 0}h1{font-size:1.65rem;margin:0}p{color:#b9c6da;line-height:1.45}.box{background:#222e41;border:1px solid #47566d;border-radius:16px;padding:18px;margin:16px 0}label{display:block;margin-bottom:7px;font-weight:650}input{width:100%;padding:14px 13px;font-size:1.05rem;background:#0f1929;color:white;border:1px solid #8295ae;border-radius:12px}button{min-height:48px;background:#ec7d26;border:1px solid #ffa868;color:#16191f;border-radius:12px;font:700 1rem system-ui;cursor:pointer;padding:10px 14px}button:disabled{opacity:.42}.song{border:1px solid #47566d;border-radius:11px;padding:13px;margin-top:10px;display:flex;justify-content:space-between;gap:10px;align-items:center}.song>div{min-width:0}.title{font-weight:700;overflow-wrap:anywhere}.artist{font-size:.9rem;color:#afc0d4;margin-top:4px;overflow-wrap:anywhere}.song button{min-width:104px}.muted{color:#b9c6da;font-size:.9rem}.success{color:#93e6b1}#feedback{min-height:20px;margin:12px 0;font-weight:650}
</style></head><body><main>
<header><h1>🎤 Request a Karaoke Song</h1><p>Search the songs on the DJ's laptop. No internet needed.</p></header>
<div class="box"><label for="singer">Your singer name</label><input id="singer" maxlength="70" placeholder="Name you want the DJ to call" autocomplete="name"><p class="muted">Requests go to the DJ for approval; they won't interrupt a performance.</p></div>
<div class="box"><label for="search">Find a song</label><input id="search" type="search" placeholder="Artist or song title" autocomplete="off"><p id="count" class="muted">Enter at least two characters to search.</p><div id="results"></div></div>
<div id="feedback" role="status" aria-live="polite"></div>
</main><script>
'use strict';
const $=id=>document.getElementById(id);let timer=null,rev=0;
$('singer').value=localStorage.getItem('openkj_guest_name')||'';
$('singer').addEventListener('input',()=>localStorage.setItem('openkj_guest_name',$('singer').value.slice(0,70)));
function elem(tag,cls,txt){const n=document.createElement(tag);if(cls)n.className=cls;if(txt!==undefined)n.textContent=txt;return n;}
async function search(){const term=$('search').value.trim(),thisRev=++rev,root=$('results');root.replaceChildren();if(term.length<2){$('count').textContent='Enter at least two characters to search.';return;}$('count').textContent='Searching local songbook…';try{const r=await fetch('/public/songs?q='+encodeURIComponent(term),{cache:'no-store'});if(!r.ok)throw Error('Search unavailable');const data=await r.json();if(thisRev!==rev)return;const songs=Array.isArray(data.results)?data.results:[];$('count').textContent=songs.length+' song(s)'+(data.more?' — showing first 40; refine your search':'');for(const song of songs){const row=elem('div','song'),info=elem('div');info.append(elem('div','title',song.title||'Unknown title'),elem('div','artist',(song.artist||'Unknown artist')+(song.disc_id?' • '+song.disc_id:'')));const btn=elem('button','','Request');btn.dataset.songId=String(song.id);btn.dataset.title=song.title||'';row.append(info,btn);root.append(row);}if(!songs.length)root.append(elem('p','muted','No matching songs. Try fewer words.'));}catch(e){if(thisRev===rev)$('count').textContent='Search unavailable. Check Wi-Fi connection.';}}
$('search').addEventListener('input',()=>{clearTimeout(timer);timer=setTimeout(search,300)});
document.addEventListener('click',async event=>{const b=event.target.closest('button[data-song-id]');if(!b)return;const name=$('singer').value.trim();if(!name){$('feedback').textContent='Enter your singer name first.';$('singer').focus();return;}b.disabled=true;$('feedback').textContent='Sending request…';try{const response=await fetch('/public/request',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({singer_name:name,song_id:Number(b.dataset.songId)})});const result=await response.json();$('feedback').textContent=response.ok?(result.message||'Request sent to DJ for approval'):(result.error||'Request not accepted');$('feedback').className=response.ok?'success':'';}catch(e){$('feedback').textContent='Could not reach OpenKJ. Check Wi-Fi connection.';}finally{b.disabled=false;}});
</script></body></html>)OKJGUESTHTML";

constexpr char kControlPage[] =
R"OPENKJHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#151922">
<title>OpenKJ - Offline Remote</title>
<style>
*{box-sizing:border-box}body{background:#151922;color:#f8fafc;font:16px system-ui,-apple-system,Segoe UI,sans-serif;margin:0;padding:14px 14px 40px}
main{max-width:500px;margin:auto}header{text-align:center;margin:12px 0 16px}h1{font-size:1.65rem;margin:0}p{color:#b8c2d1;margin:6px 0}
section{background:#212938;border:1px solid #354156;border-radius:18px;padding:18px;margin:14px 0}
h2{font-size:1.15rem;margin:0 0 10px;display:flex;justify-content:space-between;gap:10px}
small{font-size:.78rem;color:#9ca9bc}b{font-weight:700}#connection{font-size:.85rem;text-align:center;min-height:1.2em}
.badge{font-size:.75rem;color:#b5efbe;background:#184b38;padding:4px 9px;border-radius:99px;white-space:nowrap}
.track{min-height:60px;line-height:1.4}.name{font-weight:750;font-size:1.15rem;overflow-wrap:anywhere}.sub{font-size:.94rem;color:#bcc8d9;overflow-wrap:anywhere}
.progress{height:8px;background:#111822;border-radius:99px;overflow:hidden;margin:12px 0 6px}.progress>div{height:100%;width:0%;background:#ff8d32;transition:width .2s}
.times{display:flex;justify-content:space-between;color:#acbad0;font-variant-numeric:tabular-nums;font-size:.82rem;margin-bottom:12px}
.controls{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:10px}
button{touch-action:manipulation;background:#34445d;color:white;border:1px solid #596b83;border-radius:13px;font:700 1rem system-ui;min-height:60px;padding:10px 6px;cursor:pointer}
button:active{transform:scale(.985)}button:disabled{opacity:.5}button.primary{background:#eb7725;border-color:#ffa052;color:#121212}
button.full{grid-column:1 / -1}button.stop{background:#5c2931;border-color:#a85560}
button.small{min-height:42px;font-size:.88rem;padding:6px 10px}button.select{background:#235540;border-color:#4b9677}
button.secondary{background:#293a52}button:focus-visible{outline:3px solid #ffad62}
.list{display:grid;gap:10px;margin:12px 0}.entry{border:1px solid #4c5d72;border-radius:12px;padding:10px 12px;background:#172131}
.entry.current{border-color:#ff8d32}.entry.selected{background:#213e35;border-color:#4b9677}
.entrytop{display:flex;gap:12px;justify-content:space-between;align-items:flex-start}.entrytop>*{min-width:0}
.entryname{font-weight:700;overflow-wrap:anywhere}.entrydesc{font-size:.87rem;color:#b9c6d9;overflow-wrap:anywhere;margin-top:4px}
.entryactions{display:flex;gap:6px;flex-shrink:0}.entryactions button{min-width:64px}
.tag{color:#ffad62;font-size:.76rem}.queueplayed{opacity:.6}
input[type=search]{letter-spacing:normal;text-align:left;padding:0 12px;font-size:1rem}
#songResults .entryactions button{min-width:70px}
.remote-singer-manage{display:flex;align-items:center;justify-content:space-between;gap:10px;flex-wrap:wrap;margin:12px 0}
.remote-singer-manage .name{flex:1;min-width:130px}
.remote-singer-manage button{min-height:40px;font-size:.85rem}
.rotation-order{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:9px}
.rotation-order button{min-height:42px;font-size:.88rem}
.add-singer{display:grid;grid-template-columns:1fr auto;gap:9px;align-items:center;margin:10px 0 14px}
.add-singer input{min-width:0;letter-spacing:normal;text-align:left;font-size:1rem;padding:0 12px}
.add-singer button{min-width:110px;padding:10px}
label{display:block;margin:0 0 8px}input{width:100%;min-height:52px;border-radius:10px;background:#111822;color:white;border:1px solid #8492a8;text-align:center;font:22px system-ui;letter-spacing:.22em}
#youtubeHost input{letter-spacing:normal;text-align:left;font-size:1rem;padding:10px 12px;margin-bottom:12px}
#youtubeHost[hidden]{display:none}
#youtubeStatus,#youtubeFeedback{white-space:pre-wrap;overflow-wrap:anywhere}
#pinBlock{margin-bottom:10px}#message{font-size:.88rem;min-height:1.25em;text-align:center;color:#f9ce85}
</style></head><body><main>
<header><h1>🎤 OpenKJ Remote</h1><p>Works on your local Wi-Fi without Internet.</p></header>
<section id="pinBlock"><label for="pin">Session PIN (shown on your laptop)</label>
<input id="pin" type="text" inputmode="numeric" autocomplete="off" maxlength="6" placeholder="6-digit PIN">
<p style="text-align:center;font-size:.84rem">PIN changes each time OpenKJ starts.</p></section>
<div id="connection">Not connected</div>
<section style="padding:12px 18px;margin:12px 0">
<a href="/qr" style="display:block;text-decoration:none;background:#ec7d26;color:#181818;font-weight:750;text-align:center;padding:14px;border-radius:12px">📱 Show Singer Request QR Code</a>
<p style="font-size:.82rem;text-align:center;margin:8px 0 0">Open this on your phone and show it to singers.</p>
</section>
<section>
<h2>Karaoke <span id="kState" class="badge">Unknown</span></h2>
<div class="track"><div id="kTitle" class="name">—</div><div id="kSinger" class="sub">No singer playing</div></div>
<div class="progress"><div id="kProgress"></div></div><div class="times"><span id="kElapsed">0:00</span><span id="kRemaining">Remaining 0:00</span></div>
<div class="controls">
<button class="primary" data-cmd="toggle">▶ / ❚❚ Play / Pause</button>
<button data-cmd="next">⏭ Next Singer</button>
<button data-cmd="volume_down">🔉 Vol −</button>
<button data-cmd="volume_up">🔊 Vol +</button>
<button class="stop full" data-cmd="stop">■ Stop Karaoke</button>
</div><p id="kVol" style="text-align:center;font-size:.82rem">Volume —</p>
</section>
<section>
<h2>Singer Rotation <span id="singerCount" class="badge">0 singers</span></h2>
<label for="singerName">Add a singer to the rotation</label>
<div class="add-singer"><input id="singerName" type="text" maxlength="70" autocomplete="off" placeholder="Singer name"><button id="addSingerBtn" class="primary">+ Add Singer</button></div>
<p>Select a singer to inspect their queue. Playing a singer is a separate action.</p>
<div class="list" id="singerList"><p>Loading rotation…</p></div>
<div class="remote-singer-manage">
<div id="selectedSinger" class="name">Select a singer</div>
<button id="removeSingerBtn" class="small stop" disabled>Remove Singer</button>
</div>
<p class="sub">Songs already queued for this singer:</p>
<div class="list" id="queueList"><p>Select a singer first.</p></div>
<h2>Find &amp; Add Songs</h2>
<label for="songQuery">Search OpenKJ's offline karaoke catalog</label>
<input id="songQuery" type="search" placeholder="Artist, title, or disc ID" autocomplete="off">
<p id="searchInfo" class="sub">Select a singer, then enter at least two characters.</p>
<div class="list" id="songResults"></div>
</section>
<section id="youtubeHost" hidden>
<h2>YouTube Downloads <span class="badge">Host only</span></h2>
<p class="sub">The laptop downloads and checks the video. Internet is required on the laptop. Uses its saved helper programs and save folder.</p>
<label for="youtubeUrl">YouTube video link</label>
<input id="youtubeUrl" type="url" maxlength="2048" placeholder="https://www.youtube.com/watch?v=…" autocomplete="off">
<div class="controls"><button id="youtubeStart" class="primary">Download video</button><button id="youtubeCancel" disabled>Cancel download</button></div>
<p id="youtubeStatus" role="status" aria-live="polite">Connecting…</p>
<label for="youtubeArtist">Artist or karaoke channel</label>
<input id="youtubeArtist" type="text" maxlength="256" disabled>
<label for="youtubeTitle">Title</label>
<input id="youtubeTitle" type="text" maxlength="512" disabled>
<button id="youtubeImport" class="primary" disabled>Add to library</button>
<p id="youtubeFeedback" role="status" aria-live="polite"></p>
<p class="sub">After adding, select a singer and use Find &amp; Add Songs above to queue it. Downloads never start playback or change the current singer.</p>
</section>
<section id="guestPendingBox">
<h2>Singer Song Requests <span id="guestReqCount" class="badge">0 pending</span></h2>
<p>Approve a request to add it to that singer's queue. The singer can't control playback.</p>
<div class="list" id="guestReqList"><p class="sub">Waiting for requests…</p></div>
<p class="sub">Guest link: <a href="/request" style="color:#ffac6a">Open singer request page</a></p>
</section>
<section>
<h2>Break Music <span id="bState" class="badge">Unknown</span></h2>
<div class="track"><div id="bTitle" class="name">—</div><div id="bArtist" class="sub">No break music playing</div></div>
<div class="progress"><div id="bProgress"></div></div><div class="times"><span id="bElapsed">0:00</span><span id="bRemaining">Remaining 0:00</span></div>
<div class="controls">
<button data-cmd="break_previous">⏮ Previous</button>
<button data-cmd="break_next">⏭ Next</button>
<button class="primary full" data-cmd="break_toggle">▶ / ❚❚ Play / Pause</button>
<button data-cmd="break_volume_down">🔉 Vol −</button>
<button data-cmd="break_volume_up">🔊 Vol +</button>
<button data-cmd="break_fade_out">↘ Fade Out</button>
<button data-cmd="break_fade_in">↗ Fade Back In</button>
<button class="stop full" data-cmd="break_stop">■ Stop Break Music</button>
</div><p id="bVol" style="text-align:center;font-size:.82rem">Volume —</p>
</section>
<div id="message" role="status" aria-live="polite"></div>
</main><script>
'use strict';
const get=id=>document.getElementById(id);
const pinBox=get('pin'), msg=get('message');
pinBox.value=sessionStorage.getItem('okj_remote_pin')||'';
pinBox.addEventListener('input',()=>{pinBox.value=pinBox.value.replace(/\D/g,'').slice(0,6);sessionStorage.setItem('okj_remote_pin',pinBox.value);updateStatus();});
const pad=n=>String(n).padStart(2,'0');
function clock(ms){const seconds=Math.max(0,Math.floor((Number(ms)||0)/1000));return Math.floor(seconds/60)+':'+pad(seconds%60)}
function draw(prefix, data) {
  get(prefix+'State').textContent=data.state||'Unknown';
  get(prefix+'Title').textContent=data.title||'—';
  get(prefix+(prefix==='k'?'Singer':'Artist')).textContent=(prefix==='k'?data.singer:data.artist)||'No '+(prefix==='k'?'singer':'break music')+' playing';
  const elapsed=Math.max(0,Number(data.elapsed_ms)||0), duration=Math.max(0,Number(data.duration_ms)||0);
  get(prefix+'Elapsed').textContent=clock(elapsed);
  get(prefix+'Remaining').textContent='Remaining '+clock(Math.max(0,duration-elapsed));
  get(prefix+'Progress').style.width=(duration?Math.min(100,Math.round(elapsed/duration*100)):0)+'%';
  get(prefix+'Vol').textContent='Volume '+(data.volume??'—')+'%';
}
let working=false;
let selectedSingerId=-1, searchTimer=null, searchRevision=0, searchBusy=false;
let lastSingerList='',lastQueueList='';
// OKJ_REMOTE_V5_GUEST_REQUESTS
let lastPendingJSON = '';
function drawGuestRequests(data){
 const items=Array.isArray(data.guest_requests)?data.guest_requests:[];
 get('guestReqCount').textContent=items.length+' pending';
 const encoded=JSON.stringify(items);if(encoded===lastPendingJSON)return;
 lastPendingJSON=encoded;
 const list=get('guestReqList');clear(list);
 if(!items.length){list.append(el('p','sub','No pending singer requests.'));return;}
 for(const item of items){
  const row=el('div','entry'),left=el('div'),actions=el('div','entryactions');
)OPENKJHTML"
R"OPENKJHTML(  left.append(el('div','entryname',item.singer||'Unknown singer'));
  left.append(el('div','entrydesc',(item.title||'Unknown title')+' — '+(item.artist||'Unknown artist')));
  const approve=el('button','small primary','Approve');approve.dataset.guestApprove=String(item.id);
  const reject=el('button','small stop','Decline');reject.dataset.guestReject=String(item.id);
  actions.append(approve,reject);row.append(left,actions);list.append(row);
 }
}

function el(tag,cls,content){const n=document.createElement(tag);if(cls)n.className=cls;if(content!==undefined)n.textContent=content;return n;}
function clear(node){while(node.firstChild)node.removeChild(node.firstChild);}
function drawRotation(data){
 const rotation=Array.isArray(data.rotation)?data.rotation:[];
 const newId=Number(data.selected_singer_id)||-1;
 const selectionChanged=newId!==selectedSingerId;
 selectedSingerId=newId;
 get('singerCount').textContent=rotation.length+' singers';
 // OKJ_REMOTE_V9_NOW_SINGING: get actual playback information, not next unsung song.
 const karaoke=data.karaoke||{};
 const karaokeActive=(karaoke.state==='Playing'||karaoke.state==='Paused')&&Boolean(karaoke.title);
 const singersHash=JSON.stringify([rotation,selectedSingerId,karaokeActive?karaoke.title:'',karaokeActive?karaoke.artist:'',karaokeActive?karaoke.singer:'']);
 if(singersHash!==lastSingerList){
  lastSingerList=singersHash;
  const list=get('singerList');clear(list);
  if(!rotation.length)list.append(el('p','sub','No singers yet. Add one above.'));
  rotation.forEach((s,i)=>{
   const row=el('div','entry'+(s.current?' current':'')+(s.id===selectedSingerId?' selected':''));
   const top=el('div','entrytop'), left=el('div'),acts=el('div','entryactions');
   left.append(el('div','entryname',(i+1)+'. '+s.name));
   const isPerforming=Boolean(s.current&&karaokeActive&&(
     !karaoke.singer||String(s.name).trim().toLocaleLowerCase()===String(karaoke.singer).trim().toLocaleLowerCase()));
   if(isPerforming){
    const performingTitle=(karaoke.artist?karaoke.artist+' — ':'')+karaoke.title;
    left.append(el('div','entrydesc','Now singing: '+performingTitle));
    if(s.next_song)left.append(el('div','entrydesc','Up next: '+s.next_song));
    left.append(el('div','tag','NOW PERFORMING'));
   }else{
    left.append(el('div','entrydesc',s.next_song?'Up next: '+s.next_song:'No unplayed song'));
   }
   const select=el('button','small select',s.id===selectedSingerId?'Selected':'Select');
   select.disabled=s.id===selectedSingerId;select.dataset.selectSinger=String(s.id);
   const play=el('button','small secondary','▶ Play');play.dataset.playSinger=String(s.id);
   play.disabled=!s.has_song;
   // OKJ_REMOTE_V8_REORDER_SINGERS: move a singer by ID, never by stale row number.
   const order=el('div','rotation-order');
   const up=el('button','small secondary','↑ Move Up');
   up.dataset.moveSinger=String(s.id);up.dataset.moveDirection='up';up.disabled=i===0;
   const down=el('button','small secondary','↓ Move Down');
   down.dataset.moveSinger=String(s.id);down.dataset.moveDirection='down';down.disabled=i===rotation.length-1;
   order.append(up,down);
   acts.append(select,play);top.append(left,acts);row.append(top,order);list.append(row);
  });
 }
 const selected=rotation.find(s=>s.id===selectedSingerId);
 get('selectedSinger').textContent=selected?'Selected: '+selected.name:'Select a singer';
 const removeButton=get('removeSingerBtn');
 removeButton.disabled=!selected;
 removeButton.dataset.singerName=selected?selected.name:'';
 const queue=Array.isArray(data.selected_queue)?data.selected_queue:[];
 const queueHash=JSON.stringify([selectedSingerId,queue]);
 if(queueHash!==lastQueueList){
  lastQueueList=queueHash;const node=get('queueList');clear(node);
  if(!selected)node.append(el('p','sub','Select a singer first.'));
  else if(!queue.length)node.append(el('p','sub','No songs queued yet.'));
  queue.forEach((song,i)=>{
   const row=el('div','entry'+(song.played?' queueplayed':''));
   row.append(el('div','entryname',(i+1)+'. '+(song.title||'Unknown title')));
   row.append(el('div','entrydesc',(song.artist||'Unknown artist')+(song.played?' · Already sung':' · Waiting')+(song.key_change?' · Key '+song.key_change:'')));
   const play=el('button','small primary','▶ Play This Song');
   play.dataset.playQueue=String(song.id);
   play.style.marginTop='9px';
   row.append(play);
   node.append(row);
  });
 }
 if(selectionChanged){clear(get('songResults'));get('searchInfo').textContent=selected?'Search to add songs to '+selected.name:'Select a singer first.';searchSongs();}
}
function scheduleSearch(){clearTimeout(searchTimer);searchTimer=setTimeout(searchSongs,300);}
get('songQuery').addEventListener('input',scheduleSearch);
get('addSingerBtn').addEventListener('click',async()=>{
 const field=get('singerName'),name=field.value.trim();
 if(!name){msg.textContent='Enter a singer name first.';field.focus();return;}
 const btn=get('addSingerBtn');btn.disabled=true;
 try{if(await libraryAction('add_singer',null,null,null,name)){field.value='';}}
 finally{btn.disabled=false;}
});
get('singerName').addEventListener('keydown',e=>{if(e.key==='Enter'){e.preventDefault();get('addSingerBtn').click();}});
// OKJ_REMOTE_V7_REMOVE_SINGER: DJ PIN is required by /remote.
let removingSinger=false;
get('removeSingerBtn').addEventListener('click',async()=>{
 if(removingSinger||selectedSingerId<=0)return;
 const id=selectedSingerId;
 const name=get('removeSingerBtn').dataset.singerName;
 if(!name)return;
 if(!confirm('Remove "'+name+'" from the rotation?\n\nTheir queued songs may no longer be recoverable unless they are a tracked regular.\n\nThis will not interrupt karaoke playback.'))return;
 removingSinger=true;
 try{await libraryAction('remove_singer',id,null);}
 finally{removingSinger=false;}
});
async function searchSongs(){
 const revision=++searchRevision,query=get('songQuery').value.trim(),list=get('songResults');
 clear(list);
 if(selectedSingerId<=0){get('searchInfo').textContent='Select a singer first.';return;}
 if(query.length<2){get('searchInfo').textContent='Type at least two characters to search.';return;}
 if(pinBox.value.length!==6){get('searchInfo').textContent='Enter your PIN first.';return;}
 get('searchInfo').textContent='Searching the laptop catalog…';
 try{
  const response=await fetch('/songs?pin='+encodeURIComponent(pinBox.value)+'&q='+encodeURIComponent(query),{cache:'no-store'});
  if(!response.ok)throw new Error('Search failed');const data=await response.json();
  if(revision!==searchRevision)return;
  get('searchInfo').textContent=data.results.length+' results'+(data.more?' (showing first 40; refine your search)':'');
  if(!data.results.length)list.append(el('p','sub','No matching songs.'));
  data.results.forEach(s=>{
   const row=el('div','entry'),top=el('div','entrytop'),left=el('div'),right=el('div','entryactions');
   left.append(el('div','entryname',s.title||'Unknown title'));
   left.append(el('div','entrydesc',(s.artist||'Unknown artist')+(s.disc_id?' · '+s.disc_id:'')));
   const add=el('button','small primary','+ Add');add.dataset.addSong=String(s.id);
   right.append(add);top.append(left,right);row.append(top);list.append(row);
  });
 }catch(e){if(revision===searchRevision)get('searchInfo').textContent='Could not search the catalog.';}
}
async function libraryAction(cmd,singerId,songId,queueSongId=null,singerName=null,guestRequestId=null,moveDirection=null){
 if(pinBox.value.length!==6){msg.textContent='Enter your PIN first.';return;}
 if((cmd==='play_singer'||cmd==='play_queue_song')&&!confirm('Play this song now? This may interrupt the current song.'))return false;
 try{
  const response=await fetch('/remote',{method:'POST',headers:{'Content-Type':'application/json'},cache:'no-store',body:JSON.stringify({cmd,pin:pinBox.value,singer_id:singerId,song_id:songId,queue_song_id:queueSongId,singer_name:singerName,guest_request_id:guestRequestId,move_direction:moveDirection})});
  const data=await response.json();
  msg.textContent=data.message||(response.ok?'Done':data.error||'Action failed');
  if(response.ok)updateStatus();
  return response.ok;
 }catch(e){msg.textContent='Lost connection to OpenKJ.';return false;}
}
// OKJ_REMOTE_V8_REORDER_SINGERS: avoid multiple moves from rapid repeated taps.
let movingSinger=false;
async function moveSingerFromPhone(id,direction){
 if(movingSinger)return;
 movingSinger=true;
 try{await libraryAction('move_singer',id,null,null,null,null,direction);}
 finally{movingSinger=false;}
}
document.addEventListener('click',e=>{
 const button=e.target.closest('button');if(!button)return;
 if(button.dataset.selectSinger)libraryAction('select_singer',Number(button.dataset.selectSinger),null);
 else if(button.dataset.playSinger)libraryAction('play_singer',Number(button.dataset.playSinger),null);
 else if(button.dataset.moveSinger)moveSingerFromPhone(Number(button.dataset.moveSinger),button.dataset.moveDirection);
 else if(button.dataset.addSong){if(selectedSingerId>0)libraryAction('add_song',selectedSingerId,Number(button.dataset.addSong));}
 else if(button.dataset.playQueue){if(selectedSingerId>0)libraryAction('play_queue_song',selectedSingerId,null,Number(button.dataset.playQueue));}
 else if(button.dataset.guestApprove)libraryAction('approve_guest_request',null,null,null,null,Number(button.dataset.guestApprove));
 else if(button.dataset.guestReject)libraryAction('reject_guest_request',null,null,null,null,Number(button.dataset.guestReject));
});
// Authenticated host download controls. Render external titles/status as text only.
let youtubeState={},youtubeSending=false,youtubeSession='',youtubeWasReady=false;
function youtubeButtons(){
 const busy=!!youtubeState.busy,ready=!!youtubeState.ready;
 get('youtubeStart').disabled=youtubeSending||busy;
 get('youtubeCancel').disabled=youtubeSending||!busy;
 get('youtubeUrl').disabled=youtubeSending||busy;
 get('youtubeArtist').disabled=youtubeSending||!ready;
 get('youtubeTitle').disabled=youtubeSending||!ready;
 get('youtubeImport').disabled=youtubeSending||!ready||!get('youtubeArtist').value.trim()||!get('youtubeTitle').value.trim();
}
function drawYoutube(state){
 const changed=state.session!==youtubeSession;
 if(changed||(!youtubeWasReady&&state.ready)){
  get('youtubeArtist').value=state.artist||'';get('youtubeTitle').value=state.title||'';
  get('youtubeFeedback').textContent='';
 }
 youtubeState=state;youtubeSession=state.session||'';youtubeWasReady=!!state.ready;
 get('youtubeHost').hidden=false;
 get('youtubeStatus').textContent=state.message||'Ready. Configure the downloader on the laptop before first use.';
 youtubeButtons();
}
async function youtubeAction(cmd){
 if(youtubeSending)return;
 const pin=pinBox.value.trim();if(pin.length!==6){msg.textContent='Enter your PIN first.';return;}
 if(cmd==='youtube_download'&&youtubeState.ready&&!confirm('Start another download? The saved video will remain in the laptop folder, but its pending library import will be replaced.'))return;
 const title=get('youtubeTitle').value.trim();
 const payload={cmd,pin,session:youtubeSession,url:get('youtubeUrl').value.trim(),artist:get('youtubeArtist').value.trim(),title};
 youtubeSending=true;youtubeButtons();get('youtubeFeedback').textContent='Sending…';
 try{
  const response=await fetch('/remote',{method:'POST',headers:{'Content-Type':'application/json'},cache:'no-store',body:JSON.stringify(payload)});
  const data=await response.json();
  if(data.youtube_download)drawYoutube(data.youtube_download);
  get('youtubeFeedback').textContent=response.ok?(cmd==='youtube_import'?'Added to library. Search above to add it to a singer.':'Request accepted.'):data.error||'Download action failed.';
  if(response.ok&&cmd==='youtube_import'){get('songQuery').value=title;searchSongs();}
  if(response.status===403)get('youtubeHost').hidden=true;
  updateStatus();
 }catch(e){get('youtubeFeedback').textContent='Connection lost. Reconnect to check the laptop status before retrying.';}
 finally{youtubeSending=false;youtubeButtons();}
}
get('youtubeStart').addEventListener('click',()=>youtubeAction('youtube_download'));
get('youtubeCancel').addEventListener('click',()=>youtubeAction('youtube_cancel'));
get('youtubeImport').addEventListener('click',()=>youtubeAction('youtube_import'));
get('youtubeArtist').addEventListener('input',youtubeButtons);
get('youtubeTitle').addEventListener('input',youtubeButtons);
async function updateStatus(){
 const pin=pinBox.value.trim();
 if(pin.length!==6){get('youtubeHost').hidden=true;get('connection').textContent='Enter your PIN to connect';return;}
 if(working)return;working=true;
 try {
  const response=await fetch('/status?pin='+encodeURIComponent(pin),{cache:'no-store'});
  if(!response.ok){get('youtubeHost').hidden=true;get('connection').textContent=response.status===403?'Incorrect PIN':'Connection issue';return;}
  const data=await response.json();
  get('connection').textContent='● Connected to OpenKJ';
  draw('k',data.karaoke||{});draw('b',data.break_music||{});drawRotation(data);drawGuestRequests(data);drawYoutube(data.youtube_download||{});
 }catch(e){get('connection').textContent='Offline Wi-Fi connection lost';}
 finally{working=false;}
}
async function send(cmd){
 const pin=pinBox.value.trim();
 if(pin.length!==6){msg.textContent='Enter the PIN displayed on the laptop.';pinBox.focus();return;}
 msg.textContent='Sending '+cmd.replaceAll('_',' ')+'...';
 try{
  const response=await fetch('/remote',{method:'POST',headers:{'Content-Type':'application/json'},cache:'no-store',body:JSON.stringify({cmd,pin})});
  const data=await response.json();
  msg.textContent=response.ok?'✓ '+cmd.replaceAll('_',' ')+' sent':(data.error||'Command rejected');
  if(response.ok)setTimeout(updateStatus,250);
 }catch(e){msg.textContent='Not connected. Check offline hotspot and laptop firewall.';}
}
document.querySelectorAll('[data-cmd]').forEach(button=>button.addEventListener('click',()=>send(button.dataset.cmd)));
updateStatus();setInterval(updateStatus,1000);
</script></body></html>)OPENKJHTML";

bool isPrivateIp(quint32 ip) {
    return ((ip & 0xff000000u) == 0x0a000000u) ||
           ((ip & 0xfff00000u) == 0xac100000u) ||
           ((ip & 0xffff0000u) == 0xc0a80000u);
}
QByteArray jsonError(const QString &message) {
    return QJsonDocument(QJsonObject{{QStringLiteral("ok"), false},
                                     {QStringLiteral("error"), message}}).toJson(QJsonDocument::Compact);
}
} // namespace

RemoteControlServer::RemoteControlServer(QObject *parent) : QTcpServer(parent) {
    m_pin = QString::number(100000u + QRandomGenerator::global()->bounded(900000u));
    connect(this, &QTcpServer::newConnection, this, &RemoteControlServer::acceptConnections);
}

void RemoteControlServer::setStatusProvider(std::function<QJsonObject()> callback) {
    m_statusProvider = std::move(callback);
}
void RemoteControlServer::setSongSearchProvider(std::function<QJsonObject(const QString &)> callback) {
    m_songSearchProvider = std::move(callback);
}
void RemoteControlServer::setLibraryActionHandler(
        std::function<QJsonObject(const QString &, const QJsonObject &)> callback) {
    m_libraryActionHandler = std::move(callback);
}

void RemoteControlServer::setHostProviders(std::function<QJsonObject(const QJsonObject &)> state,
        std::function<QJsonObject(const QJsonObject &)> search,
        std::function<QJsonObject(const QString &, const QJsonObject &)> action) {
    m_hostState=std::move(state); m_hostSearch=std::move(search); m_hostAction=std::move(action);
}

bool RemoteControlServer::start(quint16 preferredPort) {
    if (isListening()) return true;
    if (listen(QHostAddress::AnyIPv4, preferredPort)) return true;
    return listen(QHostAddress::AnyIPv4, 0);
}

QStringList RemoteControlServer::localUrls() const {
    QStringList privateUrls, otherUrls;
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        const auto flags = iface.flags();
        if (!(flags & QNetworkInterface::IsUp) || !(flags & QNetworkInterface::IsRunning) ||
            (flags & QNetworkInterface::IsLoopBack)) continue;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            const QHostAddress ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol || ip.isLoopback()) continue;
            const QString url = QStringLiteral("http://%1:%2/").arg(ip.toString()).arg(serverPort());
            if (isPrivateIp(ip.toIPv4Address())) privateUrls.append(url);
            else otherUrls.append(url);
        }
    }
    privateUrls.removeDuplicates();otherUrls.removeDuplicates();
    privateUrls.append(otherUrls);
    if (privateUrls.isEmpty())
        privateUrls.append(QStringLiteral("http://127.0.0.1:%1/").arg(serverPort()));
    return privateUrls;
}

void RemoteControlServer::acceptConnections() {
    while (hasPendingConnections()) {
        QTcpSocket *socket = nextPendingConnection();
        m_pending.insert(socket, QByteArray{});
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() { readFrom(socket); });
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            m_pending.remove(socket);
            socket->deleteLater();
        });
        auto *timeout = new QTimer(socket);
        timeout->setSingleShot(true);
        connect(timeout, &QTimer::timeout, socket, &QTcpSocket::disconnectFromHost);
        timeout->start(5000);
    }
}

void RemoteControlServer::readFrom(QTcpSocket *socket) {
    if (!m_pending.contains(socket)) { socket->readAll(); return; }
    QByteArray &pending = m_pending[socket];
    pending.append(socket->readAll());
    if (pending.size() > kMaxRequestBytes) {
        m_pending.remove(socket);
        reply(socket, 413, "Payload Too Large", "application/json", jsonError("Request too large"));
        return;
    }
    const int delimiter = pending.indexOf("\r\n\r\n");
    if (delimiter < 0) return;
    const QByteArray header = pending.left(delimiter);
    const QList<QByteArray> lines = header.split('\n');
    if (lines.isEmpty()) return;
    const QByteArray method = lines.first().trimmed().split(' ').value(0);
    qint64 contentLength = 0;
    bool seenLength = false, bad = false;
    for (int i = 1; i < lines.size(); ++i) {
        const QByteArray line = lines[i].trimmed();
        const int colon = line.indexOf(':');
        if (colon <= 0) { bad = true; break; }
        const QByteArray key = line.left(colon).trimmed().toLower();
        const QByteArray val = line.mid(colon + 1).trimmed();
        if (key == "transfer-encoding") { bad = true; break; }
        if (key == "content-length") {
            if (seenLength) { bad = true; break; }
            seenLength = true;
            bool ok = false;
            contentLength = val.toLongLong(&ok);
            if (!ok || contentLength < 0 || contentLength > kMaxRequestBytes) { bad = true; break; }
        }
    }
    if (bad || (method == "POST" && !seenLength)) {
        m_pending.remove(socket);
        reply(socket, 400, "Bad Request", "application/json", jsonError("Invalid HTTP headers"));
        return;
    }
    const int bodyStart = delimiter + 4;
    if (contentLength > kMaxRequestBytes - bodyStart) {
        m_pending.remove(socket);
        reply(socket, 413, "Payload Too Large", "application/json", jsonError("Body too large"));
        return;
    }
    if (pending.size() - bodyStart < contentLength) return;
    const QByteArray body = pending.mid(bodyStart, static_cast<int>(contentLength));
    m_pending.remove(socket);
    processRequest(socket, header, body);
}

void RemoteControlServer::processRequest(QTcpSocket *socket, const QByteArray &header,
                                         const QByteArray &body) {
    const QList<QByteArray> firstLine = header.split('\n').first().trimmed().split(' ');
    if (firstLine.size() != 3 || !firstLine[2].startsWith("HTTP/")) {
        reply(socket, 400, "Bad Request", "application/json", jsonError("Malformed HTTP request"));
        return;
    }
    const QByteArray method = firstLine[0];
    const QUrl url = QUrl::fromEncoded(firstLine[1]);
    if (!url.isValid()) {
        reply(socket, 400, "Bad Request", "application/json", jsonError("Invalid URL"));
        return;
    }
    // OKJ_REMOTE_V5_GUEST_REQUESTS
    // Public guest page is available only to local RFC1918 or loopback clients.
    const QHostAddress peer = socket->peerAddress();
    const bool localGuest = peer.isLoopback() ||
        (peer.protocol() == QAbstractSocket::IPv4Protocol && isPrivateIp(peer.toIPv4Address()));
    if (url.path() == QStringLiteral("/request") ||
        url.path() == QStringLiteral("/public/songs") ||
        url.path() == QStringLiteral("/public/request")) {
        if (!localGuest) {
            reply(socket, 403, "Forbidden", "application/json", jsonError("Local network only"));
            return;
        }
        if (url.path() == QStringLiteral("/request")) {
            if (method != "GET") {
                reply(socket, 405, "Method Not Allowed", "application/json", jsonError("GET required"));
                return;
            }
            reply(socket, 200, "OK", "text/html; charset=utf-8", QByteArray(kGuestRequestPage));
            return;
        }
        if (url.path() == QStringLiteral("/public/songs")) {
            if (method != "GET") {
                reply(socket, 405, "Method Not Allowed", "application/json", jsonError("GET required"));
                return;
            }
            const QString q = QUrlQuery(url).queryItemValue(QStringLiteral("q")).trimmed();
            if (q.size() < 2 || q.size() > 100) {
                reply(socket, 400, "Bad Request", "application/json", jsonError("Search length must be 2-100 characters"));
                return;
            }
            const QJsonObject found = m_songSearchProvider ? m_songSearchProvider(q) : QJsonObject{};
            reply(socket, 200, "OK", "application/json", QJsonDocument(found).toJson(QJsonDocument::Compact));
            return;
        }
        if (method != "POST" || !m_libraryActionHandler) {
            reply(socket, 405, "Method Not Allowed", "application/json", jsonError("POST required"));
            return;
        }
        QJsonParseError error{};
        const QJsonDocument document = QJsonDocument::fromJson(body, &error);
        if (error.error != QJsonParseError::NoError || !document.isObject()) {
            reply(socket, 400, "Bad Request", "application/json", jsonError("Expected JSON"));
            return;
        }
        // Each client can submit one request per 5 seconds. Protects against accidental spam.
        static QHash<QString, qint64> lastGuestRequest;
        if (lastGuestRequest.size() > 2048) lastGuestRequest.clear();
        const QString ip = peer.toString();
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (lastGuestRequest.contains(ip) && now - lastGuestRequest.value(ip) < 5000) {
            reply(socket, 429, "Too Many Requests", "application/json", jsonError("Please wait a few seconds before requesting another song"));
            return;
        }
        lastGuestRequest.insert(ip, now);
        const QJsonObject result = m_libraryActionHandler(QStringLiteral("guest_submit"), document.object());
        const bool ok = result.value(QStringLiteral("ok")).toBool();
        reply(socket, ok ? 200 : 400, ok ? "OK" : "Bad Request", "application/json",
              QJsonDocument(result).toJson(QJsonDocument::Compact));
        return;
    }
    // OKJ_REMOTE_V6_LOCAL_QR - public display only; NO playback API and NO PIN exposure.
    if (method == "GET" && (url.path() == QStringLiteral("/qr") ||
                             url.path() == QStringLiteral("/qr-library.js"))) {
        if (!localGuest) {
            reply(socket, 403, "Forbidden", "application/json", jsonError("Local network only"));
            return;
        }
        if (url.path() == QStringLiteral("/qr"))
            reply(socket, 200, "OK", "text/html; charset=utf-8", QByteArray(kQrPage));
        else
            reply(socket, 200, "OK", "application/javascript; charset=utf-8", QByteArray(kQrLibrary));
        return;
    }
    if (method == "GET" && url.path() == QStringLiteral("/classic")) {
        reply(socket,200,"OK","text/html; charset=utf-8",QByteArray(kControlPage)); return;
    }
    const QHash<QString,QString> assets{{"/", "index.html"},{"/host-console.css","host.css"},{"/host-console.js","host.js"}};
    if (method == "GET" && assets.contains(url.path())) {
        QFile asset(":/host-console/"+assets.value(url.path()));
        if (!asset.open(QIODevice::ReadOnly)) { reply(socket,500,"Internal Server Error","text/plain","Host interface unavailable"); return; }
        const auto type=url.path().endsWith(".css") ? "text/css; charset=utf-8" : url.path().endsWith(".js") ? "application/javascript; charset=utf-8" : "text/html; charset=utf-8";
        reply(socket,200,"OK",type,asset.readAll()); return;
    }
    if (method != "GET" && method != "POST") {
        reply(socket, 405, "Method Not Allowed", "application/json", jsonError("GET or POST required"));
        return;
    }
    if (url.path() != QStringLiteral("/remote") && url.path() != QStringLiteral("/status") &&
        url.path() != QStringLiteral("/songs") && url.path() != "/host/state" &&
        url.path() != "/host/search" && url.path() != "/host/action") {
        reply(socket, 404, "Not Found", "application/json", jsonError("Unknown endpoint"));
        return;
    }
    QString pin, cmd;
    QJsonObject posted;
    if (method == "GET") {
        const QUrlQuery query(url);
        cmd = query.queryItemValue(QStringLiteral("cmd"));
        pin = query.queryItemValue(QStringLiteral("pin"));
    } else {
        if (url.path() == QStringLiteral("/status")) {
            reply(socket, 405, "Method Not Allowed", "application/json", jsonError("GET required"));
            return;
        }
        QJsonParseError err{};
        const QJsonDocument json = QJsonDocument::fromJson(body, &err);
        if (err.error != QJsonParseError::NoError || !json.isObject()) {
            reply(socket, 400, "Bad Request", "application/json", jsonError("Expected JSON object"));
            return;
        }
        posted = json.object();
        cmd = posted.value(QStringLiteral("cmd")).toString();
        pin = posted.value(QStringLiteral("pin")).toString();
    }
    if (pin != m_pin) {
        const auto ip=socket->peerAddress().toString();
        const auto now=QDateTime::currentMSecsSinceEpoch();
        if (m_failedPins.size()>2048) m_failedPins.clear();
        auto &attempts=m_failedPins[ip];
        if (!attempts.second || now-attempts.second>=60000) attempts={0,now};
        if (attempts.first>=12) {
            reply(socket,429,"Too Many Requests","application/json",jsonError("Too many incorrect PINs. Wait one minute.")); return;
        }
        ++attempts.first;
        reply(socket, 403, "Forbidden", "application/json", jsonError("Incorrect PIN"));
        return;
    }
    // Correct host credentials remain usable even if another client behind the same IP mistyped.
    m_failedPins.remove(socket->peerAddress().toString());
    if (url.path().startsWith("/host/")) {
        const bool action=url.path()=="/host/action";
        if ((action && method!="POST") || (!action && method!="GET")) {
            reply(socket,405,"Method Not Allowed","application/json",jsonError(action?"POST required":"GET required")); return;
        }
        QJsonObject args=posted;
        if (!action) {
            // URLSearchParams uses form encoding: '+' is a space, while '%2B'
            // is a literal plus. Replace before percent decoding, exactly once.
            const auto decodeFormValue=[](const QString &value) {
                return QUrl::fromPercentEncoding(value.toUtf8().replace('+',' '));
            };
            for (const auto &item:QUrlQuery(url).queryItems(QUrl::FullyEncoded)) {
                const auto key=decodeFormValue(item.first);
                if (key!="pin") args.insert(key,decodeFormValue(item.second));
            }
        }
        QJsonObject result;
        if (action && m_hostAction) result=m_hostAction(cmd,args);
        else if (url.path()=="/host/state" && m_hostState) result=m_hostState(args);
        else if (url.path()=="/host/search" && m_hostSearch) result=m_hostSearch(args);
        else result=QJsonObject{{"ok",false},{"error","Host service unavailable"}};
        const bool ok=result.value("ok").toBool();
        reply(socket,ok?200:400,ok?"OK":"Bad Request","application/json",QJsonDocument(result).toJson(QJsonDocument::Compact)); return;
    }
    if (url.path() == QStringLiteral("/songs")) {
        if (method != "GET") {
            reply(socket, 405, "Method Not Allowed", "application/json", jsonError("GET required"));
            return;
        }
        const QString query = QUrlQuery(url).queryItemValue(QStringLiteral("q")).trimmed();
        if (query.size() < 2 || query.size() > 100) {
            reply(socket, 400, "Bad Request", "application/json", jsonError("Search length must be 2-100 characters"));
            return;
        }
        const auto results = m_songSearchProvider ? m_songSearchProvider(query) : QJsonObject{};
        reply(socket, 200, "OK", "application/json", QJsonDocument(results).toJson(QJsonDocument::Compact));
        return;
    }
    if (url.path() == QStringLiteral("/status")) {
        QJsonObject state = m_statusProvider ? m_statusProvider() : QJsonObject{};
        state.insert(QStringLiteral("ok"), true);
        reply(socket, 200, "OK", "application/json", QJsonDocument(state).toJson(QJsonDocument::Compact));
        return;
    }
    if (cmd == QStringLiteral("select_singer") || cmd == QStringLiteral("play_singer") ||
        cmd == QStringLiteral("add_song") || cmd == QStringLiteral("add_singer") ||
        cmd == QStringLiteral("play_queue_song") ||
        cmd == QStringLiteral("remove_singer") ||
        cmd == QStringLiteral("move_singer") ||
        cmd == QStringLiteral("approve_guest_request") ||
        cmd == QStringLiteral("reject_guest_request") ||
        cmd == QStringLiteral("youtube_download") ||
        cmd == QStringLiteral("youtube_cancel") ||
        cmd == QStringLiteral("youtube_import")) {
        if (method != "POST" || !m_libraryActionHandler) {
            reply(socket, 405, "Method Not Allowed", "application/json", jsonError("POST required"));
            return;
        }
        QJsonObject result = m_libraryActionHandler(cmd, posted);
        const bool ok = result.value(QStringLiteral("ok")).toBool();
        reply(socket, ok ? 200 : 400, ok ? "OK" : "Bad Request", "application/json",
              QJsonDocument(result).toJson(QJsonDocument::Compact));
        return;
    }
    const QStringList commands = {
        QStringLiteral("play"), QStringLiteral("pause"), QStringLiteral("toggle"),
        QStringLiteral("next"), QStringLiteral("stop"), QStringLiteral("volume_up"),
        QStringLiteral("volume_down"), QStringLiteral("break_play"),
        QStringLiteral("break_pause"), QStringLiteral("break_toggle"),
        QStringLiteral("break_next"), QStringLiteral("break_previous"),
        QStringLiteral("break_stop"), QStringLiteral("break_volume_up"),
        QStringLiteral("break_volume_down"), QStringLiteral("break_fade_out"),
        QStringLiteral("break_fade_in")};
    if (!commands.contains(cmd)) {
        reply(socket, 400, "Bad Request", "application/json", jsonError("Unknown command"));
        return;
    }
    reply(socket, 200, "OK", "application/json", QByteArrayLiteral("{\"ok\":true}"));
    // Send HTTP response BEFORE invoking any UI action that could open a modal warning.
    QTimer::singleShot(0, this, [this, cmd]() {
        if (cmd == "play") emit playRequested();
        else if (cmd == "pause") emit pauseRequested();
        else if (cmd == "toggle") emit playPauseRequested();
        else if (cmd == "next") emit nextRequested();
        else if (cmd == "stop") emit stopRequested();
        else if (cmd == "volume_up") emit volumeUpRequested();
        else if (cmd == "volume_down") emit volumeDownRequested();
        else if (cmd == "break_play") emit breakPlayRequested();
        else if (cmd == "break_pause") emit breakPauseRequested();
        else if (cmd == "break_toggle") emit breakToggleRequested();
        else if (cmd == "break_next") emit breakNextRequested();
        else if (cmd == "break_previous") emit breakPreviousRequested();
        else if (cmd == "break_stop") emit breakStopRequested();
        else if (cmd == "break_volume_up") emit breakVolumeUpRequested();
        else if (cmd == "break_volume_down") emit breakVolumeDownRequested();
        else if (cmd == "break_fade_out") emit breakFadeOutRequested();
        else if (cmd == "break_fade_in") emit breakFadeInRequested();
    });
}

void RemoteControlServer::reply(QTcpSocket *socket, int status, const char *description,
                                const QByteArray &contentType, const QByteArray &body) {
    const QByteArray h = QByteArray("HTTP/1.1 ") + QByteArray::number(status) + " " +
        description + "\r\nContent-Type: " + contentType +
        "\r\nContent-Length: " + QByteArray::number(body.size()) +
        "\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
        "Referrer-Policy: no-referrer\r\nConnection: close\r\n\r\n";
    socket->write(h);
    socket->write(body);
    socket->disconnectFromHost();
}
